cmake_minimum_required(VERSION 2.8)
project(ddb_lyricbar)

find_package(PkgConfig REQUIRED)
find_package(GTK2 2.4 COMPONENTS gtk gtkmm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wconversion")
set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -std=c11   -Wall -Wextra -Wconversion")
pkg_check_modules(LIBXMLPP REQUIRED libxml++-2.6)
pkg_check_modules(GTK3              gtkmm-3.0)

set(SRC_LIST main.c ui.cpp utils.cpp)
set(CMAKE_SHARED_LIBRARY_PREFIX)

include_directories(${LIBXMLPP_INCLUDE_DIRS})

if(GTK2_FOUND)
    add_library(${PROJECT_NAME}_gtk2 SHARED ${SRC_LIST})
    target_include_directories(${PROJECT_NAME}_gtk2 PUBLIC ${GTK2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}_gtk2
        ${GTK2_LIBRARIES}
        ${LIBXMLPP_LIBRARIES}
    )
    install_targets(lib/deadbeef ${PROJECT_NAME}_gtk2)
endif()

if(GTK3_FOUND)
    add_library(${PROJECT_NAME}_gtk3 SHARED ${SRC_LIST})
    target_include_directories(${PROJECT_NAME}_gtk3 PUBLIC ${GTK3_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}_gtk3
        ${GTK3_LIBRARIES}
        ${LIBXMLPP_LIBRARIES}
    )
    install_targets(lib/deadbeef ${PROJECT_NAME}_gtk3)
endif()
