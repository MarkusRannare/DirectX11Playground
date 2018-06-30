@echo off

:: Available platforms
set Platforms=Win32 Win64

:: Same as "cd ..", but we can go back to this directory with popd
pushd ..
:: Create directory structure for build
call :CreateIfNotExist Intermediate
cd Intermediate
call :CreateIfNotExist Build
cd Build
set BuildDir=%cd%
:: Enable of usage of variables in for-loops
Setlocal EnableDelayedExpansion
:: Iterate over all platforms
FOR %%G IN (%Platforms%) DO (
	set Platform=%%G

	:: Create platform folder
	call :CreateIfNotExist !Platform!
 	cd !Platform!

	:: Remove Win32 from Platform, as cmake doesn't want Win32 passed in 
	set CMakePlatform= !Platform!
	IF !Platform! == Win32 (
		set CMakePlatform=
	)

	:: Create solution
	cmake ../../../Source/ -G "Visual Studio 14 2015!CMakePlatform!"
	
	cd !BuildDir!

	:: %~dp0 is the path to the batch file
	:: Create a shortcut to the solution
	set BatchFileLocation=%~dp0
	set LinkFile="!BatchFileLocation!\MoGET !Platform!.lnk"
	IF EXIST LinkFile (
		del "!LinkFile!"
	)
	call :CreateShortcut !LinkFile! "!BuildDir!\!Platform!\MoGET.sln" !Platform!
)
Endlocal

popd
pause

:: Create a folder if one doesn't exist
:CreateIfNotExist <Folder>
	:: @todo: Verify that we have passed a parameter to function
	:: Append a slash to the directory
	set Directory=%~1\
	IF NOT EXIST %Directory% (
		mkdir %~1
	)
	EXIT /B 0

:: Create a shortcut LinkFile->Target on with the name MoGET <Platform>
:CreateShortcut <LinkFile> <Target> <Platform>
	set LinkFile=%~1
	set Target=%~2
	set Platform=%~3
	call :PathFromFileName WorkingDirectory %Target%
	echo Set oWS = WScript.CreateObject("WScript.Shell") > CreateShortcut.vbs
	echo sLinkFile = "%LinkFile%" >> CreateShortcut.vbs
	echo Set oLink = oWS.CreateShortcut(sLinkFile) >> CreateShortcut.vbs
	echo oLink.TargetPath = "%Target%" >> CreateShortcut.vbs
	echo oLink.WorkingDirectory = "%WorkingDirectory%" >> CreateShortcut.vbs
	echo oLink.Description = "MoGET !Platform!" >> CreateShortcut.vbs
	echo oLink.IconLocation = "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" >> CreateShortcut.vbs
	echo oLink.Save >> CreateShortcut.vbs
	cscript CreateShortcut.vbs
	del CreateShortcut.vbs
	EXIT /B 0

:PathFromFileName <ResultVar> <PathVar>
    set "%~1=%~dp2"
    exit /B