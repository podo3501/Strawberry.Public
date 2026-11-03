@echo off
rem Build the UI test solution (Debug x64) and run the tests.
rem Usage: Test_UI.bat [gtest_filter]   e.g. Test_UI.bat UserInterfaceT_Toggle.*
setlocal
cd /d "%~dp0Testing\Game"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSBUILD="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"
if not defined MSBUILD (
    echo MSBuild.exe not found.
    exit /b 1
)

"%MSBUILD%" UserInterface-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -v:m -nologo
if errorlevel 1 exit /b 1

set "FILTER=%~1"
if not defined FILTER set "FILTER=*"
"x64\Debug\UserInterface-Test.exe" --gtest_filter=%FILTER% --gtest_brief=1
exit /b %errorlevel%
