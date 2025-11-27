cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

project(SKNanoAnalyzer VERSION 1.0.0)

set(BOOST_ROOT $ENV{CONDA_PREFIX})
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# find packages
find_package(ROOT REQUIRED)
find_package(Boost REQUIRED)
find_package(Torch REQUIRED)
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

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:-O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:MinSizeRel>>:-O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:-ffast-math>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-ffast-math>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:MinSizeRel>>:-ffast-math>
        $<$<COMPILE_LANGUAGE:CXX>:-march=native>
    )
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
add_subdirectory(DataFormats)
add_subdirectory(AnalyzerTools)
add_subdirectory(Analyzers)

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

#set cxx standard
set(CMAKE_CXX_STANDARD ${ROOT_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

