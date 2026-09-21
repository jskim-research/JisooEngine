file(GLOB_RECURSE JISOO_ENGINE_RUNTIME_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.inl"
)

add_library(JisooEngineRuntime STATIC ${JISOO_ENGINE_RUNTIME_SOURCES})
add_library(JisooEngine::Runtime ALIAS JisooEngineRuntime)

target_include_directories(JisooEngineRuntime
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

jisoo_configure_target(JisooEngineRuntime "${CMAKE_CURRENT_SOURCE_DIR}")

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    PREFIX "Source"
    FILES ${JISOO_ENGINE_RUNTIME_SOURCES}
)

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

set_property(TARGET JisooEngineRuntime JisooEngineEditor PROPERTY FOLDER "JisooEngine")
