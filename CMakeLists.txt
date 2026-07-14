cmake_minimum_required(VERSION 3.20)
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(SKNanoAnalyzer VERSION 1.0.0 LANGUAGES CXX)

include(CTest)

# Apply one language contract to every subdirectory and generated dictionary.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BOOST_ROOT $ENV{CONDA_PREFIX})
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# find packages
find_package(ROOT REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(Boost REQUIRED)
find_package(hwy CONFIG REQUIRED)
find_package(Torch REQUIRED)
if(NOT correctionlib_DIR)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c
                "import pathlib, correctionlib; print(pathlib.Path(correctionlib.__file__).parent / 'cmake')"
        RESULT_VARIABLE SKNANO_CORRECTIONLIB_DISCOVERY_RESULT
        OUTPUT_VARIABLE SKNANO_CORRECTIONLIB_DISCOVERY_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    if(SKNANO_CORRECTIONLIB_DISCOVERY_RESULT EQUAL 0 AND
       EXISTS "${SKNANO_CORRECTIONLIB_DISCOVERY_PATH}")
        set(correctionlib_DIR "${SKNANO_CORRECTIONLIB_DISCOVERY_PATH}")
    endif()
endif()
find_package(correctionlib REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(VecCore QUIET)
find_package(Vc QUIET)

include(${ROOT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(SKNANO_USE_NATIVE_ARCH "Build with -march=native for same-machine local runs" OFF)
option(SKNANO_ENABLE_LTO "Enable interprocedural optimization in Release builds" OFF)
set(SKNANO_NUMERIC_MODE "StrictReference" CACHE STRING
    "Numeric policy: StrictReference or FastValidated")
set_property(CACHE SKNANO_NUMERIC_MODE PROPERTY STRINGS StrictReference FastValidated)
set(SKNANO_PGO_PHASE "OFF" CACHE STRING "PGO phase: OFF, GENERATE, or USE")
set_property(CACHE SKNANO_PGO_PHASE PROPERTY STRINGS OFF GENERATE USE)
set(SKNANO_PGO_PROFILE "${CMAKE_BINARY_DIR}/pgo-data" CACHE PATH
    "GCC profile directory or Clang merged .profdata path")

if(NOT SKNANO_NUMERIC_MODE STREQUAL "StrictReference" AND
   NOT SKNANO_NUMERIC_MODE STREQUAL "FastValidated")
    message(FATAL_ERROR "SKNANO_NUMERIC_MODE must be StrictReference or FastValidated")
endif()
if(NOT SKNANO_PGO_PHASE MATCHES "^(OFF|GENERATE|USE)$")
    message(FATAL_ERROR "SKNANO_PGO_PHASE must be OFF, GENERATE, or USE")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:-O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:MinSizeRel>>:-O3>
    )
    if(SKNANO_NUMERIC_MODE STREQUAL "FastValidated")
        add_compile_options(
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:-ffast-math>
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-ffast-math>
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:MinSizeRel>>:-ffast-math>
        )
        add_compile_definitions(SKNANO_FAST_VALIDATED=1)
    else()
        add_compile_definitions(SKNANO_STRICT_REFERENCE=1)
    endif()
    if(SKNANO_USE_NATIVE_ARCH)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-march=native>)
    endif()

    if(SKNANO_PGO_PHASE STREQUAL "GENERATE")
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-fprofile-generate=${SKNANO_PGO_PROFILE}
                                -fprofile-prefix-path=${CMAKE_BINARY_DIR})
            add_link_options(-fprofile-generate=${SKNANO_PGO_PROFILE})
        else()
            add_compile_options(-fprofile-instr-generate=${SKNANO_PGO_PROFILE}/default_%m.profraw)
            add_link_options(-fprofile-instr-generate=${SKNANO_PGO_PROFILE}/default_%m.profraw)
        endif()
    elseif(SKNANO_PGO_PHASE STREQUAL "USE")
        if(NOT EXISTS "${SKNANO_PGO_PROFILE}")
            message(FATAL_ERROR "SKNANO_PGO_PROFILE does not exist: ${SKNANO_PGO_PROFILE}")
        endif()
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-fprofile-use=${SKNANO_PGO_PROFILE}
                                -fprofile-prefix-path=${CMAKE_BINARY_DIR}
                                -fprofile-correction)
            add_link_options(-fprofile-use=${SKNANO_PGO_PROFILE} -fprofile-correction)
        else()
            add_compile_options(-fprofile-instr-use=${SKNANO_PGO_PROFILE})
            add_link_options(-fprofile-instr-use=${SKNANO_PGO_PROFILE})
        endif()
    endif()
endif()

if(SKNANO_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT SKNANO_IPO_SUPPORTED OUTPUT SKNANO_IPO_ERROR)
    if(NOT SKNANO_IPO_SUPPORTED)
        message(FATAL_ERROR "LTO is not supported: ${SKNANO_IPO_ERROR}")
    endif()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
endif()

if(VecCore_FOUND)
    message(STATUS "VecCore detected: ${VecCore_VERSION}")
    add_compile_definitions(ROOT_USE_VECCORE)
    link_libraries(VecCore::VecCore)
endif()

if(Vc_FOUND)
    message(STATUS "Vc detected: ${Vc_VERSION}")
    add_compile_definitions(ROOT_USE_VC)
    link_libraries(Vc::Vc)
endif()

# Add subdirectories
add_subdirectory(external/KinematicFitter)
add_subdirectory(external/RoccoR)
add_subdirectory(SKNanoCore)
add_subdirectory(DataFormats)
add_subdirectory(AnalyzerTools)
add_subdirectory(Analyzers)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Fetch external dependencies
include(FetchContent)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG master
)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG v3.11.3)

FetchContent_MakeAvailable(yaml-cpp json)
