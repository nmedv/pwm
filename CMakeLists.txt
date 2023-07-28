cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(pwm)

file(GLOB_RECURSE SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (WIN32)
	set(LIBS
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/libcrypto.lib"
		ws2_32.lib
		crypt32.lib
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/console.lib")
endif()

# if (MINGW)
	# set(CMAKE_EXE_LINKER_FLAGS -static)
# endif()

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Link libcrypto library
target_link_libraries(${PROJECT_NAME} ${LIBS})


install(TARGETS ${PROJECT_NAME})