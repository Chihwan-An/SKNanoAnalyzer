# find LHAPDF and CorrectionLib
set(CORRECTION_INCLUDE_DIR $ENV{CORRECTION_INCLUDE_DIR})
set(LHAPDF_INCLUDE_DIR $ENV{LHAPDF_INCLUDE_DIR})

set(SKNANO_BRANCH_SCHEMA
    ${CMAKE_SOURCE_DIR}/data/Run3_v15_Run2_v15/NanoAODv15.json)
set(SKNANO_BRANCH_GENERATOR
    ${CMAKE_SOURCE_DIR}/python/generate_branch_code.py)
set(SKNANO_BRANCH_OVERLAY
    ${CMAKE_SOURCE_DIR}/data/Run3_v15_Run2_v15/branch_overlay.json)
set(SKNANO_BRANCH_ADDONS
    ${CMAKE_SOURCE_DIR}/data/Run3_v15_Run2_v15/custom/CustomBPHBranches.json)
set(SKNANO_GENERATED_INCLUDE_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(SKNANO_GENERATED_BRANCH_FILES
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_branch_decls.inc
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_branch_register.inc
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_branch_reset.inc
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_input_columns.inc
    ${SKNANO_GENERATED_INCLUDE_DIR}/generated_loader_api.inc)

add_custom_command(
    OUTPUT ${SKNANO_GENERATED_BRANCH_FILES}
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${SKNANO_GENERATED_INCLUDE_DIR}
    COMMAND ${Python3_EXECUTABLE} ${SKNANO_BRANCH_GENERATOR}
            --json ${SKNANO_BRANCH_SCHEMA}
            --overlay ${SKNANO_BRANCH_OVERLAY}
            --addon ${SKNANO_BRANCH_ADDONS}
            --out-dir ${SKNANO_GENERATED_INCLUDE_DIR}
    DEPENDS ${SKNANO_BRANCH_GENERATOR} ${SKNANO_BRANCH_SCHEMA}
            ${SKNANO_BRANCH_OVERLAY} ${SKNANO_BRANCH_ADDONS}
    COMMENT "Generating canonical NanoAOD branch bindings"
    VERBATIM
)
add_custom_target(SKNanoBranchCodegen
    DEPENDS ${SKNANO_GENERATED_BRANCH_FILES})

# Define the library with source files and the generated dictionary
file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cc)
add_library(Analyzers SHARED
    ${SOURCES}
    #${CMAKE_CURRENT_BINARY_DIR}/G__Analyzers.cxx
)
add_dependencies(Analyzers SKNanoBranchCodegen)

target_include_directories(Analyzers PUBLIC
    $<BUILD_INTERFACE:${SKNANO_GENERATED_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../DataFormats/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../SKNanoCore/include>
    $<INSTALL_INTERFACE:include>
PRIVATE
    $<BUILD_INTERFACE:${LHAPDF_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CORRECTION_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../AnalyzerTools/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../external/KinematicFitter/include>
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
)

# Generate the ROOT dictionary before defining the library
file(GLOB HEADERS CONFIGURE_DEPENDS include/*.h)
foreach(HEADER ${HEADERS})
    file(RELATIVE_PATH REL_HEADER ${CMAKE_CURRENT_SOURCE_DIR} ${HEADER})
    list(APPEND REL_HEADERS ${REL_HEADER})
endforeach()
set(DICTIONARY_HEADERS ${REL_HEADERS})

ROOT_GENERATE_DICTIONARY(G__Analyzers 
    ${DICTIONARY_HEADERS}
    MODULE Analyzers
    LINKDEF include/AnalyzersLinkDef.hpp
    EXTRA_DEPENDENCIES ${SKNANO_GENERATED_BRANCH_FILES}
)



# Link ROOT libraries
target_link_libraries(Analyzers PUBLIC ${ROOT_LIBRARIES})
target_link_libraries(Analyzers PUBLIC DataFormats)
target_link_libraries(Analyzers PUBLIC AnalyzerTools)
target_link_libraries(Analyzers PUBLIC SKNanoCore)
target_link_libraries(Analyzers PUBLIC KinFitter)
#target_link_libraries(Analyzers PUBLIC MLHelper)
target_link_libraries(Analyzers PUBLIC absl::flat_hash_map absl::strings absl::hash)
target_link_libraries(Analyzers PUBLIC nlohmann_json::nlohmann_json)

# Specify where to install the library
install(TARGETS Analyzers DESTINATION lib)


# Iinstall .rootmap and .pcm files to the lib
set(ROOTMAP "libAnalyzers.rootmap")
set(PCM "libAnalyzers_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ROOTMAP}" "${CMAKE_CURRENT_BINARY_DIR}/${PCM}" DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)
install(FILES ${SKNANO_GENERATED_BRANCH_FILES} DESTINATION include)
