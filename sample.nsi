/*
  EasySetup -- NSIS sample script for commercial usage.

  Copyright (c) 2010-2018 <http://ez8.co> <orca.zhang@yahoo.com>
  This library is released under the MIT License.
  Please see LICENSE file or visit https://github.com/MFCer/EasySetup for details.
*/
Unicode true
SetCompressor /SOLID lzma
; --------------------传入参数部分---------------------

; 输出文件名称
OutFile "sample_setup.exe"
; 输入文件目录
!define INPUT_PATH "files"

; ---------------------OEM部分开始-------------------
; 产品名称
!define PRODUCT_NAME "SAMPLE"
; 产品路径（用于开始菜单和安装目录）
!define PRODUCT_PATH "SAMPLE"
; 公司名称
!define PRODUCT_PUBLISHER "ez8.co"
; 产品版本
!define PRODUCT_VERSION "0.1"

; ---------------------预定义部分开始-------------------

; 安装程序初始定义常量
; 主程序名称
!define MAIN_FILE_NAME "sample"

!define PRODUCT_APP_PATH_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_FILE_NAME}.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY HKLM

!define SETUP_MUTEX_NAME "{71761230-bcde-11e8-bef6-080027d825b8}"

!define DRIVER_FILE_NAME "dokan.sys"
!define DRIVER_DLL_FILE_NAME "dokan.dll"
!define SERVICE_FILE_NAME "sample_svc.exe"

!define DRV_SERVICE_NAME "dokan"
!define SVC_SERVICE_NAME "SampleSvc"

/*
!define EXPLORER_PATH "$WINDIR\explorer.exe"
*/

!include "x64.nsh"
!include "WinVer.nsh"
; 包含版本比较头文件，非NSIS提供，NSIS提供的有BUG，此为热心网友修改版
!include "VersionCompare.nsh"
!include "LogicLib.nsh"

!addplugindir "tools\plugins"

; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "res\Install.ico"
!define MUI_UNICON "res\Uninstall.ico"
; 用于欢迎页面和完成页面的位图. (推荐尺寸: 164x314 象素).
!define MUI_WELCOMEFINISHPAGE_BITMAP "res\LeftBitmap.bmp"

; 以下一些特殊回调函数在使用MUI时必须用这种方式重新定义
!define MUI_CUSTOMFUNCTION_UNGUIINIT un.myUnGuiInit

; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 许可协议页面
!define MUI_LICENSEPAGE_BUTTON
!insertmacro MUI_PAGE_LICENSE "res\sample.rtf"
; 安装目录选择页面
!insertmacro MUI_PAGE_DIRECTORY
; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES
; 安装完成页面
!define MUI_FINISHPAGE_RUN "$INSTDIR\${MAIN_FILE_NAME}.exe"
!insertmacro MUI_PAGE_FINISH

; 安装卸载过程页面
;!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

; 安装预释放文件
!insertmacro MUI_RESERVEFILE_LANGDLL
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; ------ MUI 现代界面定义结束 ------

Name "${PRODUCT_NAME}"
InstallDir "$PROGRAMFILES\${PRODUCT_PATH}"
ShowInstDetails nevershow
ShowUnInstDetails show
BrandingText ${PRODUCT_PUBLISHER}

; 系统互斥检查
!macro MutexCheck _mutexname _outvar _handle
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${_mutexname}" ) i.r1 ?e'
  StrCpy ${_handle} $1
  Pop ${_outvar}
!macroend

; 获取父路径
!macro GetParent _outvar_ _path_
  Push $R1
  Push $R2
  Push $R3

  StrCpy $R1 0
  StrLen $R2 "${_path_}"

  ${Do}
    IntOp $R1 $R1 + 1
    IntCmp $R1 $R2 ${_break} 0 ${_break}
    StrCpy $R3 "${_path_}" 1 -$R1
    StrCmp $R3 "\" ${_break}
  ${Loop}

  StrCpy ${_outvar_} "${_path_}" -$R1

  Pop $R3
  Pop $R2
  Pop $R1
!macroend

; 删除指定目录，并且检查父目录是否为空
!macro RemoveDir _path_ _recursive_
  Push $R0
  IntCmp ${_recursive_} 1 0 +3
    ; 删除目录
    RMDir /r "${_path_}"
    Goto +2
  RMDir "${_path_}"

  StrCpy $R0 "${_path_}"
  ${Do}
    ; 如果父目录为空，删除
    !insertmacro GetParent $R0 $R0
	
    ClearErrors
    RMDir $R0
    IfErrors ${_break}
  ${Loop}
  Pop $R0
!macroend

; 根据安装日志卸载文件的调用宏
!macro DelFileByLog LogFile
  IfFileExists `${LogFile}` 0 +4
    Push `${LogFile}`
    Call un.DelFileByLog
    Delete `${LogFile}`
!macroend

; 拷贝驱动文件
!macro DokanDriver os arc
  SetOutPath $SYSDIR\drivers
  File dokan\drv\${os}\${arc}\${DRIVER_FILE_NAME}
!macroend

; 拷贝Dokan依赖文件
!macro DokanDeps os
  SetOutPath "$INSTDIR"
  File dokan\dll\${os}\${DRIVER_DLL_FILE_NAME}
  SetOutPath $SYSDIR
  File dokan\svc\${os}\${SERVICE_FILE_NAME}
!macroend

; ---------------------预定义部分结束-------------------

; 版本号，需要将主版本号和构建号合并
Var ProductVersion

; ---------------------多语言字符串定义部分开始-------------------

LangString MSG_INSTALLING ${LANG_ENGLISH} "$(^Name) is installing."
LangString MSG_INSTALLING ${LANG_SIMPCHINESE} "$(^Name) 已在安装。"

LangString MSG_UNINSTALLING ${LANG_ENGLISH} "$(^Name) is uninstalling."
LangString MSG_UNINSTALLING ${LANG_SIMPCHINESE} "$(^Name) 已在卸载。"

LangString MSG_OS_VER_NOT_SUPPORT ${LANG_ENGLISH} "Your OS is not supported. Currently supports Windows XP, 2003, Vista, 2008, 7, 2008R2."
LangString MSG_OS_VER_NOT_SUPPORT ${LANG_SIMPCHINESE} "暂不支持您的操作系统，目前仅支持Windows XP, 2003, Vista, 2008, 7, 2008R2。"

LangString MSG_OLD_VERSION ${LANG_ENGLISH} "Latest version of $(^Name) detected. $\n\
       Do you want to continue with the installation?"
LangString MSG_OLD_VERSION ${LANG_SIMPCHINESE} "安装程序检测到您的机器上已有更新版本的 $(^Name)。$\n\
       您是否要继续进行安装？"

LangString MSG_OVER_WRITE ${LANG_ENGLISH} "$(^Name) detected. $\n\
       Do you want to continue with the installation?"
LangString MSG_OVER_WRITE ${LANG_SIMPCHINESE} "安装程序检测到您的机器上已安装 $(^Name)。$\n\
       您是否要继续进行安装？"

LangString MSG_UNINSTALLING_PREVIOUS_VERSION ${LANG_ENGLISH} "Uninstalling previous version."
LangString MSG_UNINSTALLING_PREVIOUS_VERSION ${LANG_SIMPCHINESE} "正在卸载之前安装的版本..."

LangString MSG_DETECT_PROGRAM_RUNNING ${LANG_ENGLISH} "$R0 is running."
LangString MSG_DETECT_PROGRAM_RUNNING ${LANG_SIMPCHINESE} "检测到 $R0 正在运行。"

LangString MSG_FAILED_TO_INSTALL_DRIVER ${LANG_ENGLISH} "You should reboot your computer to enable the driver before using the program. "
LangString MSG_FAILED_TO_INSTALL_DRIVER ${LANG_SIMPCHINESE} "驱动需要重启才能生效，未重启前暂时无法使用程序。"

LangString MSG_FAILED_TO_INSTALL_SERVICE ${LANG_ENGLISH} "Failed to install service. $\nError:"
LangString MSG_FAILED_TO_INSTALL_SERVICE ${LANG_SIMPCHINESE} "安装服务失败。$\n错误号："

LangString MSG_PROMPT_EXE_RUNNING ${LANG_ENGLISH} "$(^Name) is running. $\n\
		Press OK after ${PRODUCT_NAME} exited, or CANCEL to exit installer."
LangString MSG_PROMPT_EXE_RUNNING ${LANG_SIMPCHINESE} "检测到 $(^Name) 正在运行。$\n\
		关闭程序后点击确定重试，或点击取消退出安装程序。"

LangString MSG_PROMPT_RESTART_EXPLORER ${LANG_ENGLISH} "Press OK to continue restarting explorer.exe to enable components."
LangString MSG_PROMPT_RESTART_EXPLORER ${LANG_SIMPCHINESE} "请点击“确定”按钮重启explorer进程，使相关组件生效。（注：重启过程中，文件夹和拷贝任务等都将自动关闭）"

LangString UNMSG_PROMPT_RESTART_EXPLORER ${LANG_ENGLISH} "Press OK to continue restarting explorer.exe to uninstall components."
LangString UNMSG_PROMPT_RESTART_EXPLORER ${LANG_SIMPCHINESE} "请点击“确定”按钮重启explorer进程，以卸载相关组件。（注：重启过程中，文件夹和拷贝任务等都将自动关闭）"

LangString UNMSG_PROMPT_ENSURE ${LANG_ENGLISH} "Are you sure to remove $(^Name) and all its components?"
LangString UNMSG_PROMPT_ENSURE ${LANG_SIMPCHINESE} "你确实要完全移除 $(^Name) ，及其所有的组件？"

LangString UNMSG_PROMPT_EXE_RUNNING ${LANG_ENGLISH} "$(^Name) is running. $\n\
		Press OK after ${PRODUCT_NAME} exited, or CANCEL to exit."
LangString UNMSG_PROMPT_EXE_RUNNING ${LANG_SIMPCHINESE} "检测到 $(^Name) 正在运行。$\n\
		关闭程序后点击确定重试，或点击取消退出卸载程序。"

; ---------------------多语言字符串定义部分结束-------------------


; ---------------------区段（可选组件）定义部分开始-------------------

; Gif图启动区段
Section GifStart 0
  SetOutPath '$PLUGINSDIR'
  File "res\detail.gif"
  AnimGif::play /NOUNLOAD /VALIGN=90 '$PLUGINSDIR\detail.gif'
SectionEnd

; 安装Redis库区段
Section Redis 1
  SetOutPath "$TEMP"
  SetOverwrite ifnewer
	
  ;是否已经安装 VS2008 SP1 Redistributable 运行库
  EnumRegValue $0 ${PRODUCT_UNINST_ROOT_KEY} "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{86CE1746-9EFF-3C9C-8755-81EA8903AC34}" 0
  StrCmp $0 "" 0 +6
    DetailPrint "正在安装 VS2008 SP1 Redistributable(x86) 运行库..."
    File /r "tools\vcredist\vcredist_x86.exe"
    ExecWait '"vcredist_x86.exe" /q'
    Delete "vcredist_x86.exe"
	Goto +2
  DetailPrint "已安装 VS2008 SP1 Redistributable(x86) 运行库，跳过..."

  /*
  ${If} ${RunningX64}
    SetRegView 64
    EnumRegValue $0 ${PRODUCT_UNINST_ROOT_KEY} "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{56F27690-F6EA-3356-980A-02BA379506EE}" 0
    StrCmp $0 "" 0 +6
      DetailPrint "正在安装 VS2008 SP1 Redistributable(x64) 运行库..."
      File /r "tools\vcredist\vcredist_x64.exe"
      ExecWait '"vcredist_x64.exe" /q'
      Delete "vcredist_x64.exe"
  	Goto +2
    DetailPrint "已安装 VS2008 SP1 Redistributable(x64) 运行库，跳过..."
    SetRegView lastused
  ${EndIf}
  */
SectionEnd

Section UninstallOlder 2
  ReadRegStr $1 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  StrCmp $1 "" Done

  /*
  ; 卸载程序启动前就关闭explorer
  MessageBox MB_OK|MB_ICONINFORMATION "$(UNMSG_PROMPT_RESTART_EXPLORER)" /SD IDOK
  toolbox::KillProcByPath ${EXPLORER_PATH}
  */

  DetailPrint "$(MSG_UNINSTALLING_PREVIOUS_VERSION)"
  !insertmacro GetParent $R0 $1
  nsExec::Exec '"$1" /S _?=$R0' $0

  ; 删除安装目录
  !insertmacro RemoveDir $R0 0
Done:
SectionEnd

Section Driver 3
  SetOverwrite off
  
  ${DisableX64FSRedirection}
  ${If} ${RunningX64}
      ${If} ${IsWin2012R2}
        !insertmacro DokanDriver "win7" "amd64"
      ${ElseIf} ${IsWin8.1}
        !insertmacro DokanDriver "win7" "amd64"
      ${ElseIf} ${IsWin2012}
        !insertmacro DokanDriver "win7" "amd64"
      ${ElseIf} ${IsWin8}
        !insertmacro DokanDriver "win7" "amd64"
      ${ElseIf} ${IsWin7}
        !insertmacro DokanDriver "win7" "amd64"
      ${ElseIf} ${IsWin2008R2}
        !insertmacro DokanDriver "win7" "amd64"
      ${ElseIf} ${IsWinVista}
        !insertmacro DokanDriver "wlh" "amd64"
      ${ElseIf} ${IsWin2008}
        !insertmacro DokanDriver "wlh" "amd64"
      ${ElseIf} ${IsWin2003}
        !insertmacro DokanDriver "wnet" "amd64"
      ${EndIf}
  ${Else}
      ${If} ${IsWin2012R2}
        !insertmacro DokanDriver "win7" "i386"
      ${ElseIf} ${IsWin8.1}
        !insertmacro DokanDriver "win7" "i386"
      ${ElseIf} ${IsWin2012}
        !insertmacro DokanDriver "win7" "i386"
      ${ElseIf} ${IsWin8}
        !insertmacro DokanDriver "win7" "i386"
      ${ElseIf} ${IsWin7}
        !insertmacro DokanDriver "win7" "i386"
      ${ElseIf} ${IsWin2008R2}
        !insertmacro DokanDriver "win7" "i386"
      ${ElseIf} ${IsWinVista}
        !insertmacro DokanDriver "wlh" "i386"
      ${ElseIf} ${IsWin2008}
        !insertmacro DokanDriver "wlh" "i386"
      ${ElseIf} ${IsWin2003}
        !insertmacro DokanDriver "wnet" "i386"
      ${ElseIf} ${IsWinXP}
        !insertmacro DokanDriver "wxp" "i386"
      ${EndIf}
  ${EndIf}
  
  ${If} ${IsWin2012R2}
    !insertmacro DokanDeps "win7"
  ${ElseIf} ${IsWin8.1}
    !insertmacro DokanDeps "win7"
  ${ElseIf} ${IsWin2012}
    !insertmacro DokanDeps "win7"
  ${ElseIf} ${IsWin8}
    !insertmacro DokanDeps "win7"
  ${ElseIf} ${IsWin7}
    !insertmacro DokanDeps "win7"
  ${ElseIf} ${IsWin2008R2}
    !insertmacro DokanDeps "win7"
  ${ElseIf} ${IsWinVista}
    !insertmacro DokanDeps "wlh"
  ${ElseIf} ${IsWin2008}
    !insertmacro DokanDeps "wlh"
  ${ElseIf} ${IsWin2003}
    !insertmacro DokanDeps "wnet"
  ${ElseIf} ${IsWinXP}
    !insertmacro DokanDeps "wxp"
  ${EndIf}
  
  SetOverwrite lastused
  
  ; 取消删除dokan
  ${If} ${AtLeastWinVista}
    toolbox::StopService "DokanDelayRemover"
    toolbox::RemoveService "DokanDelayRemover"
  ${Else}
    toolbox::KillProcByPath "$TEMP\DokanDelayRemover.exe"
    DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "DokanDelayRemover"
    Delete "$TEMP\DokanDelayRemover.exe"
  ${EndIf}
  
  ; 安装dokan驱动
  DetailPrint "正在安装dokan驱动..."
  toolbox::InstallService "${DRV_SERVICE_NAME}" "dokan driver" 2 "\SystemRoot\system32\drivers\${DRIVER_FILE_NAME}" ${RunningX64}
  StrCmp $R1 "0" SERVICE
    MessageBox MB_ICONINFORMATION|MB_OK|MB_DEFBUTTON1 "$(MSG_FAILED_TO_INSTALL_DRIVER)"
    SetRebootFlag true

SERVICE:  
  ; 安装SampleSvc服务
  DetailPrint "正在安装SampleSvc服务..."
  toolbox::InstallService "${SVC_SERVICE_NAME}" "${SVC_SERVICE_NAME}" 16 "$SYSDIR\${SERVICE_FILE_NAME}" 0
  StrCmp $R1 "0" Done

  MessageBox MB_ICONINFORMATION|MB_OK|MB_DEFBUTTON1 "$(MSG_FAILED_TO_INSTALL_SERVICE)$R1"

Done:
SectionEnd

; 主区段（安装文件、创建快捷方式和写入注册表信息等）
Section Main 4
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  
  ; 打开日志
  LogSet on
  
  ; 编译时，将对应目录的文件打包到安装程序;
  ; 安装时，相反，将文件释放到安装目录
  File /r "${INPUT_PATH}\*.*"
  
  ; 关闭日志
  LogSet off
  
  ; 创建卸载程序
  WriteUninstaller "$INSTDIR\uninst.exe"
  
  ; 记录注册表信息
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_APP_PATH_REGKEY}" "" "$INSTDIR\${MAIN_FILE_NAME}.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${MAIN_FILE_NAME}.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "$ProductVersion"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"

  ; 创建快捷方式
  SetShellVarContext current
  
  CreateShortCut "$INSTDIR\${PRODUCT_NAME}.lnk" "$INSTDIR\${MAIN_FILE_NAME}.exe"
  CopyFiles "$INSTDIR\${PRODUCT_NAME}.lnk" "$DESKTOP\${PRODUCT_NAME}.lnk"
  Delete "$INSTDIR\${PRODUCT_NAME}.lnk"
  
  ; 创建开始菜单
  CreateDirectory "$SMPROGRAMS\${PRODUCT_PATH}\"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_PATH}\${PRODUCT_NAME}.lnk" "$INSTDIR\${MAIN_FILE_NAME}.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_PATH}\卸载 ${PRODUCT_NAME}.lnk" "$INSTDIR\uninst.exe"
  
  ; 注册XPCOM组件
  nsExec::Exec `$INSTDIR\regxpcom`
  
  /*
  ; 注册组件
  nsExec::Exec `regsvr32.exe /s "$INSTDIR\shellext.dll"`
  
  ; 如果explorer已经停止，直接启动，否则为重启
  toolbox::FindProcByPath ${EXPLORER_PATH}
  IntCmp $R1 1 0 +3
  MessageBox MB_OK|MB_ICONINFORMATION "$(MSG_PROMPT_RESTART_EXPLORER)" /SD IDOK
  toolbox::KillProcByPath ${EXPLORER_PATH}
  Exec ${EXPLORER_PATH}
  */
  
SectionEnd

Section GifStop 5
  AnimGif::stop
  Delete '$PLUGINSDIR\detail.gif'
SectionEnd

/******************************
 *  以下是安装程序的卸载部分  *
 ******************************/

; 卸载区段
Section Uninstall
  ; 查看主程序是否在运行
  StrCpy $R0 "$INSTDIR\${MAIN_FILE_NAME}.exe"
  toolbox::FindProcByPath $R0
  IntCmp $R1 1 0 +5
    ; 如果在运行，让用户选择
    DetailPrint $(MSG_DETECT_PROGRAM_RUNNING)
    MessageBox MB_ICONINFORMATION|MB_OKCANCEL|MB_DEFBUTTON1 $(UNMSG_PROMPT_EXE_RUNNING) IDOK +2
    Quit
; Retry:
  Goto -5
  
  ; 卸载SampleSvc服务
  DetailPrint "正在卸载SampleSvc服务..."
  toolbox::RemoveService "${SVC_SERVICE_NAME}"
  
  SetOverwrite off
  
  ${DisableX64FSRedirection}
  IfSilent +9
    ; 卸载dokan驱动
    SetOutPath "$TEMP"
    DetailPrint "延迟卸载dokan驱动..."
    File "tools\plugins\DokanDelayRemover.exe"
    ${If} ${AtLeastWinVista}
      toolbox::InstallService "DokanDelayRemover" "DokanDelayRemover" 272 "$TEMP\DokanDelayRemover.exe" 0
    ${Else}
      ExecShell "open" "$TEMP\DokanDelayRemover.exe" "" SW_HIDE
    ${EndIf}
  ${EnableX64FSRedirection}
  
  SetOverwrite lastused

  /*
  ; 卸载组件
  nsExec::Exec `regsvr32.exe /u /s "$INSTDIR\shellext.dll"`

  ; 如果explorer已经停止，直接跳过重启部分
  toolbox::FindProcByPath ${EXPLORER_PATH}
  IntCmp $R1 1 0 +4
  MessageBox MB_OK|MB_ICONINFORMATION "$(UNMSG_PROMPT_RESTART_EXPLORER)" /SD IDOK
  toolbox::KillProcByPath ${EXPLORER_PATH}
  Exec ${EXPLORER_PATH}
  */

  ; 移除安装目录所有文件
  ; 调用宏只根据安装日志卸载安装程序自己安装过的文件
  !insertmacro DelFileByLog "$INSTDIR\install.log"
  
  ; 删除临时生成的文件
  ; Delete "$INSTDIR\log.txt"
  Delete "$INSTDIR\${DRIVER_DLL_FILE_NAME}"
  Delete "$INSTDIR\uninst.exe"

  ; 删除TmpPath
  !insertmacro RemoveDir "$INSTDIR\TmpPath" 1
  
  ; 删除空文件夹
  Push $INSTDIR
  Call un.RemoveEmptyDir

  ; 删除上层空文件夹
  !insertmacro RemoveDir $INSTDIR 0

  SetShellVarContext current
  ; 删除桌面快捷方式
  Rename "$DESKTOP\${PRODUCT_NAME}.lnk" "$DESKTOP\${PRODUCT_NAME}.tmp"
  Delete "$DESKTOP\${PRODUCT_NAME}.tmp"
  ; 删除开始菜单项
  !insertmacro RemoveDir "$SMPROGRAMS\${PRODUCT_PATH}" 1
  
  SetShellVarContext all
  ; 删除所有用户的开始菜单项
  !insertmacro RemoveDir "$SMPROGRAMS\${PRODUCT_PATH}" 1
  
  ; 删除注册表项
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_APP_PATH_REGKEY}"

SectionEnd

; ---------------------区段（可选组件）定义部分结束-------------------


#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

; 安装程序初始化完成调用的回调函数
Function .onInit
  !insertmacro MutexCheck "${SETUP_MUTEX_NAME}" $0 $9
  StrCmp $0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION $(MSG_INSTALLING) /SD IDOK
    Quit
    
  ${If} ${IsWin7}
  ${ElseIf} ${IsWin2008R2}
  ${ElseIf} ${IsWinVista}
  ${ElseIf} ${IsWin2008}
  ${ElseIf} ${IsWin2003}
  ${ElseIf} ${IsWinXP}
  ${Else}
      MessageBox MB_OK $(MSG_OS_VER_NOT_SUPPORT)
      Quit
  ${EndIf}
  
  StrCpy $ProductVersion "${PRODUCT_VERSION}${BUILD}"

  ; 如果不是再次安装，跳过，否则弹出提示
  ReadRegStr $R0 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_APP_PATH_REGKEY}" ""
  StrCmp $R0 "" CheckRunning
    ; 放入新安装版本
    StrCpy $1 $ProductVersion
    ; 读取已安装版本
    ReadRegStr $2 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion"
    Push $2
    Push $1
    ; 比较
    Call VersionCheckF
    Pop $1
    ; 根据结果弹出提示，比已安装版本新则跳过
    Intcmp $1 1 Done +3
      ; 已安装版本较新
      MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 $(MSG_OLD_VERSION) /SD IDNO IDYES +4
      Quit
    ; 与已安装版本相同
    MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 $(MSG_OVER_WRITE) /SD IDNO IDYES +2
    Quit

CheckRunning:
  ; 查看主程序是否正在运行
  toolbox::FindProcByPath $R0
  IntCmp $R1 1 0 +5
    ; 如果正在运行，由用户选择选项
    DetailPrint $(MSG_DETECT_PROGRAM_RUNNING)
    MessageBox MB_ICONINFORMATION|MB_OKCANCEL|MB_DEFBUTTON1 $(MSG_PROMPT_EXE_RUNNING) /SD IDCANCEL IDOK +2
    Quit
; Retry:
  Goto -5

Done:
  ReadRegStr $1 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation"
  StrCmp $1 "" +2
    StrCpy $INSTDIR $1
  
  ; Redis段不计算大小
  SectionSetSize 1 0
FunctionEnd

; 卸载程序用户界面初始化后调用的回调函数
Function un.myUnGuiInit
  ; 创建全局互斥，查看是否已在卸载
  !insertmacro MutexCheck "${SETUP_MUTEX_NAME}" $0 $9
  StrCmp $0 0 +3
    ; 如果已在卸载，弹出提示
    MessageBox MB_OK|MB_ICONEXCLAMATION $(MSG_UNINSTALLING) /SD IDOK
    Quit
  ; 让用户确认是否继续卸载
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 $(UNMSG_PROMPT_ENSURE) /SD IDYES IDYES +2
  Abort
FunctionEnd

; 以下是卸载程序通过安装日志卸载文件的专用函数，请不要随意修改
Function un.DelFileByLog
  Exch $R0
  Push $R1
  Push $R2
  Push $R3
  FileOpen $R0 $R0 r
  ${Do}
    FileReadUTF16LE $R0 $R1
    ${IfThen} $R1 == `` ${|} ${ExitDo} ${|}
    StrCpy $R1 $R1 -2
    StrCpy $R2 $R1 11
    StrCpy $R3 $R1 20
    ${If} $R2 == "File: wrote"
    ${OrIf} $R2 == "File: skipp"
    ${OrIf} $R3 == "CreateShortCut: out:"
    ${OrIf} $R3 == "created uninstaller:"
      Push $R1
      Push `"`
      Call un.DelFileByLog.StrLoc
      Pop $R2
      ${If} $R2 != ""
        IntOp $R2 $R2 + 1
        StrCpy $R3 $R1 "" $R2
        Push $R3
        Push `"`
        Call un.DelFileByLog.StrLoc
        Pop $R2
        ${If} $R2 != ""
          StrCpy $R3 $R3 $R2
          Delete /REBOOTOK $R3
        ${EndIf}
      ${EndIf}
    ${EndIf}
    StrCpy $R2 $R1 7
    ${If} $R2 == "Rename:"
      Push $R1
      Push "->"
      Call un.DelFileByLog.StrLoc
      Pop $R2
      ${If} $R2 != ""
        IntOp $R2 $R2 + 2
        StrCpy $R3 $R1 "" $R2
        Delete /REBOOTOK $R3
      ${EndIf}
    ${EndIf}
  ${Loop}
  FileClose $R0
  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd

Function un.DelFileByLog.StrLoc
  Exch $R0
  Exch
  Exch $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5
  StrLen $R2 $R0
  StrLen $R3 $R1
  StrCpy $R4 0
  ${Do}
    StrCpy $R5 $R1 $R2 $R4
    ${If} $R5 == $R0
    ${OrIf} $R4 = $R3
      ${ExitDo}
    ${EndIf}
    IntOp $R4 $R4 + 1
  ${Loop}
  ${If} $R4 = $R3
    StrCpy $R0 ""
  ${Else}
    StrCpy $R0 $R4
  ${EndIf}
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd

; 删除指定目录下的空目录
; 需要先Push参数
Function un.RemoveEmptyDir
  Exch $R0
  Push $0
  Push $1
  FindFirst $0 $1 "$R0\*"
  ${Do}
    StrCmp $1 "" ${_break}
    ${If} $1 != "."
    ${AndIf} $1 != ".."
      Push "$R0\$1"
      Call un.RemoveEmptyDir
	${EndIf}
    FindNext $0 $1
  ${Loop}
  FindClose $0
  RMDir $R0
  Pop $1
  Pop $0
  Pop $R0
FunctionEnd