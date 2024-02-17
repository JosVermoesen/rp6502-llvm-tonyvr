cmake_minimum_required(VERSION 3.18)
add_subdirectory(tools)
set(LLVM_MOS_PLATFORM rp6502)
find_package(llvm-mos-sdk REQUIRED)
project(MY-RP6502-PROJECT)

add_executable(txt_test_cpp)
rp6502_executable(txt_test_cpp)
target_sources(txt_test_cpp PRIVATE
    src/txt_test.cpp
    src/TxtDisplay.cpp
    src/TxtKeyboard.cpp
    src/TxtMouse.cpp
)
