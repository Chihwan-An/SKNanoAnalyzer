add_library(SKNanoCore STATIC
    src/HighwayKernels.cc
)

set_target_properties(SKNanoCore PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(SKNanoCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(SKNanoCore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(SKNanoCore PUBLIC hwy::hwy)

install(DIRECTORY include/ DESTINATION include)
install(TARGETS SKNanoCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
