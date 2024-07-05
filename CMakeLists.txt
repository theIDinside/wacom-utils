cmake_minimum_required(VERSION 3.25)
project(wacom-utils LANGUAGES CXX C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string(TOUPPER ${CMAKE_BUILD_TYPE} WU_BUILD_TYPE)

set(COMMON_FLAGS "-msse2 -D__MMX__ -D__SSE__ -D__SSE2__ -march=native")
set(WU_DEBUG_FLAGS "-g3 -O0 -Wall -Wextra -fpermissive")
set(WU_TEST_FLAGS "${WU_DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_FLAGS} ${WU_DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_FLAGS} ${WU_RELEASE_FLAGS}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES src/main.cpp src/app.cpp src/selection.cpp src/wacom.cpp src/process.cpp)
add_executable(wu ${SOURCES})
target_link_libraries(wu X11)
set_target_properties(wu PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


if(WU_BUILD_TYPE STREQUAL "DEBUG")
  target_compile_definitions(wu PRIVATE WU_DEBUG=1)
elseif(WU_BUILD_TYPE STREQUAL "RELEASE")
  target_compile_definitions(wu PRIVATE WU_DEBUG=0)
endif()