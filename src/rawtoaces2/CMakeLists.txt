cmake_minimum_required(VERSION 3.5)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_executable( rawtoaces2
    main.cpp
)

set_property(TARGET rawtoaces2 PROPERTY CXX_STANDARD 17)

target_include_directories( rawtoaces2
    PUBLIC
        ${OpenImageIO_INCLUDE_DIR}
)

target_link_libraries ( rawtoaces2
    PUBLIC
#        OpenImageIO::OpenImageIO
        rawtoaces_util2
)

install( TARGETS rawtoaces2 DESTINATION bin )
