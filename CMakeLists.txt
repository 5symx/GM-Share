cmake_minimum_required(VERSION 3.24)
project(gms LANGUAGES C CXX CUDA)
enable_language(CUDA)

#set(SERVER_VERSION server)
set(SERVER_VERSION "server" CACHE STRING "Server version")
message(STATUS "SERVER_VERSION is set to: ${SERVER_VERSION}")

if (SERVER_VERSION STREQUAL "server_naive")
    add_definitions(-DUSE_NAIVE_CLIENT)
    message(STATUS "client headfile is set to: naive")
endif()

set(CMAKE_CUDA_COMPILER nvcc)
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-deprecated-declarations -Wno-comment -Wno-deprecated -Wno-maybe-uninitialized")

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

option(ENABLE_LOGGING "Enable logging" OFF)

link_directories(/usr/local/cuda/lib)
link_directories(/usr/local/cuda/lib64)
link_directories(/usr/local/cuda/targets/x86_64-linux/lib/stubs)

set(ENV{CUDNN_PATH} /usr/local/cuda/lib)

include(GNUInstallDirs)

include(FindCUDA/select_compute_arch)
CUDA_DETECT_INSTALLED_GPUS(INSTALLED_GPU_CCS_1)
string(REPLACE " " "" INSTALLED_GPU_CCS_1 ${INSTALLED_GPU_CCS_1})
string(REPLACE "." "" INSTALLED_GPU_CCS_1 ${INSTALLED_GPU_CCS_1})
string(SUBSTRING ${INSTALLED_GPU_CCS_1} 0 2 CUDA_ARCH_LIST)
message(CUDA_ARCH_LIST="${CUDA_ARCH_LIST}")

set(GMS_SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/gms/cuda_util.cpp
    ${PROJECT_SOURCE_DIR}/src/gms/util.cpp
    ${PROJECT_SOURCE_DIR}/src/gms/generated/cuda_api.cpp
)

set(GMS_SERVER_SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/gms/${SERVER_VERSION}.cpp
    ${PROJECT_SOURCE_DIR}/src/gms/generated/server.cpp
    ${PROJECT_SOURCE_DIR}/src/gms/cuda_launch.cpp
)

set(GMS_CLIENT_SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/gms/client.cpp
)

add_library(gms STATIC ${GMS_SRC_FILES})
target_include_directories(gms
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC  /usr/local/cuda/include
)

set(gms_client_targets "gms_client" "gms_client_local")

# gms_client_env_vars
set(gms_client_env_vars "ENABLE_LOGGING")

foreach(gms_client_target IN LISTS gms_client_targets)
    add_library(
        ${gms_client_target} SHARED
        ${PROJECT_SOURCE_DIR}/src/gms/preload/gms_client.cpp
        ${PROJECT_SOURCE_DIR}/src/gms/preload/generated/gms_client.cpp
        ${GMS_SRC_FILES}
        ${GMS_CLIENT_SRC_FILES}
    )

    # Loop over the strings using foreach
    foreach(env_var ${gms_client_env_vars})
        if(${env_var})
            target_compile_definitions(${gms_client_target} PRIVATE ${env_var}=${env_var})
        endif()
    endforeach()

    target_include_directories(${gms_client_target}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PUBLIC  /usr/local/cuda/include
    )

    target_compile_definitions(${gms_client_target} PRIVATE IS_CLIENT="TRUE")
endforeach()

target_compile_definitions(gms_client_local PRIVATE RUN_LOCALLY="TRUE")

add_executable(gms_server ${PROJECT_SOURCE_DIR}/src/gms/gms_server.cpp ${GMS_SERVER_SRC_FILES})

if(ENABLE_LOGGING)
    target_compile_definitions(gms_server PRIVATE ENABLE_LOGGING=${ENABLE_LOGGING})
endif()

if(MEASURE_PREEMPTION_LATENCY)
    target_compile_definitions(gms_server PRIVATE MEASURE_PREEMPTION_LATENCY=${MEASURE_PREEMPTION_LATENCY})
endif()

target_include_directories(gms_server
    PUBLIC /usr/local/cuda/include
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${FOLLY_INCLUDE_DIR}
)

target_link_libraries(gms_server PUBLIC
    cuda
    cudart
    cublas
    dl
    gms
    nvidia-ml
)

# Add an option to enable/disable this special build. It's off by default.
option(ENABLE_LLAMA_OVERRIDE "Enable building the llama_model_load_from_file override library" ON)

# Only proceed with this section if the user has enabled the option.
if(ENABLE_LLAMA_OVERRIDE)
    message(STATUS "Llama override library build is ENABLED.")

    # Define a variable for the path to the llama.cpp source directory.
    set(LLAMA_CPP_DIR "" CACHE PATH "Path to the root of the llama.cpp source directory")

    # Ensure the user has provided the path.
    if(NOT LLAMA_CPP_DIR OR NOT EXISTS "${LLAMA_CPP_DIR}/include/llama.h")
        message(FATAL_ERROR "When ENABLE_LLAMA_OVERRIDE is ON, you must set LLAMA_CPP_DIR to a valid llama.cpp source directory.")
    endif()
    message(STATUS "Override using llama.cpp source from: ${LLAMA_CPP_DIR}")

    # Define our new shared library target from its source file.
    # Make sure the path to override.cpp is correct.
    add_library(llama_override SHARED ${PROJECT_SOURCE_DIR}/preload/override.cpp)

    # Set properties for the library.
    set_target_properties(llama_override PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME "override" # Creates override.so instead of libllama_override.so
    )

    # Add the llama.cpp directory to the include path for this target only.
    target_include_directories(llama_override PRIVATE ${LLAMA_CPP_DIR}/include ${LLAMA_CPP_DIR}/ggml/include)


    target_link_libraries(llama_override PUBLIC dl)

    message(STATUS "The override library will be built as: ${PROJECT_SOURCE_DIR}/build/liboverride.so (or similar)")

endif()

add_subdirectory(scripts)
