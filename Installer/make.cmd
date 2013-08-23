@echo off

REM Paths
set BuildPath=%temp%\WinUnionFS_Install
set Output=WinUnionFS_Installer.exe

REM Tools
set SevenZip="%ProgramFiles%\7-Zip\7z.exe"

if exist %BuildPath% rmdir /S /Q %BuildPath%
mkdir %BuildPath%

copy /V ..\bin\Release_x64\WinUnionFS.dll %BuildPath%\WinUnionFS64.dll
copy /V ..\bin\Release_Win32\WinUnionFS.dll %BuildPath%\WinUnionFS32.dll
copy /V installer.sfx %BuildPath%\installer.sfx
copy /V config.txt %BuildPath%\config.txt

pushd %BuildPath%
%SevenZip% a -t7z -mx=9 -m9=LZMA2 "Files.7z" WinUnionFS64.dll WinUnionFS32.dll
upx --ultra-brute installer.sfx
copy /b installer.sfx + config.txt + Files.7z Installer.exe
popd

move %BuildPath%\Installer.exe %Output%

rmdir /S /Q %BuildPath%
