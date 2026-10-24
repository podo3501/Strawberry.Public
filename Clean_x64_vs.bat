@echo off
cd /d "%~dp0"

for /d /r %%d in (x64) do (
    if exist "%%d\" (
        echo "%%d" | findstr /i /l /c:"\.git\" /c:"\external\" /c:"\vcpkg_installed\" >nul
        if errorlevel 1 (
            echo 삭제: %%d
            rd /s /q "%%d"
        )
    )
)

if exist ".vs" (
    echo 삭제: %cd%\.vs
    rd /s /q ".vs"
)

echo 완료.