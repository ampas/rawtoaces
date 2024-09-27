cmake_minimum_required(VERSION 3.5)
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )

add_library ( rawtoaces_util2 ${DO_SHARED}
    rawtoaces_util.cpp
    ${PROJECT_SOURCE_DIR}/include/rawtoaces/rawtoaces_util.h
)

set_property(TARGET rawtoaces_util2 PROPERTY CXX_STANDARD 17)

if ( OIIO_FOUND )
    target_include_directories ( rawtoaces_util2 PRIVATE ${OIIO_INCLUDE_DIRS} )
    target_link_directories    ( rawtoaces_util2 PUBLIC  ${OIIO_LIBRARY_DIRS} )
    target_link_libraries      ( rawtoaces_util2
        PUBLIC
            ${OIIO_LIBRARIES}
            ${OIIO_LDFLAGS_OTHER}
    )
endif()
 
target_link_libraries ( rawtoaces_util2
    PUBLIC
        ${RAWTOACESIDTLIB}
        OpenImageIO::OpenImageIO
)
    
set_target_properties( rawtoaces_util2 PROPERTIES
    SOVERSION ${RAWTOACES_MAJOR_VERSION}.${RAWTOACES_MINOR_VERSION}.${RAWTOACES_PATCH_VERSION}
    VERSION ${RAWTOACES_VERSION} )

install(
    FILES
        ${PROJECT_SOURCE_DIR}/include/rawtoaces/rawtoaces_util.h	 	
    DESTINATION include/rawtoaces
)

install( TARGETS rawtoaces_util2 DESTINATION lib )
