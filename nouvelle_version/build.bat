@echo off
title ProxyTool - Compilation

echo ========================================
echo          Compilation de ProxyTool
echo ========================================
echo.

REM -static : integre toutes les bibliotheques MinGW (libstdc++, libgcc, libwinpthread)
REM dans l'exe, pour qu'il fonctionne sur un PC sans MinGW.
g++ main.cpp proxy.cpp -std=c++17 -static -mwindows -o ProxyTool.exe -lwininet -lshell32

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
echo Verification : la commande ci-dessous ne doit lister AUCUNE dll
echo libstdc++ / libgcc / libwinpthread :
echo   objdump -p ProxyTool.exe ^| findstr /i "DLL Name"
echo.
pause
