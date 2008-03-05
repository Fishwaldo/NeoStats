@echo off
IF EXIST tools\NeoStats-Setup*.exe del tools\NeoStats-Setup*.exe
"C:\Program Files\Pantaray\QSetup\Composer.exe" tools\NeoStats.qsp /Compile /Exit
move tools\NeoStats-Setup.exe tools\NeoStats-Setup-3-0-1-%1.exe
