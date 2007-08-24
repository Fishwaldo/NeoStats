@echo off
IF EXIST tools\NeoStats-Setup*.exe del tools\NeoStats-Setup*.exe
IF NOT EXIST NeoStats3.0 mkdir NeoStats3.0
cd NeoStats3.0
IF NOT EXIST logs mkdir logs
IF NOT EXIST data mkdir data
IF NOT EXIST modules mkdir modules
@copy ..\NeoStats.exe .
@copy ..\modules\*.dll modules
@copy ..\data\*.* data
@copy ..\README .
@copy ..\COPYING .
@copy ..\src\win32\neostats.conf .
cd ..
"C:\Program Files\Pantaray\QSetup\Composer.exe" tools\NeoStats.qsp /Compile /Exit
move tools\NeoStats-Setup.exe tools\NeoStats-Setup-3-0-a4-%1.exe
