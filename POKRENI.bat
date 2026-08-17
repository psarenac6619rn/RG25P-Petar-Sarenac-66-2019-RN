@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SciFiOutpost Launcher
cls
echo ==========================================
echo       SciFiOutpost - Build ^& Run
echo ==========================================
echo.
echo Git i vcpkg NISU potrebni.
echo Pri prvom pokretanju potreban je internet da CMake preuzme GLFW i GLEW.
echo.
if exist "build\Release\SciFiOutpost.exe" goto RUN_RELEASE
if exist "build\SciFiOutpost.exe" goto RUN_SINGLE
set "CMAKE_EXE="
where cmake >nul 2>nul
if not errorlevel 1 set "CMAKE_EXE=cmake"
if not defined CMAKE_EXE if exist "%ProgramFiles%\CMake\bin\cmake.exe" set "CMAKE_EXE=%ProgramFiles%\CMake\bin\cmake.exe"
if not defined CMAKE_EXE if exist "%ProgramFiles(x86)%\CMake\bin\cmake.exe" set "CMAKE_EXE=%ProgramFiles(x86)%\CMake\bin\cmake.exe"
if not defined CMAKE_EXE (
    echo [GRESKA] CMake nije pronadjen.
    echo.
    echo Instaliraj CMake za Windows, zatim ponovo pokreni POKRENI.bat.
    echo Takodje je potreban Visual Studio 2022 ili Build Tools sa C++ komponentama.
    echo.
    pause
    exit /b 1
)
echo [1/3] CMake konfiguracija...
"%CMAKE_EXE%" -S . -B build -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if errorlevel 1 goto FAIL
echo.
echo [2/3] Build Release...
"%CMAKE_EXE%" --build build --config Release --parallel
if errorlevel 1 goto FAIL
echo.
echo [3/3] Build zavrsen.
:RUN_RELEASE
if exist "build\Release\SciFiOutpost.exe" (
    echo Pokrecem SciFiOutpost...
    echo.
    pushd "build\Release"
    SciFiOutpost.exe
    set "APP_ERROR=%ERRORLEVEL%"
    popd
    if not "%APP_ERROR%"=="0" (
        echo.
        echo Program je zavrsen sa kodom %APP_ERROR%.
        pause
    )
    exit /b %APP_ERROR%
)
:RUN_SINGLE
if exist "build\SciFiOutpost.exe" (
    echo Pokrecem SciFiOutpost...
    echo.
    pushd "build"
    SciFiOutpost.exe
    set "APP_ERROR=%ERRORLEVEL%"
    popd
    if not "%APP_ERROR%"=="0" pause
    exit /b %APP_ERROR%
)
echo [GRESKA] SciFiOutpost.exe nije pronadjen nakon build-a.
pause
exit /b 1
:FAIL
echo.
echo ==========================================
echo BUILD NIJE USPEO
echo ==========================================
echo.
echo Najcesci razlozi:
echo  - nije instaliran Visual Studio / Build Tools sa C++ podrskom
 echo  - nema internet veze pri prvom pokretanju
 echo  - antivirus/firewall blokira CMake download
 echo.
echo Posalji screenshot cele greske ako se ovo pojavi.
pause
exit /b 1
