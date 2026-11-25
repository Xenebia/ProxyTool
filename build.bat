@echo off
title ProxyTool - Compilation

echo ========================================
echo          Compilation de ProxyTool
echo ========================================
echo.

g++ main.cpp proxy.cpp -std=c++17 -mwindows -o ProxyTool.exe -lwininet -lshell32 -static-libgcc -static-libstdc++

if errorlevel 1 (
    echo.
    echo ERREUR : la compilation a echoue.
    pause
    exit /b 1
)

echo.
echo Compilation reussie !
echo Fichier cree : ProxyTool.exe
echo.
pause
