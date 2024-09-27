cmake_minimum_required(VERSION 3.5)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_executable( rawtoaces
    main.cpp
)

target_include_directories( rawtoaces
    PUBLIC
        ${AcesContainer_INCLUDE_DIRS}
)

target_link_libraries ( rawtoaces
    PUBLIC
        ${RAWTOACESLIB}
    INTERFACE
        Boost::headers
)

if ( LIBRAW_CONFIG_FOUND )
    target_link_libraries ( rawtoaces PUBLIC libraw::raw )
else ()
    target_link_directories(rawtoaces PUBLIC ${libraw_LIBRARY_DIRS} )
    target_link_libraries(rawtoaces PUBLIC ${libraw_LIBRARIES} ${libraw_LDFLAGS_OTHER} )
endif ()

install( TARGETS rawtoaces DESTINATION bin )
