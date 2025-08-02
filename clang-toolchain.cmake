# clang-toolchain.cmake
set(CMAKE_C_COMPILER /opt/homebrew/opt/llvm@19/bin/clang-19)
set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm@19/bin/clang++)


# Get the SDK path
execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-path
    OUTPUT_VARIABLE SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Use it as sysroot
set(CMAKE_C_FLAGS "--sysroot=${SDK_PATH}" CACHE STRING "")
set(CMAKE_CXX_FLAGS "--sysroot=${SDK_PATH}" CACHE STRING "")