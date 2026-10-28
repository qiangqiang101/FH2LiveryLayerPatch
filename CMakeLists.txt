cmake_minimum_required(VERSION 3.20)
project(fh2_livery_layer_patch VERSION 1.0.0 LANGUAGES C CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(platform)   # fh2mod_platform (d3dcompiler_47.dll + DWrite.dll) and fh2mod_api

configure_file(cmake/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/version.h)

add_library(livery_layer_patch SHARED src/livery_layer_patch.cpp src/version.rc)
target_link_libraries(livery_layer_patch PRIVATE fh2mod_api)
target_include_directories(livery_layer_patch PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_compile_definitions(livery_layer_patch PRIVATE LIVERY_LAYER_PATCH_VERSION="${PROJECT_VERSION}"
                           WIN32_LEAN_AND_MEAN NOMINMAX)
set_target_properties(livery_layer_patch PROPERTIES OUTPUT_NAME LiveryLayerPatch PREFIX "")
if(MSVC)
    target_compile_options(livery_layer_patch PRIVATE /W4 /utf-8)
else()
    target_compile_options(livery_layer_patch PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
    target_link_options(livery_layer_patch PRIVATE -static "$<$<CONFIG:Release>:-s>")
endif()

# `cmake --build build --target dist`: build/dist laid out as it goes in the Azimuth folder.
# The platform ships only as DWrite.dll: Azimuth 1.0.3 doesn't load d3dcompiler_47.dll.
set(_dist ${CMAKE_CURRENT_BINARY_DIR}/dist)
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${_dist}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_dist}/fh2-mods
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:fh2mod_platform>/DWrite.dll ${_dist}/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:livery_layer_patch> ${_dist}/fh2-mods/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/platform/third_party/minhook/LICENSE.txt
            ${_dist}/LICENSE-MinHook.txt
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/dist-readme.txt ${_dist}/README-LiveryLayerPatch.txt
    DEPENDS livery_layer_patch fh2mod_platform
    VERBATIM)
