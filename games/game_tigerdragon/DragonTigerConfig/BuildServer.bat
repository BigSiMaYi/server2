ExeclExport.exe auto

FileCompare.exe .\Code\Server ..\..\server\games\game_dragon_tiger

copy Xml\*.xml ..\Config
@echo off
if "%nopause%" == "true" (@echo on) else (pause)