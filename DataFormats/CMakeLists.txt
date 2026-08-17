# Define the library with source files and the generated dictionary
file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cc)
add_library(DataFormats SHARED
    ${SOURCES}
    #${CMAKE_CURRENT_BINARY_DIR}/G__DataFormats.cxx
)

# Specify where the library should look for header files
target_include_directories(DataFormats PUBLIC
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../SKNanoCore/include>
    $<INSTALL_INTERFACE:include>
)

# Generate the ROOT dictionary before defining the library
file(GLOB HEADERS CONFIGURE_DEPENDS include/*.h)
foreach(HEADER ${HEADERS})
    file(RELATIVE_PATH REL_HEADER ${CMAKE_CURRENT_SOURCE_DIR} ${HEADER})
    list(APPEND REL_HEADERS ${REL_HEADER})
endforeach()
set(DICTIONARY_HEADERS ${REL_HEADERS})

ROOT_GENERATE_DICTIONARY(G__DataFormats
    ${DICTIONARY_HEADERS}
    MODULE DataFormats
    LINKDEF include/DataFormatsLinkDef.hpp
)


# Link ROOT libraries
target_link_libraries(DataFormats PUBLIC ${ROOT_LIBRARIES} SKNanoCore)

# Specify where to install the library
install(TARGETS DataFormats DESTINATION lib)

# Iinstall .rootmap and .pcm files to the lib
set(ROOTMAP "libDataFormats.rootmap")
set(PCM "libDataFormats_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ROOTMAP}" "${CMAKE_CURRENT_BINARY_DIR}/${PCM}" DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
