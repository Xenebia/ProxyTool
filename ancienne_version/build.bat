@echo off
title ProxyTool (console) - Compilation

echo ========================================
echo     Compilation de ProxyTool (console)
echo ========================================
echo.

REM -static : integre toutes les bibliotheques MinGW dans l'exe.
g++ main.cpp proxy.cpp -std=c++17 -static -o ProxyTool.exe -lwininet

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
