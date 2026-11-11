@echo off
rem Build the Foundation test solution (Debug x64) and run Core-Test and Serialization-Test.
rem Usage: Test_Foundation.bat [gtest_filter]   e.g. Test_Foundation.bat AssetPathAgent.*
rem Console shows errors only (English). The full build log is Testing\x64\build_foundation.log.
setlocal
set VSLANG=1033
cd /d "%~dp0Testing"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSBUILD="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\amd64\MSBuild.exe`) do set "MSBUILD=%%i"
if not defined MSBUILD (
    echo MSBuild.exe not found.
    exit /b 1
)

if not exist x64 mkdir x64
echo [%time:~0,8%] Building Foundation-Test.slnx (Debug x64)...
"%MSBUILD%" Foundation-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -nologo -clp:ErrorsOnly;NoSummary -flp:logfile=x64\build_foundation.log;verbosity=normal
if errorlevel 1 (
    echo Build failed. Full log: Testing\x64\build_foundation.log
    exit /b 1
)
echo [%time:~0,8%] Build succeeded. Running tests...

set "FILTER=%~1"
if not defined FILTER set "FILTER=*"
set "RESULT=0"
for %%t in (Core-Test Serialization-Test) do (
    echo [%%t]
    "x64\Debug\%%t.exe" --gtest_filter=%FILTER% --gtest_brief=1
    if errorlevel 1 set "RESULT=1"
)
exit /b %RESULT%
