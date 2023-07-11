@echo off

set OLD_CD=%CD%
cd %~dp0

if defined VCToolsVersion (
	echo.
) else (
	@"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall" x86
)

cmake --preset=win32_msvc_debug
cmake --build build\win32_msvc_debug

cd %OLD_CD%