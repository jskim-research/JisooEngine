cmake_minimum_required(VERSION 3.24)

project(JisooEngine LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "JisooEngine currently supports Windows only.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "JisooEngine currently requires the Microsoft Visual C++ toolchain.")
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

include(CTest)

function(jisoo_configure_target TARGET_NAME PRODUCT_ROOT)
    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)

    target_compile_definitions(${TARGET_NAME}
        PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            UNICODE
            _UNICODE
    )

    target_compile_options(${TARGET_NAME}
        PRIVATE
            /W4
            /permissive-
            /utf-8
            /Zc:__cplusplus
            $<$<CONFIG:Release>:/Zi>
    )

    set_target_properties(${TARGET_NAME}
        PROPERTIES
            COMPILE_PDB_NAME "${TARGET_NAME}"
    )

    foreach(CONFIGURATION Debug Release)
        string(TOUPPER "${CONFIGURATION}" CONFIGURATION_UPPER)

        set_target_properties(${TARGET_NAME}
            PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${CONFIGURATION_UPPER}
                    "${PRODUCT_ROOT}/Binaries/Win64/${CONFIGURATION}"
                LIBRARY_OUTPUT_DIRECTORY_${CONFIGURATION_UPPER}
                    "${PRODUCT_ROOT}/Binaries/Win64/${CONFIGURATION}"
                ARCHIVE_OUTPUT_DIRECTORY_${CONFIGURATION_UPPER}
                    "${PRODUCT_ROOT}/Intermediate/Libraries/Win64/${CONFIGURATION}"
                PDB_OUTPUT_DIRECTORY_${CONFIGURATION_UPPER}
                    "${PRODUCT_ROOT}/Binaries/Win64/${CONFIGURATION}"
                COMPILE_PDB_OUTPUT_DIRECTORY_${CONFIGURATION_UPPER}
                    "${PRODUCT_ROOT}/Intermediate/Symbols/Win64/${CONFIGURATION}"
        )
    endforeach()
endfunction()

function(jisoo_configure_executable TARGET_NAME PRODUCT_ROOT)
    jisoo_configure_target(${TARGET_NAME} "${PRODUCT_ROOT}")

    set_target_properties(${TARGET_NAME}
        PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    target_link_options(${TARGET_NAME}
        PRIVATE
            $<$<CONFIG:Release>:/DEBUG:FULL>
    )
endfunction()

add_subdirectory(
    JisooEngine
    "${CMAKE_SOURCE_DIR}/JisooEngine/Intermediate/Build/VS2022-x64"
)
add_subdirectory(
    JisooGame
    "${CMAKE_SOURCE_DIR}/JisooGame/Intermediate/Build/VS2022-x64"
)

set_property(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    PROPERTY VS_STARTUP_PROJECT JisooGameEditor
)
