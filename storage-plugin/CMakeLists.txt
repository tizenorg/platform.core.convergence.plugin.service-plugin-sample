cmake_minimum_required(VERSION 2.8)
project(storage-plugin-sample C CXX)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/storage-plugin-sample.c)

SET(PLUGIN_LIBDIR "${LIB_INSTALL_DIR}/service-provider/storage")
MESSAGE("libdir install dir ${LIB_INSTALL_DIR}")
MESSAGE("plugin libdir ${PLUGIN_LIBDIR}")

INCLUDE(FindPkgConfig)
pkg_check_modules(storage_pkgs REQUIRED 
	dlog
	glib-2.0
	service-adaptor
	)

FOREACH(flag ${storage_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${storage_pkgs_LDFLAGS})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGIN_LIBDIR} COMPONENT RuntimeLibraries)
