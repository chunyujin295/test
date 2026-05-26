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

@rem 设置更新相关的关键路径及关键目录.
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

@rem 设置更新包相关目录
  set update_dir=%Directory%\otawork\otawork\download\update
  set update_bin_dir=%Directory%\otawork\otawork\download\update\newpackage\bin

rem 获取当前目录的父目录
  for %%d in (%~dp0..) do set ParentDirectory=%%~fd
  echo ParentDirectory=%ParentDirectory%

echo %Directory%
echo %ParentDirectory%

@rem 设置更新包相关目录
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
@rem 如果没有更新包对应的update目录,表明需要更新安装.
	echo no update dir,don't need to update.	
	goto exec_current_bin
) else (
	echo has update dir,need to update.		
	if not exist %update_bin_dir% (
@rem 如更新包对应的update目录不存在bin目录,不需更新.
		echo no update bin dir,don't need to update.	
		goto exec_current_bin
	) else (
		echo has update bin dir,need to update.	
		if not exist %current_bin_dir% (
@rem 若当前目录不存在bin目录,结束流程.
			echo current bin not exists.
			goto exec_current_bin
		) else (
			echo current bin exists.

@rem 若当前目录存在binbak目录,删除binbak目录.
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

@rem 将当前安装目录中的bin目录更更改为binbak,以作备份.
			move %current_bin_dir% %current_binbak_dir%

			if exist %current_bin_dir% (
				echo rename current bin dir failed.
				goto exec_current_bin
			)

@rem 将更新包对应的bin目录移动到安装目录中的bin目录

			move %update_bin_dir% .\	
			if exist %update_bin_dir% (
@rem 若对应的更新包bin目录仍然存在,说明bin目录移动失败.
				echo update bin dir still exists,failed.
				goto exec_current_bin
			)

			if not exist %current_bin_dir% (
@rem 当前安装目录若无bin,提示错误,结束流程.
				echo current bin dir not exists,move update bin dir failed.
				goto exec_current_bin
			)

			if not exist %current_bin_version% (
@rem 当前bin目录若没有version.json,提示错误信息,并结束.
				echo current bin dir has not version.json,move update bin dir failed.
				goto exec_current_bin
			)



		)

	)
) 

:exec_current_bin
::pause
::exit
@rem 判断当前bin目录是否存在Mohacker.exe
if not exist %current_bin_main% (
@rem 目录不存在Mohacker.exe,失败退出.
	echo current bin dir's mohakcer not exists.
) else (
@rem 目录存在Mohacker.exe,启动该程序.
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