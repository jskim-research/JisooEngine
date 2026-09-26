find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)

set(JISOO_HEADER_GENERATOR
    "${PROJECT_SOURCE_DIR}/Scripts/GenerateHeaders.py"
)
set(JISOO_ENGINE_GENERATED_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/Intermediate/Generated"
)
set(JISOO_ENGINE_REFLECTION_HUB
    "${JISOO_ENGINE_GENERATED_ROOT}/Reflection.gen.cpp"
)

file(GLOB_RECURSE JISOO_ENGINE_REFLECTION_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.h"
)

execute_process(
    COMMAND
        "${Python3_EXECUTABLE}"
        "${JISOO_HEADER_GENERATOR}"
        --source-root "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        --generated-root "${JISOO_ENGINE_GENERATED_ROOT}"
        --module JisooEngine
    RESULT_VARIABLE JISOO_ENGINE_REFLECTION_RESULT
    OUTPUT_VARIABLE JISOO_ENGINE_REFLECTION_OUTPUT
    ERROR_VARIABLE JISOO_ENGINE_REFLECTION_ERROR
)

if(NOT JISOO_ENGINE_REFLECTION_RESULT EQUAL 0)
    message(FATAL_ERROR
        "Failed to generate JisooEngine reflection code.\n"
        "${JISOO_ENGINE_REFLECTION_OUTPUT}${JISOO_ENGINE_REFLECTION_ERROR}"
    )
endif()

add_custom_target(JisooEngineReflectionCode
    COMMAND
        "${Python3_EXECUTABLE}"
        "${JISOO_HEADER_GENERATOR}"
        --source-root "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        --generated-root "${JISOO_ENGINE_GENERATED_ROOT}"
        --module JisooEngine
    BYPRODUCTS "${JISOO_ENGINE_REFLECTION_HUB}"
    DEPENDS
        "${JISOO_HEADER_GENERATOR}"
        ${JISOO_ENGINE_REFLECTION_HEADERS}
    COMMENT "Generating JisooEngine UObject metadata"
    VERBATIM
)

set_source_files_properties(
    "${JISOO_ENGINE_REFLECTION_HUB}"
    PROPERTIES GENERATED TRUE
)

file(GLOB_RECURSE JISOO_ENGINE_RUNTIME_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.inl"
)

add_library(JisooEngineRuntime STATIC
    ${JISOO_ENGINE_RUNTIME_SOURCES}
    "${JISOO_ENGINE_REFLECTION_HUB}"
)
add_library(JisooEngine::Runtime ALIAS JisooEngineRuntime)

add_dependencies(JisooEngineRuntime JisooEngineReflectionCode)

target_link_libraries(JisooEngineRuntime
    PRIVATE
        d3d12
        dxgi
)

target_include_directories(JisooEngineRuntime
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        "${JISOO_ENGINE_GENERATED_ROOT}"
)

jisoo_configure_target(JisooEngineRuntime "${CMAKE_CURRENT_SOURCE_DIR}")

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    PREFIX "Source"
    FILES ${JISOO_ENGINE_RUNTIME_SOURCES}
)

source_group("Generated" FILES "${JISOO_ENGINE_REFLECTION_HUB}")

file(GLOB_RECURSE JISOO_ENGINE_EDITOR_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Editor/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Editor/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Editor/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Editor/*.inl"
)

add_library(JisooEngineEditor STATIC ${JISOO_ENGINE_EDITOR_SOURCES})
add_library(JisooEngine::Editor ALIAS JisooEngineEditor)

target_include_directories(JisooEngineEditor
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

target_link_libraries(JisooEngineEditor
    PUBLIC
        JisooEngine::Runtime
)

jisoo_configure_target(JisooEngineEditor "${CMAKE_CURRENT_SOURCE_DIR}")

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    PREFIX "Source"
    FILES ${JISOO_ENGINE_EDITOR_SOURCES}
)

set_property(
    TARGET JisooEngineRuntime JisooEngineEditor JisooEngineReflectionCode
    PROPERTY FOLDER "JisooEngine"
)

if(BUILD_TESTING)
    set(JISOO_TEST_GENERATED_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/Intermediate/GeneratedTests"
    )
    set(JISOO_TEST_REFLECTION_HUB
        "${JISOO_TEST_GENERATED_ROOT}/Reflection.gen.cpp"
    )

    file(GLOB_RECURSE JISOO_OBJECT_SYSTEM_TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests/CoreUObject/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests/CoreUObject/*.h"
    )
    file(GLOB_RECURSE JISOO_TEST_REFLECTION_HEADERS
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests/*.h"
    )

    execute_process(
        COMMAND
            "${Python3_EXECUTABLE}"
            "${JISOO_HEADER_GENERATOR}"
            --source-root "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
            --generated-root "${JISOO_TEST_GENERATED_ROOT}"
            --module JisooEngineTests
        RESULT_VARIABLE JISOO_TEST_REFLECTION_RESULT
        OUTPUT_VARIABLE JISOO_TEST_REFLECTION_OUTPUT
        ERROR_VARIABLE JISOO_TEST_REFLECTION_ERROR
    )

    if(NOT JISOO_TEST_REFLECTION_RESULT EQUAL 0)
        message(FATAL_ERROR
            "Failed to generate UObject test reflection code.\n"
            "${JISOO_TEST_REFLECTION_OUTPUT}${JISOO_TEST_REFLECTION_ERROR}"
        )
    endif()

    add_custom_target(JisooEngineTestReflectionCode
        COMMAND
            "${Python3_EXECUTABLE}"
            "${JISOO_HEADER_GENERATOR}"
            --source-root "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
            --generated-root "${JISOO_TEST_GENERATED_ROOT}"
            --module JisooEngineTests
        BYPRODUCTS "${JISOO_TEST_REFLECTION_HUB}"
        DEPENDS
            "${JISOO_HEADER_GENERATOR}"
            ${JISOO_TEST_REFLECTION_HEADERS}
        COMMENT "Generating UObject test metadata"
        VERBATIM
    )

    set_source_files_properties(
        "${JISOO_TEST_REFLECTION_HUB}"
        PROPERTIES GENERATED TRUE
    )

    add_executable(JisooObjectSystemTests
        ${JISOO_OBJECT_SYSTEM_TEST_SOURCES}
        "${JISOO_TEST_REFLECTION_HUB}"
    )
    add_dependencies(
        JisooObjectSystemTests
        JisooEngineReflectionCode
        JisooEngineTestReflectionCode
    )
    target_include_directories(JisooObjectSystemTests
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
            "${JISOO_TEST_GENERATED_ROOT}"
    )
    target_link_libraries(JisooObjectSystemTests PRIVATE JisooEngine::Runtime)
    jisoo_configure_target(JisooObjectSystemTests "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME JisooObjectSystem COMMAND JisooObjectSystemTests)
    add_test(
        NAME JisooHeaderGenerator
        COMMAND "${Python3_EXECUTABLE}" "${JISOO_HEADER_GENERATOR}" --self-test
    )

    source_group(
        TREE "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
        PREFIX "Tests"
        FILES ${JISOO_OBJECT_SYSTEM_TEST_SOURCES}
    )
    source_group("Generated" FILES "${JISOO_TEST_REFLECTION_HUB}")
    set_property(
        TARGET JisooObjectSystemTests JisooEngineTestReflectionCode
        PROPERTY FOLDER "JisooEngine/Tests"
    )
endif()
