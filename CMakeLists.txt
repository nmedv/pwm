cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(pwm-project)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	if( ${CMAKE_SYSTEM_VERSION} EQUAL 6.1 ) # Windows 7
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	endif()
endif()


add_subdirectory("core")
add_subdirectory("console")

install(TARGETS core pwm)