#!/usr/bin/env python3
"""Generate the minimal UObject type metadata used by JisooEngine."""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from tempfile import TemporaryDirectory


REFLECTED_CLASS_RE = re.compile(
    r"\bUCLASS\s*\([^)]*\)\s*"
    r"class\s+"
    r"(?:(?:[A-Z_][A-Z0-9_]*|final|abstract)\s+)*"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)"
    r"(?:\s+(?:final|abstract))?"
    r"(?:\s*:\s*(?:(?:public|protected|private)\s+)?"
    r"(?P<super>[A-Za-z_][A-Za-z0-9_:]*))?",
    re.MULTILINE,
)


@dataclass(frozen=True)
class ReflectedClass:
    name: str
    super_name: str | None
    generated_body_line: int


@dataclass(frozen=True)
class ReflectedHeader:
    source: Path
    relative_source: Path
    generated_header: Path
    generated_source: Path
    file_id: str
    classes: tuple[ReflectedClass, ...]


class GenerationError(RuntimeError):
    pass


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate JisooEngine UObject metadata.")
    parser.add_argument("--source-root", type=Path)
    parser.add_argument("--generated-root", type=Path)
    parser.add_argument("--module", default="JisooEngine")
    parser.add_argument("--self-test", action="store_true")
    return parser.parse_args()


def mask_non_code(text: str) -> str:
    """문자 위치와 줄 번호를 보존하면서 주석과 문자열 내용을 가린다."""
    result = list(text)
    index = 0
    state = "code"
    quote = ""

    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""

        if state == "code":
            if char == "/" and next_char == "/":
                result[index] = result[index + 1] = " "
                index += 2
                state = "line_comment"
                continue
            if char == "/" and next_char == "*":
                result[index] = result[index + 1] = " "
                index += 2
                state = "block_comment"
                continue
            if char in {'"', "'"}:
                result[index] = " "
                quote = char
                state = "string"
        elif state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                result[index] = " "
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                result[index] = result[index + 1] = " "
                index += 2
                state = "code"
                continue
            if char != "\n":
                result[index] = " "
        elif state == "string":
            if char == "\\":
                result[index] = " "
                if index + 1 < len(text):
                    if text[index + 1] != "\n":
                        result[index + 1] = " "
                    index += 2
                    continue
            if char == quote:
                result[index] = " "
                state = "code"
            elif char != "\n":
                result[index] = " "

        index += 1

    return "".join(result)


def find_matching_brace(text: str, opening_brace: int) -> int:
    depth = 0
    for index in range(opening_brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return index
    return -1


def brace_depth_at(text: str, position: int) -> int:
    depth = 0
    for char in text[:position]:
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
    return depth


def conditional_depth_at(text: str, position: int) -> int:
    depth = 0
    for line in text[:position].splitlines():
        directive = line.strip()
        if re.match(r"#\s*(if|ifdef|ifndef)\b", directive):
            depth += 1
        elif re.match(r"#\s*endif\b", directive):
            depth = max(0, depth - 1)
    return depth


def line_number(text: str, position: int) -> int:
    return text.count("\n", 0, position) + 1


def make_file_id(module: str, relative_source: Path) -> str:
    value = f"FID_{module}_{relative_source.as_posix()}"
    return re.sub(r"[^A-Za-z0-9_]", "_", value)


def generated_include_for(relative_source: Path) -> str:
    return relative_source.with_name(f"{relative_source.stem}.generated.h").as_posix()


def parse_reflected_header(
    source: Path,
    source_root: Path,
    generated_root: Path,
    module: str,
) -> ReflectedHeader | None:
    source_text = source.read_text(encoding="utf-8-sig")
    scan_text = mask_non_code(source_text)
    matches = list(REFLECTED_CLASS_RE.finditer(scan_text))
    if not matches:
        return None

    relative_source = source.relative_to(source_root)
    expected_include = generated_include_for(relative_source)
    include_pattern = re.compile(
        rf'#\s*include\s*["<]{re.escape(expected_include)}[">]'
    )
    expected_include_match = include_pattern.search(source_text)
    if expected_include_match is None:
        raise GenerationError(
            f"{relative_source}: reflected header must include \"{expected_include}\""
        )
    include_matches = list(
        re.finditer(
            r'^[ \t]*#[ \t]*include[ \t]*["<][^">]+[">]',
            source_text,
            re.MULTILINE,
        )
    )
    if (
        include_matches
        and line_number(source_text, include_matches[-1].start())
        != line_number(source_text, expected_include_match.start())
    ):
        raise GenerationError(
            f"{relative_source}: generated header must be the last include"
        )

    classes: list[ReflectedClass] = []
    for match in matches:
        declaration_line = line_number(scan_text, match.start())
        if brace_depth_at(scan_text, match.start()) != 0:
            raise GenerationError(
                f"{relative_source}({declaration_line}): nested or namespaced UCLASS is not supported"
            )
        if conditional_depth_at(scan_text, match.start()) != 0:
            raise GenerationError(
                f"{relative_source}({declaration_line}): conditional UCLASS declaration is not supported"
            )

        prefix = scan_text[max(0, match.start() - 512):match.start()]
        if re.search(r"template\s*<[^;{}]*>\s*$", prefix, re.DOTALL):
            raise GenerationError(
                f"{relative_source}({declaration_line}): templated UCLASS is not supported"
            )

        opening_brace = scan_text.find("{", match.end())
        if opening_brace < 0:
            raise GenerationError(
                f"{relative_source}({declaration_line}): class body was not found"
            )
        inheritance_tail = scan_text[match.end():opening_brace]
        if "," in inheritance_tail:
            raise GenerationError(
                f"{relative_source}({declaration_line}): multiple inheritance is not supported"
            )

        closing_brace = find_matching_brace(scan_text, opening_brace)
        if closing_brace < 0:
            raise GenerationError(
                f"{relative_source}({declaration_line}): class body is not closed"
            )

        body = scan_text[opening_brace + 1:closing_brace]
        generated_body_matches = list(re.finditer(r"\bGENERATED_BODY\s*\(\s*\)", body))
        if len(generated_body_matches) != 1:
            raise GenerationError(
                f"{relative_source}({declaration_line}): UCLASS requires exactly one GENERATED_BODY()"
            )

        class_name = match.group("name")
        super_name = match.group("super")
        if super_name is None and class_name != "UObject":
            raise GenerationError(
                f"{relative_source}({declaration_line}): only UObject may omit a reflected superclass"
            )

        generated_body_position = opening_brace + 1 + generated_body_matches[0].start()
        classes.append(
            ReflectedClass(
                name=class_name,
                super_name=super_name,
                generated_body_line=line_number(scan_text, generated_body_position),
            )
        )

    relative_generated_header = relative_source.with_name(
        f"{relative_source.stem}.generated.h"
    )
    relative_generated_source = relative_source.with_name(
        f"{relative_source.stem}.gen.cpp"
    )
    return ReflectedHeader(
        source=source,
        relative_source=relative_source,
        generated_header=generated_root / relative_generated_header,
        generated_source=generated_root / relative_generated_source,
        file_id=make_file_id(module, relative_source),
        classes=tuple(classes),
    )


def render_generated_header(header: ReflectedHeader) -> str:
    lines = [
        "// 이 파일은 Scripts/GenerateHeaders.py가 생성한다. 직접 수정하지 않는다.",
        "#pragma once",
        "",
        "#undef CURRENT_FILE_ID",
        f"#define CURRENT_FILE_ID {header.file_id}",
        "",
    ]

    for reflected_class in header.classes:
        macro_name = (
            f"{header.file_id}_{reflected_class.generated_body_line}_GENERATED_BODY"
        )
        macro_lines = [
            f"#define {macro_name} \\",
            "public: \\",
            f"    using ThisClass = {reflected_class.name}; \\",
        ]
        if reflected_class.super_name:
            macro_lines.append(
                f"    using Super = {reflected_class.super_name}; \\",
            )
        macro_lines.extend(
            [
                "    static UClass* StaticClass(); \\",
                "private: \\",
                "    static void InternalConstructor(void* Memory, const FObjectInitializer& ObjectInitializer); \\",
                "    static void InternalDestructor(UObject* Object); \\",
                "public:",
                "",
            ]
        )
        lines.extend(macro_lines)

    return "\n".join(lines)


def render_generated_source(header: ReflectedHeader) -> str:
    lines = [
        "// 이 파일은 Scripts/GenerateHeaders.py가 생성한다. 직접 수정하지 않는다.",
        '#include "Runtime/CoreUObject/Class.h"',
        '#include "Runtime/CoreUObject/ObjectInitializer.h"',
        f'#include "{header.relative_source.as_posix()}"',
        "",
        "#include <new>",
        "",
    ]

    for reflected_class in header.classes:
        class_name = reflected_class.name
        super_expression = (
            f"{reflected_class.super_name}::StaticClass()"
            if reflected_class.super_name
            else "nullptr"
        )
        lines.extend(
            [
                f"UClass* {class_name}::StaticClass()",
                "{",
                "    static UClass Class(",
                f'        "{class_name}",',
                f"        {super_expression},",
                f"        sizeof({class_name}),",
                f"        alignof({class_name}),",
                f"        &{class_name}::InternalConstructor,",
                f"        &{class_name}::InternalDestructor);",
                "    return &Class;",
                "}",
                "",
                f"void {class_name}::InternalConstructor(",
                "    void* Memory,",
                "    const FObjectInitializer& ObjectInitializer)",
                "{",
                f"    new (Memory) {class_name}(ObjectInitializer);",
                "}",
                "",
                f"void {class_name}::InternalDestructor(UObject* Object)",
                "{",
                f"    static_cast<{class_name}*>(Object)->~{class_name}();",
                "}",
                "",
            ]
        )

    return "\n".join(lines)


def render_hub(headers: list[ReflectedHeader], generated_root: Path) -> str:
    lines = [
        "// 이 파일은 Scripts/GenerateHeaders.py가 생성한다. 직접 수정하지 않는다.",
    ]
    for header in headers:
        include_path = header.generated_source.relative_to(generated_root).as_posix()
        lines.append(f'#include "{include_path}"')
    lines.append("")
    return "\n".join(lines)


def write_if_changed(path: Path, content: str) -> bool:
    encoded = content.encode("utf-8")
    if path.exists() and path.read_bytes() == encoded:
        return False

    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f"{path.name}.tmp")
    temporary.write_bytes(encoded)
    os.replace(temporary, path)
    return True


def remove_stale_outputs(generated_root: Path, current_outputs: set[str]) -> None:
    manifest_path = generated_root / ".reflection-manifest.json"
    previous_outputs: set[str] = set()
    if manifest_path.exists():
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        previous_outputs = set(manifest.get("outputs", []))

    for relative_output in previous_outputs - current_outputs:
        target = (generated_root / relative_output).resolve()
        try:
            target.relative_to(generated_root.resolve())
        except ValueError as error:
            raise GenerationError(f"refusing to remove output outside generated root: {target}") from error
        if target.is_file():
            target.unlink()

    write_if_changed(
        manifest_path,
        json.dumps({"outputs": sorted(current_outputs)}, indent=2) + "\n",
    )


def generate(source_root: Path, generated_root: Path, module: str) -> int:
    source_root = source_root.resolve()
    generated_root = generated_root.resolve()
    if not source_root.is_dir():
        raise GenerationError(f"source root does not exist: {source_root}")

    reflected_headers: list[ReflectedHeader] = []
    for source in sorted(source_root.rglob("*.h")):
        reflected = parse_reflected_header(source, source_root, generated_root, module)
        if reflected:
            reflected_headers.append(reflected)

    hub_path = generated_root / "Reflection.gen.cpp"
    outputs = {hub_path.relative_to(generated_root).as_posix()}
    for header in reflected_headers:
        write_if_changed(header.generated_header, render_generated_header(header))
        write_if_changed(header.generated_source, render_generated_source(header))
        outputs.add(header.generated_header.relative_to(generated_root).as_posix())
        outputs.add(header.generated_source.relative_to(generated_root).as_posix())

    write_if_changed(hub_path, render_hub(reflected_headers, generated_root))
    remove_stale_outputs(generated_root, outputs)
    return len(reflected_headers)


def run_self_test() -> int:
    with TemporaryDirectory() as temporary_directory:
        root = Path(temporary_directory)
        source_root = root / "Source"
        generated_root = root / "Generated"
        header = source_root / "TestObject.h"
        header.parent.mkdir(parents=True)
        header.write_text(
            '#pragma once\n'
            '#include "TestObject.generated.h"\n'
            'UCLASS()\n'
            'class UTestObject:public UObject\n'
            '{\n'
            '    GENERATED_BODY()\n'
            '};\n',
            encoding="utf-8",
        )
        count = generate(source_root, generated_root, "SelfTest")
        generated_header = (generated_root / "TestObject.generated.h").read_text(
            encoding="utf-8"
        )
        generated_source = (generated_root / "TestObject.gen.cpp").read_text(
            encoding="utf-8"
        )
        if count != 1:
            raise GenerationError("self-test did not discover the reflected header")
        if "using Super = UObject" not in generated_header:
            raise GenerationError("self-test did not generate the superclass alias")
        if "UClass* UTestObject::StaticClass()" not in generated_source:
            raise GenerationError("self-test did not generate StaticClass")
        if "new (Memory) UTestObject(ObjectInitializer);" not in generated_source:
            raise GenerationError(
                "self-test did not forward FObjectInitializer to the constructor"
            )

        template_header = source_root / "TemplateObject.h"
        template_header.write_text(
            '#pragma once\n'
            '#include "TemplateObject.generated.h"\n'
            'template<typename T>\n'
            'UCLASS()\n'
            'class UTemplateObject : public UObject\n'
            '{\n'
            '    GENERATED_BODY()\n'
            '};\n',
            encoding="utf-8",
        )
        try:
            generate(source_root, generated_root, "SelfTest")
        except GenerationError as error:
            if "templated UCLASS" not in str(error):
                raise
        else:
            raise GenerationError("self-test accepted a templated UCLASS")

    print("GenerateHeaders self-test passed.")
    return 0


def main() -> int:
    args = parse_args()
    try:
        if args.self_test:
            return run_self_test()
        if args.source_root is None or args.generated_root is None:
            raise GenerationError("--source-root and --generated-root are required")
        generated_count = generate(args.source_root, args.generated_root, args.module)
        print(f"Generated reflection code for {generated_count} header(s).")
        return 0
    except (GenerationError, OSError, json.JSONDecodeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
