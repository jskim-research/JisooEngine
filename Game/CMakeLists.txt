file(GLOB_RECURSE JISOO_GAME_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.inl"
)

add_executable(JisooGame WIN32 ${JISOO_GAME_SOURCES})

target_link_libraries(JisooGame
    PRIVATE
        JisooEngine
)

jisoo_configure_executable(JisooGame)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    PREFIX "Source"
    FILES ${JISOO_GAME_SOURCES}
)
