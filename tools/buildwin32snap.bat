@echo off
mkdir NeoStats3.0
cd NeoStats3.0
mkdir logs
mkdir data
mkdir modules
@copy ..\src\Release\NeoStats.exe .
@copy ..\modules\*.dll modules
@copy ..\data\*.* data
@copy ..\README .
@copy ..\COPYING .
@copy ..\src\win32\neostats.conf .
cd ..
"C:\Program Files\Pantaray\QSetup\Composer.exe" tools\NeoStats.qsp /Compile /Exit
move tools\NeoStats-Setup.exe tools\NeoStats-Setup-3-0-a3-%1.exe
