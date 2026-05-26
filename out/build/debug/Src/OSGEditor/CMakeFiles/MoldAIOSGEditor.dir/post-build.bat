@echo off
cd /D D:\Code\moldaiengine\out\build\debug\Src\OSGEditor || (set FAIL_LINE=2& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy_directory D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgPlugins-3.6.5 D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64/osgPlugins-3.6.5/
 || (set FAIL_LINE=3& goto :ABORT)
cd /D D:\Code\moldaiengine\out\build\debug\Src\OSGEditor || (set FAIL_LINE=4& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osg.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=5& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgText.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=6& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgDB.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=7& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/OpenThreads.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=8& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgUtil.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=9& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgGA.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=10& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgViewer.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=11& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgParticle.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=12& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgUI.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=13& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgVolume.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=14& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgWidget.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=15& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgAnimation.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=16& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgFX.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=17& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgManipulator.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=18& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgPresentation.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=19& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgShadow.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=20& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgSim.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=21& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/OSG/3.7.5/dll/$(Platform)/$(Configuration)/osgTerrain.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=22& goto :ABORT)
goto :EOF

:ABORT
set ERROR_CODE=%ERRORLEVEL%
echo Batch file failed at line %FAIL_LINE% with errorcode %ERRORLEVEL%
exit /b %ERROR_CODE%