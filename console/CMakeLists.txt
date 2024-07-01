cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(pwm-console)

add_executable(pwm
	main.cpp
	getopt.c
	multibyte.c
)

target_link_libraries(pwm core)