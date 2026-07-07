cmake_minimum_required(VERSION 3.15)

# Project name
project(P-RTLog C CXX ASM)

# C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -fno-exceptions -fno-rtti")

# Include paths
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/p-span
    ${CMAKE_SOURCE_DIR}/src/log_tokenized
    ${CMAKE_SOURCE_DIR}/src/log_tokenized/backend/RTT
    ${CMAKE_SOURCE_DIR}/third_party/pigweed/pw_tokenizer/pw_tokenizer/public
    ${CMAKE_SOURCE_DIR}/third_party/pigweed/pw_log_tokenized/public
    ${CMAKE_SOURCE_DIR}/third_party/pigweed/pw_log_tokenized/public_overrides
    ${CMAKE_SOURCE_DIR}/third_party/pigweed/pw_log_tokenized/light_public_overrides
    ${CMAKE_SOURCE_DIR}/third_party/pigweed/pw_log_tokenized/gcc_partially_tokenized_public_overrides
    ${CMAKE_SOURCE_DIR}/third_party/pigweed/pw_log_tokenized/metadata_public_overrides
    ${CMAKE_SOURCE_DIR}/third_party/segger_rtt/RTT
    ${CMAKE_SOURCE_DIR}/third_party/segger_rtt/Config
    ${CMAKE_SOURCE_DIR}/stubs
)

# Compile definitions
add_compile_definitions(
    P_TOKENIZER_CFG_ARG_TYPES_SIZE_BYTES=4
    P_TOKENIZER_CFG_C_HASH_LENGTH=128
    USING_RTT_BACKEND
)

# Source files
set(P_RTLOG_SOURCES
    # Tokenized logging (C++)
    # src/log_tokenized/log_tokenized_light.cc
    
    # Tokenized logging (C version - optional, comment out if using C++ version)
    src/log_tokenized/plog_tokenized_light.c
    
    # RTT backend
    src/log_tokenized/backend/RTT/rtt_backend.c
    
    # SEGGER RTT
    third_party/segger_rtt/RTT/SEGGER_RTT.c
    # third_party/segger_rtt/RTT/SEGGER_RTT_printf.c  # Optional: printf support
)

# Create static library (for testing compilation)
add_library(p_rtlog STATIC ${P_RTLOG_SOURCES})

# Optional: Create a test executable
# add_executable(p_rtlog_test 
#     test/main.c
#     ${P_RTLOG_SOURCES}
# )

# Print build info
message(STATUS "P-RTLog build configuration:")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
