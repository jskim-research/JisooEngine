file(GLOB_RECURSE JISOO_ENGINE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.inl"
)

add_library(JisooEngine STATIC ${JISOO_ENGINE_SOURCES})

target_include_directories(JisooEngine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

jisoo_configure_target(JisooEngine)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    PREFIX "Source"
    FILES ${JISOO_ENGINE_SOURCES}
)
