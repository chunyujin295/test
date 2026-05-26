@echo off
cd /D D:\Code\moldaiengine\out\build\debug\Src\Core || (set FAIL_LINE=2& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:/Code/moldaiengine/data/cameras/CD.txt D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=3& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy_directory D:/Code/moldaiengine/data/gdal/data D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64/data/
 || (set FAIL_LINE=4& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/Freeimage/3.18.0/dll/$(Platform)/$(Configuration)/FreeImage.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=5& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/glog/0.4.0/dll/$(Platform)/$(Configuration)/glog.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=6& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/libiconv/1.17/dll/$(Platform)/$(Configuration)/iconv-2.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=7& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/libfreexl/1.0.6/dll/$(Platform)/$(Configuration)/freexl.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=8& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/xlnt/1.5.0/dll/$(Platform)/$(Configuration)/xlnt.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=9& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/opencv/4100/dll/$(Platform)/$(Configuration)/opencv_world4100.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=10& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/geos/3.7.1/dll/$(Platform)/$(Configuration)/geos_c.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=11& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/geos/3.7.1/dll/$(Platform)/$(Configuration)/geos.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=12& goto :ABORT)
cd /D D:\Code\moldaiengine\out\build\debug\Src\Core || (set FAIL_LINE=13& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/expat/2.4.1/dll/$(Platform)/$(Configuration)/libexpat.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=14& goto :ABORT)
cd /D D:\Code\moldaiengine\out\build\debug\Src\Core || (set FAIL_LINE=15& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/openssl/1.1.1k/dll/$(Platform)/$(Configuration)/capi.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=16& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/openssl/1.1.1k/dll/$(Platform)/$(Configuration)/libcrypto-1_1-x64.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=17& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/openssl/1.1.1k/dll/$(Platform)/$(Configuration)/libssl-1_1-x64.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=18& goto :ABORT)
cd /D D:\Code\moldaiengine\out\build\debug\Src\Core || (set FAIL_LINE=19& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/Freeimage/3.18.0/dll/$(Platform)/$(Configuration)/FreeImage.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=20& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/exiv2/0.28.0/dll/$(Platform)/$(Configuration)/exiv2.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=21& goto :ABORT)
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E copy D:\vc142/zlib/1.2.11/dll/$(Platform)/$(Configuration)/zlib.dll D:/Code/moldaiengine/out/build/debug/MoldAI/Bin/x64
 || (set FAIL_LINE=22& goto :ABORT)
goto :EOF

:ABORT
set ERROR_CODE=%ERRORLEVEL%
echo Batch file failed at line %FAIL_LINE% with errorcode %ERRORLEVEL%
exit /b %ERROR_CODE%