::[Bat To Exe Converter]
::
::YAwzoRdxOk+EWAnk
::fBw5plQjdG8=
::YAwzuBVtJxjWCl3EqQJgSA==
::ZR4luwNxJguZRRnk
::Yhs/ulQjdF+5
::cxAkpRVqdFKZSDk=
::cBs/ulQjdF+5
::ZR41oxFsdFKZSDk=
::eBoioBt6dFKZSDk=
::cRo6pxp7LAbNWATEpCI=
::egkzugNsPRvcWATEpCI=
::dAsiuh18IRvcCxnZtBJQ
::cRYluBh/LU+EWAnk
::YxY4rhs+aU+JeA==
::cxY6rQJ7JhzQF1fEqQJQ
::ZQ05rAF9IBncCkqN+0xwdVs0
::ZQ05rAF9IAHYFVzEqQJQ
::eg0/rx1wNQPfEVWB+kM9LVsJDGQ=
::fBEirQZwNQPfEVWB+kM9LVsJDGQ=
::cRolqwZ3JBvQF1fEqQJQ
::dhA7uBVwLU+EWDk=
::YQ03rBFzNR3SWATElA==
::dhAmsQZ3MwfNWATElA==
::ZQ0/vhVqMQ3MEVWAtB9wSA==
::Zg8zqx1/OA3MEVWAtB9wSA==
::dhA7pRFwIByZRRnk
::Zh4grVQjdCuDJFON9U06IR5oQQuJAGa7FaEZ+tT27vmTp19MaO8+a5veyIiKIfQS1kflYaoP125ul+4UCThXcASufBsxuz0QijXLMt+Z0w==
::YB416Ek+ZG8=
::
::
::978f952a14a936cc963da21a135fa983
::[Bat To Exe Converter]
::
::YAwzoRdxOk+EWAnk
::fBw5plQjdG8=
::YAwzuBVtJxjWCl3EqQJgSA==
::ZR4luwNxJguZRRnk
::Yhs/ulQjdF+5
::cxAkpRVqdFKZSDk=
::cBs/ulQjdF+5
::ZR41oxFsdFKZSDk=
::eBoioBt6dFKZSDk=
::cRo6pxp7LAbNWATEpCI=
::egkzugNsPRvcWATEpCI=
::dAsiuh18IRvcCxnZtBJQ
::cRYluBh/LU+EWAnk
::YxY4rhs+aU+JeA==
::cxY6rQJ7JhzQF1fEqQJQ
::ZQ05rAF9IBncCkqN+0xwdVs0
::ZQ05rAF9IAHYFVzEqQJQ
::eg0/rx1wNQPfEVWB+kM9LVsJDGQ=
::fBEirQZwNQPfEVWB+kM9LVsJDGQ=
::cRolqwZ3JBvQF1fEqQJQ
::dhA7uBVwLU+EWDk=
::YQ03rBFzNR3SWATElA==
::dhAmsQZ3MwfNWATElA==
::ZQ0/vhVqMQ3MEVWAtB9wSA==
::Zg8zqx1/OA3MEVWAtB9wSA==
::dhA7pRFwIByZRRnk
::Zh4grVQjdCuDJF2N51YnOyd5QwyDP2C/FPsZ8O2b
::YB416Ek+ZG8=
::
::
::978f952a14a936cc963da21a135fa983
@echo off
%test bat command%
::test comment
@title test bat command
::dir .\*.txt /b > txt.log
set today=%Date%
@echo %today%

@echo step1
rem set update_dir=%~dp0..\otawork\otawork\download\update
rem set update_bin_dir=%~dp0..\otawork\otawork\download\update\newpackage\bin
rem set update_dir=""
rem set update_bin_dir=""

@echo step2
set current_bin_dir=%~dp0bin
set current_binbak_dir=%~dp0binbak
@echo step3
set current_bin_version=%~dp0bin\version.json
set current_bin_main=%~dp0bin\MoldAI.exe
@echo step4

  echo batchfile=%0
  echo full=%~f0
setlocal
  for %%d in (%~dp0.) do set Directory=%%~fd
  echo Directory=%Directory%

  set update_dir=%Directory%\otawork\otawork\download\update
  set update_bin_dir=%Directory%\otawork\otawork\download\update\newpackage\bin

  for %%d in (%~dp0..) do set ParentDirectory=%%~fd
  echo ParentDirectory=%ParentDirectory%

echo %Directory%
echo %ParentDirectory%

set update_dir=%ParentDirectory%\otawork\download\update
set update_bin_dir=%ParentDirectory%\otawork\download\update\newpackage\bin

@echo step45
rem echo %update_dir%
rem echo %update_bin_dir%

echo %update_dir%
echo %update_bin_dir%

rem setlocal
rem set ParentDir=%~p1
rem set ParentDir=%ParentDir: =:%
rem set ParentDir=%ParentDir:\= %
rem call :getparentdir %ParentDir%
rem set ParentDir=%ParentDir::= %

rem echo ParentDir is %ParentDir%



@echo step5


if not exist %update_dir% (
	echo no update dir,don't need to update.	
	goto exec_current_bin
) else (
	echo has update dir,need to update.		
	if not exist %update_bin_dir% (
		echo no update bin dir,don't need to update.	
		goto exec_current_bin
	) else (
		echo has update bin dir,need to update.	
		if not exist %current_bin_dir% (
			echo current bin not exists.
			goto exec_current_bin
		) else (
			echo current bin exists.

			if exist %current_binbak_dir% (
				@echo step41
				echo remove binbak dir.
				@echo step42
				rd %current_binbak_dir% /s /q	
	
				if exist %current_binbak_dir% (
					echo remove binbak dir failed.
					goto exec_current_bin
				)
			)


			move %current_bin_dir% %current_binbak_dir%

			if exist %current_bin_dir% (
				echo rename current bin dir failed.
				goto exec_current_bin
			)

			move %update_bin_dir% .\	
			if exist %update_bin_dir% (
				echo update bin dir still exists,failed.
				goto exec_current_bin
			)

			if not exist %current_bin_dir% (
				echo current bin dir not exists,move update bin dir failed.
				goto exec_current_bin
			)

			if not exist %current_bin_version% (
				echo current bin dir has not version.json,move update bin dir failed.
				goto exec_current_bin
			)



		)

	)
) 

:exec_current_bin
::pause
::exit
if not exist %current_bin_main% (
	echo current bin dir's mohakcer not exists.
) else (
	echo boot app started.
	start %current_bin_main%
)

endlocal
goto :EOF

:getparentdir
if "%~1" EQU "" goto :EOF
Set ParentDir=%~1
shift
goto :getparentdir

@rem if not update,don't remove the binbak dir.

@rem @set "OLDPATH=%PATH%"
@rem @echo %PATH%
@rem @set "PATH=%PATH%;%~dp0\bin"
@rem @echo %PATH%
@rem start .\MokHacker.exe
@rem echo run complete.
@rem @set "PATH=%OLDPATH%"
@rem @echo %PATH%
::@pause