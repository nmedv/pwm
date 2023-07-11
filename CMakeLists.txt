cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(pwm)

file(GLOB_RECURSE SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (MINGW)
	set(CMAKE_EXE_LINKER_FLAGS -static)
endif()

set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_ROOT_DIR "C:/openssl-3.0.8")
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${OPENSSL_ROOT_DIR}/include")

target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto)