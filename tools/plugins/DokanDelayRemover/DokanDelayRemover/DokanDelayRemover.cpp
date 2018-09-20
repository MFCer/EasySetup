// DokanDelayRemover.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "DokanDelayRemover.h"

typedef BOOL (WINAPI *lpfnWow64DisableWow64FsRedirection) (PVOID *OldValue);
typedef BOOL (WINAPI *lpfnWow64RevertWow64FsRedirection) (PVOID OldValue);

// ���庯��ָ��
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
			// ɾ������
			ServiceDelete(_T("dokan"));
			// ɾ���ļ�
			if (!DeleteFile (drvPath)) {
				MoveFileEx (drvPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
			ServiceDelete(_T("DokanDelayRemover"));
			// ��ɾ��
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

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
//  ע��:
//
//    ����ϣ��
//    �˴�������ӵ� Windows 95 �еġ�RegisterClassEx��
//    ����֮ǰ�� Win32 ϵͳ����ʱ������Ҫ�˺��������÷������ô˺���ʮ����Ҫ��
//    ����Ӧ�ó���Ϳ��Ի�ù�����
//    ����ʽ��ȷ�ġ�Сͼ�ꡣ
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
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	return TRUE;
}

//
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_QUERYENDSESSION:
		{
			// �������ע��
			if (lParam == 0)
			{
				Wow64Helper wh (Wow64Helper::IsWow64 ());
				TCHAR drvPath[MAX_PATH];
				GetWindowsDirectory(drvPath, MAX_PATH);
				lstrcat (drvPath, _T("\\system32\\drivers\\dokan.sys"));
				// ɾ������
				ServiceDelete(_T("dokan"));
				// ɾ���ļ�
				if (!DeleteFile (drvPath)) {
					MoveFileEx (drvPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
				HKEY hKey;
				if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey))
				{
					RegDeleteValue(hKey, _T("DokanDelayRemover"));
					RegCloseKey(hKey);
				}
				// ��ɾ��
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
		// ������л������������ͷž������λ������
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		return FALSE;
	}

	// ����ϵͳ��̬���ӿ�"Kernel32.dll"
	HINSTANCE hKernelDll = LoadLibrary(_T("Kernel32.dll"));
	// ������سɹ�
	if (hKernelDll)
	{
		// �õ�������ַ
		pfnWow64DisableWow64FsRedirection = (lpfnWow64DisableWow64FsRedirection)GetProcAddress(hKernelDll, "Wow64DisableWow64FsRedirection");
		pfnWow64RevertWow64FsRedirection = (lpfnWow64RevertWow64FsRedirection)GetProcAddress(hKernelDll, "Wow64RevertWow64FsRedirection");
		// �ͷ�ϵͳ��̬���ӿ���
		FreeLibrary(hKernelDll);
	}

	OSVERSIONINFOEX osvi = {sizeof(OSVERSIONINFOEX)};
	GetVersionEx((OSVERSIONINFO*) &osvi);

	// �����vista�����ϣ�ʹ�÷������
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

		// ��ʼ��ȫ���ַ���
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_DOKANDELAYREMOVER, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		// ִ��Ӧ�ó����ʼ��:
		if (!InitInstance (hInstance, nCmdShow))
		{
			return FALSE;
		}

		// ����Ϣѭ��:
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return (int) msg.wParam;
	}
}
