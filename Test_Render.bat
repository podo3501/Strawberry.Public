@echo off
rem Build the runtime test solution (Debug x64) and run the render Agent tests (no window).
rem Usage: Test_Render.bat [gtest_filter] [gbv]   e.g. Test_Render.bat RGResourceIDAllocatorAgent.*
rem Default filter is *Agent.*. The old T tests (RenderServiceT etc.) open windows and are never run here.
rem User tests (visible windows, a human checks them) run with a filter: Test_Render.bat RenderServiceUser.*
rem Console shows errors only (English). The full build log is Testing\x64\build_render.log.
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
echo [%time:~0,8%] Building Runtime-Test.slnx (Debug x64)...
"%MSBUILD%" Runtime-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -nologo -clp:ErrorsOnly;NoSummary -flp:logfile=x64\build_render.log;verbosity=normal
if errorlevel 1 (
    echo Build failed. Full log: Testing\x64\build_render.log
    exit /b 1
)
echo [%time:~0,8%] Build succeeded. Running render Agent tests...

set "FILTER=%~1"
if not defined FILTER set "FILTER=*Agent.*"
rem GPU-based validation is OFF by default (it makes the run about 3x slower). Turn it on with a second argument: Test_Render.bat <filter> gbv
rem It is always off for User tests (visible windows on the real GPU): the validation layer crashes them.
if /i "%~2"=="gbv" set "STRAWBERRY_GPU_VALIDATION=1"
echo %FILTER% | findstr /i /c:"User" >nul
if not errorlevel 1 set "STRAWBERRY_GPU_VALIDATION="
"x64\Debug\DxRender-Test.exe" --gtest_filter=%FILTER% --gtest_brief=1
exit /b %errorlevel%
