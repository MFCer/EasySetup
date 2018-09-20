// DokanDelayRemover.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "DokanDelayRemover.h"

typedef BOOL (WINAPI *lpfnWow64DisableWow64FsRedirection) (PVOID *OldValue);
typedef BOOL (WINAPI *lpfnWow64RevertWow64FsRedirection) (PVOID OldValue);

// 定义函数指针
static lpfnWow64DisableWow64FsRedirection pfnWow64DisableWow64FsRedirection = NULL;
static lpfnWow64RevertWow64FsRedirection pfnWow64RevertWow64FsRedirection = NULL;

class Wow64Helper
{
public:
	Wow64Helper(BOOL needRedirect)
		: m_oldValue(NULL)
	{
		if (needRedirect && pfnWow64DisableWow64FsRedirection)
		{
			pfnWow64DisableWow64FsRedirection(&m_oldValue);
		}
	}
	~Wow64Helper(void)
	{
		if (m_oldValue && pfnWow64RevertWow64FsRedirection)
		{
			pfnWow64RevertWow64FsRedirection(m_oldValue);
		}
	}

	static BOOL IsWow64(void)
	{
		static BOOL s_isWow64 = _IsWow64();
		return s_isWow64;
	}

private:
	static BOOL _IsWow64(void)
	{
		BOOL isWow64Process = FALSE;
		IsWow64Process(GetCurrentProcess(), &isWow64Process);
		return isWow64Process;
	}

	PVOID m_oldValue;
};

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);

static DWORD ServiceDelete(LPCTSTR lpszServiceName)
{
	SC_HANDLE controlHandle;
	SC_HANDLE serviceHandle;
	DWORD result = NO_ERROR;

	controlHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (controlHandle == NULL)
	{
		result = GetLastError();
		return result;
	}

	serviceHandle = OpenService(controlHandle, lpszServiceName, DELETE);

	if (serviceHandle == NULL)
	{
		result = GetLastError();
		CloseServiceHandle(controlHandle);
		return result;
	}

	if (DeleteService(serviceHandle))
	{
		result = NO_ERROR;
	}
	else
	{
		result = GetLastError();
	}

	CloseServiceHandle(serviceHandle);
	CloseServiceHandle(controlHandle);

	return result;
}

static HANDLE                g_EventControl = NULL;
static SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
static SERVICE_STATUS        g_ServiceStatus;

static DWORD WINAPI HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	switch (dwControl) {
	case SERVICE_CONTROL_PRESHUTDOWN:
		{
			Wow64Helper wh (Wow64Helper::IsWow64 ());
			TCHAR drvPath[MAX_PATH];
			GetWindowsDirectory(drvPath, MAX_PATH);
			lstrcat (drvPath, _T("\\system32\\drivers\\dokan.sys"));
			// 删除服务
			ServiceDelete(_T("dokan"));
			// 删除文件
			if (!DeleteFile (drvPath)) {
				MoveFileEx (drvPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
			ServiceDelete(_T("DokanDelayRemover"));
			// 自删除
			TCHAR exePath[MAX_PATH];
			GetModuleFileName(NULL, exePath, MAX_PATH);
			MoveFileEx (exePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
		break;

	case SERVICE_CONTROL_STOP:
		g_ServiceStatus.dwWaitHint     = 50000;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
		SetEvent(g_EventControl);
		break;

	case SERVICE_CONTROL_INTERROGATE:
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
		break;

	default:
		break;
	}

	return NO_ERROR;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	g_StatusHandle = RegisterServiceCtrlHandlerEx(_T("DokanDelayRemover"), HandlerEx, NULL);

	g_ServiceStatus.dwServiceType				= SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
	g_ServiceStatus.dwWin32ExitCode				= NO_ERROR;
	g_ServiceStatus.dwControlsAccepted			= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PRESHUTDOWN;
	g_ServiceStatus.dwServiceSpecificExitCode	= 0;
	g_ServiceStatus.dwWaitHint					= 0;
	g_ServiceStatus.dwCheckPoint				= 1;
	g_ServiceStatus.dwCurrentState				= SERVICE_RUNNING;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	g_EventControl = CreateEvent(NULL, TRUE, FALSE, NULL);
	WaitForSingleObject(g_EventControl, INFINITE);

	CloseHandle(g_EventControl);

	g_ServiceStatus.dwWaitHint     = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}


#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // 将实例句柄存储在全局变量中

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_QUERYENDSESSION:
		{
			// 如果不是注销
			if (lParam == 0)
			{
				Wow64Helper wh (Wow64Helper::IsWow64 ());
				TCHAR drvPath[MAX_PATH];
				GetWindowsDirectory(drvPath, MAX_PATH);
				lstrcat (drvPath, _T("\\system32\\drivers\\dokan.sys"));
				// 删除服务
				ServiceDelete(_T("dokan"));
				// 删除文件
				if (!DeleteFile (drvPath)) {
					MoveFileEx (drvPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
				HKEY hKey;
				if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey))
				{
					RegDeleteValue(hKey, _T("DokanDelayRemover"));
					RegCloseKey(hKey);
				}
				// 自删除
				TCHAR exePath[MAX_PATH];
				GetModuleFileName(NULL, exePath, MAX_PATH);
				MoveFileEx (exePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
		}
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	HANDLE m_hMutex = CreateMutex(NULL, FALSE, _T("DokanDelayRemover"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		// 如果已有互斥量存在则释放句柄并复位互斥量
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		return FALSE;
	}

	// 加载系统动态链接库"Kernel32.dll"
	HINSTANCE hKernelDll = LoadLibrary(_T("Kernel32.dll"));
	// 如果加载成功
	if (hKernelDll)
	{
		// 得到函数地址
		pfnWow64DisableWow64FsRedirection = (lpfnWow64DisableWow64FsRedirection)GetProcAddress(hKernelDll, "Wow64DisableWow64FsRedirection");
		pfnWow64RevertWow64FsRedirection = (lpfnWow64RevertWow64FsRedirection)GetProcAddress(hKernelDll, "Wow64RevertWow64FsRedirection");
		// 释放系统动态链接库句柄
		FreeLibrary(hKernelDll);
	}

	OSVERSIONINFOEX osvi = {sizeof(OSVERSIONINFOEX)};
	GetVersionEx((OSVERSIONINFO*) &osvi);

	// 如果是vista及以上，使用服务程序
	if (osvi.dwMajorVersion >= 6)
	{
		SERVICE_TABLE_ENTRY DispatchTable[] = {
			{_T("DokanDelayRemover"), ServiceMain},
			{NULL, NULL}
		};

		if (!StartServiceCtrlDispatcher(DispatchTable))
		{
			MessageBox(NULL, _T("Do not start this program manually!"), _T("Prompt"), MB_OK | MB_ICONWARNING);
		}
		return 0;
	}
	else {
		TCHAR exePath[MAX_PATH];
		GetModuleFileName(NULL, exePath, MAX_PATH);
		HKEY hKey;
		if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, NULL, &hKey, NULL))
		{
			RegSetValueEx(hKey, _T("DokanDelayRemover"), NULL, REG_SZ, (LPBYTE)exePath, (DWORD)(lstrlen(exePath) * sizeof(TCHAR)));
			RegCloseKey(hKey);
		}

		MSG msg;

		// 初始化全局字符串
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_DOKANDELAYREMOVER, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		// 执行应用程序初始化:
		if (!InitInstance (hInstance, nCmdShow))
		{
			return FALSE;
		}

		// 主消息循环:
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return (int) msg.wParam;
	}
}
