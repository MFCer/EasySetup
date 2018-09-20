
@echo off

cd /d %~dp0%

rem ���û�������
set toolsDir=tools
set packageDir=packages
set extractDir=%packageDir%\extracts
set fileDir=files

rem �����ʷ������װ��
del /q *.exe

rem ���������ѹ����
if exist %packageDir% (rmdir /s /q %packageDir%)
xcopy /y /i *.zip %packageDir%

rem ��ѹ�����ѹ����
if exist %extractDir% (rmdir /s /q %extractDir%)
"%toolsDir%\7z\7z" x %packageDir%\*.zip -y -o%extractDir%

rem ������ѹ����ļ�
if exist %fileDir% (rmdir /s /q %fileDir%)
xcopy /e /y /i %extractDir%\* %fileDir%

rem ������װ���ű�
"%toolsDir%\NSIS\makensis.exe" /DBUILD=%version% sample.nsi

