file(GLOB_RECURSE JISOO_EDITOR_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.inl"
)

add_executable(JisooEditor WIN32 ${JISOO_EDITOR_SOURCES})

target_link_libraries(JisooEditor
    PRIVATE
        JisooEngine
)

jisoo_configure_executable(JisooEditor)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    PREFIX "Source"
    FILES ${JISOO_EDITOR_SOURCES}
)
