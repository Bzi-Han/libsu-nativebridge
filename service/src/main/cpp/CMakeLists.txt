cmake_minimum_required(VERSION 3.12.0)

project(NativeBridgeHijack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-fno-rtti)
add_link_options(-s)

add_definitions(-DANDROID_ABI="${ANDROID_ABI}")

find_library(liblog log)

include_directories(
    includes
)

# Build native bridge hijack library
add_library(nativebridgehijack SHARED src/nativebridge/mian.cc)
target_link_libraries(nativebridgehijack ${liblog})
if(NOT TEST_BUILD)
set_target_properties(nativebridgehijack PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../assets/${ANDROID_ABI}"
)
endif()