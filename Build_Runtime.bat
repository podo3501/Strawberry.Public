@echo off
rem Build only (no tests) the runtime test solution (Debug x64) to check that everything compiles.
rem Usage: Build_Runtime.bat
rem Console shows errors only (English). The full build log is x64\build_runtime.log.
setlocal
set VSLANG=1033
cd /d "%~dp0Testing"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSBUILD="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"
if not defined MSBUILD (
    echo MSBuild.exe not found.
    exit /b 1
)

if not exist x64 mkdir x64
"%MSBUILD%" Runtime-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -nologo -clp:ErrorsOnly;NoSummary -flp:logfile=x64\build_runtime.log;verbosity=normal
if errorlevel 1 (
    echo Build failed. Full log: Testing\x64\build_runtime.log
    exit /b 1
)

echo Build succeeded.
exit /b 0
