ExeclExport.exe auto

copy Lua\Cows_RoomCFG.lua ..\..\Client\GameData_Cows\src\config
copy Lua\Cows_MultiLanguageCFG.lua ..\..\Client\GameData_Cows\src\config
copy Lua\Cows_CardsCFG.lua ..\..\Client\GameData_Cows\src\config
copy Lua\Cows_SoundCFG.lua ..\..\Client\GameData_Cows\src\config
copy Lua\Cows_HelpCFG.lua ..\..\Client\GameData_Cows\src\config
@echo off
if "%nopause%" == "true" (@echo on) else (pause)