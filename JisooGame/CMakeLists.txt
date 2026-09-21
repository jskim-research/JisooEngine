# Game libraries will be added when project-specific code is implemented.
set(JISOO_ENGINE_MAIN "${PROJECT_SOURCE_DIR}/JisooEngine/Source/main.cpp")

add_executable(JisooGame WIN32 ${JISOO_ENGINE_MAIN})

target_compile_definitions(JisooGame PRIVATE JISOO_WITH_EDITOR=0)

target_link_libraries(JisooGame
    PRIVATE
        JisooEngine::Runtime
)

jisoo_configure_executable(JisooGame "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(JisooGameEditor WIN32 ${JISOO_ENGINE_MAIN})

target_compile_definitions(JisooGameEditor PRIVATE JISOO_WITH_EDITOR=1)

target_link_libraries(JisooGameEditor
    PRIVATE
        JisooEngine::Runtime
        JisooEngine::Editor
)

jisoo_configure_executable(JisooGameEditor "${CMAKE_CURRENT_SOURCE_DIR}")

source_group(
    TREE "${PROJECT_SOURCE_DIR}/JisooEngine/Source"
    PREFIX "Engine/Source"
    FILES ${JISOO_ENGINE_MAIN}
)

set_property(TARGET JisooGame JisooGameEditor PROPERTY FOLDER "JisooGame")
