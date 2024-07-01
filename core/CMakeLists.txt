cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(pwm-core)

if (PWM_CORE_LIBCRYPTO_STATIC)
	set(CORE_LIBS ${CORE_LIBS} "${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/libcrypto.lib")
else()
	set(CORE_LIBS ${CORE_LIBS} "${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/shared/libcrypto.lib")
endif()

add_library(core SHARED
	src/error.c
	src/pwm.cpp
)

target_include_directories(core PUBLIC "include")

target_link_libraries(core ${CORE_LIBS})