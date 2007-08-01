@echo off
call vsvars32.bat
c:\WINDOWS\Microsoft.NET\Framework\v2.0.50727\MSBuild.exe neostatslibs.sln /p:Configuration=Release
c:\WINDOWS\Microsoft.NET\Framework\v2.0.50727\MSBuild.exe neostats.sln /p:Configuration=Release
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
c:\WINDOWS\Microsoft.NET\Framework\v2.0.50727\MSBuild.exe neostatslibs.sln /p:Configuration=Release /t:Cleanrem 
c:\WINDOWS\Microsoft.NET\Framework\v2.0.50727\MSBuild.exe neostats.sln /p:Configuration=Release /t:Clean