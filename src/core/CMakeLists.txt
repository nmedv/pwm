cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(pwm_core)

file(GLOB_RECURSE CORE_SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if (WIN32)
	set(CORE_LIBS ws2_32.lib crypt32.lib)
endif()

if (PWM_CORE_LIBCRYPTO_STATIC)
	set(CORE_LIBS ${CORE_LIBS} "${CMAKE_SOURCE_DIR}/lib/win32/libcrypto.lib")
else()
	set(CORE_LIBS ${CORE_LIBS} "${CMAKE_SOURCE_DIR}/lib/win32/shared/libcrypto.lib")
endif()

add_library(core SHARED ${CORE_SRCS})
target_link_libraries(core ${CORE_LIBS})