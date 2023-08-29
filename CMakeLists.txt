cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(pwm)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	if( ${CMAKE_SYSTEM_VERSION} EQUAL 6.1 ) # Windows 7
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	endif()
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")


# Pwm core
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/core")

set(PWM_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/getopt.c")
if (WIN32)
	set(PWM_SRCS ${PWM_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/src/multibyte.c")
endif()

add_executable(pwm "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp" ${PWM_SRCS})
target_link_libraries(pwm core)

add_executable(pwmgui WIN32 "${CMAKE_CURRENT_SOURCE_DIR}/src/main_gui.cpp" ${PWM_SRCS})
target_link_libraries(pwmgui core)

install(TARGETS core pwm pwmgui)