@echo off
rem Build the UI test solution (Debug x64) and run the tests.
rem Usage: Test_UI.bat [gtest_filter]   e.g. Test_UI.bat UserInterfaceT_Toggle.*
rem Console shows errors only (English). The full build log is x64\build_ui.log.
setlocal
set VSLANG=1033
cd /d "%~dp0Testing\Game"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSBUILD="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\amd64\MSBuild.exe`) do set "MSBUILD=%%i"
if not defined MSBUILD (
    echo MSBuild.exe not found.
    exit /b 1
)

if not exist x64 mkdir x64
echo [%time:~0,8%] Building UserInterface-Test (Debug x64)...
"%MSBUILD%" UserInterface-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -nologo -clp:ErrorsOnly;NoSummary -flp:logfile=x64\build_ui.log;verbosity=normal
if errorlevel 1 (
    echo Build failed. Full log: Testing\Game\x64\build_ui.log
    exit /b 1
)
echo [%time:~0,8%] Build succeeded. Running tests...

set "FILTER=%~1"
if not defined FILTER set "FILTER=*"
"x64\Debug\UserInterface-Test.exe" --gtest_filter=%FILTER% --gtest_brief=1
exit /b %errorlevel%
