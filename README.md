# EasySetup

用于商业用途的NSIS示例脚本(NSIS sample script for commercial usage.)

[![license](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat)](https://github.com/MFCer/EasySetup/blob/master/LICENSE)

# 特点 / Features

- UNICODE版本NSIS/多国语言支持
- 安装卸载vc运行时库示例
- 驱动卸载安装无需重启（tools\plugins\DokanDelayRemover，延迟卸载）
- 插件增强
  - 按安装日志卸载（un.DelFileByLog函数）
  - 更准确的版本比较（tools\NSIS\Include\VersionCompare.nsh）
  - 自定义插件（tools\plugins\toolbox）
    - 安装卸载服务
    - 按路径查找/结束进程
- 更好看的界面
  - 侧边图片展示（res/LeftBitmap.bmp）
  - 安装包logo提供矢量ai原稿（res/Install.ai，导出png后到 https://converticon.com/ 转ico即可）
  - 安装详情gif引导（可自行替换res/detail.gif）
- 绿色卸载无残留空目录增强

# Misc

- Please feel free to use yapi.
- Looking forward to your suggestions.