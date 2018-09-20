
@echo off

cd /d %~dp0%

rem 设置环境变量
set toolsDir=tools
set packageDir=packages
set extractDir=%packageDir%\extracts
set fileDir=files

rem 清除历史遗留安装包
del /q *.exe

rem 拷贝程序库压缩包
if exist %packageDir% (rmdir /s /q %packageDir%)
xcopy /y /i *.zip %packageDir%

rem 解压程序库压缩包
if exist %extractDir% (rmdir /s /q %extractDir%)
"%toolsDir%\7z\7z" x %packageDir%\*.zip -y -o%extractDir%

rem 拷贝解压后的文件
if exist %fileDir% (rmdir /s /q %fileDir%)
xcopy /e /y /i %extractDir%\* %fileDir%

rem 启动安装包脚本
"%toolsDir%\NSIS\makensis.exe" /DBUILD=%version% sample.nsi

