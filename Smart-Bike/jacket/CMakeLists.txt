cmake_minimum_required (VERSION 2.8)
project (libfreespace-examples C)

set (PROJECT_VERSION_MAJOR 0)
set (PROJECT_VERSION_MINOR 7)
set (PROJECT_VERSION_PATCH 0)


add_subdirectory(../libfreespace libfreespace)


set(LIBFREESPACE_LIB_TYPE "${LIBFREESPACE_LIB_TYPE_DEFAULT}" CACHE INTERNAL "")

if (UNIX)
	if (NOT ANDROID)
		set(LIBFREESPACE_LIBRARIES ${LIBFREESPACE_LIBRARIES} m pthread)
	endif()
endif()

include_directories(${LIBFREESPACE_INCLUDE_DIRS})

set(APP_CTRL_SRCS src/appControlHandler.c src/appControlHandler.h)
set(MATH_SRCS src/math/quaternion.c src/math/quaternion.h src/math/vec3.h)

add_executable(motion_example src/motion_example.c ${APP_CTRL_SRCS})
target_link_libraries(motion_example ${LIBFREESPACE_LIBRARIES})


if (CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-Wall) # TODO  should really set cflags
endif()


