@echo off
rem Build the runtime test solution (Debug x64) and run the audio Agent tests (no sound, no audio device).
rem Usage: Test_Audio.bat [gtest_filter]   e.g. Test_Audio.bat VoicePoolAgent.*
rem Default filter is *Agent.*. AudioServiceUser (a human listens to it) is always excluded.
rem Console shows errors only (English). The full build log is Testing\x64\build_audio.log.
setlocal
set VSLANG=1033
set SDL_AUDIODRIVER=dummy
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
"%MSBUILD%" Runtime-Test.slnx -p:Configuration=Debug -p:Platform=x64 -m -nologo -clp:ErrorsOnly;NoSummary -flp:logfile=x64\build_audio.log;verbosity=normal
if errorlevel 1 (
    echo Build failed. Full log: Testing\x64\build_audio.log
    exit /b 1
)
echo [%time:~0,8%] Build succeeded. Running audio Agent tests...

set "FILTER=%~1"
if not defined FILTER set "FILTER=*Agent.*"
echo %FILTER% | findstr /c:"-" >nul
if errorlevel 1 (
    set "FILTER=%FILTER%-AudioServiceUser.*"
) else (
    set "FILTER=%FILTER%:AudioServiceUser.*"
)
rem Shard count: default 1 (one process). TEST_SHARDS overrides it; a run with a filter is never split.
set "SHARDS=1"
if defined TEST_SHARDS set "SHARDS=%TEST_SHARDS%"
if not "%~1"=="" set "SHARDS=1"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Testing\Run_Test_Shards.ps1" -Exe "x64\Debug\SDLAudio-Test.exe" -Name audio -Shards %SHARDS% -Filter "%FILTER%"
exit /b %errorlevel%
