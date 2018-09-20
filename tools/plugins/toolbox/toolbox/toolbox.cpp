// AnyShareDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "nsis/pluginapi.h" // nsis plugin
#pragma comment(lib, "nsis\\pluginapi.lib")

#include "toolbox.h"
#include <tlhelp32.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <stdlib.h>
#include <wchar.h>

#pragma warning(disable : 4996)

HINSTANCE g_hInstance;
HWND g_hwndParent;

#ifdef _DEBUG
#ifndef _vstprintf_s
	#ifdef _UNICODE
		#define _vstprintf_s vswprintf_s
	#else
		#define _vstprintf_s vsprintf_s
	#endif
#endif
#ifndef _vstprintf
	#ifdef _UNICODE
		#define _vstprintf vswprintf
	#else
		#define _vstprintf vsprintf
	#endif
#endif
void TRACE(LPCTSTR lpszFmt, ...)
{
	TCHAR msg[1024];
	memset(msg, 0, 1024);
	va_list args;
	va_start(args, lpszFmt);
#if _MSC_VER >= 1400 // vs2005+
	_vstprintf_s(msg, 1024, lpszFmt, args);
#else
	_vstprintf(msg, lpszFmt, args);
#endif
	va_end(args);
	OutputDebugString(msg);
}

#endif

typedef BOOL (WINAPI *lpfnWow64DisableWow64FsRedirection) (PVOID *OldValue);
typedef BOOL (WINAPI *lpfnWow64RevertWow64FsRedirection) (PVOID OldValue);

// 定义函数指针
static lpfnWow64DisableWow64FsRedirection pfnWow64DisableWow64FsRedirection = NULL;
static lpfnWow64RevertWow64FsRedirection pfnWow64RevertWow64FsRedirection = NULL;

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = (HINSTANCE)hInst;
	// 加载系统动态链接库"Kernel32.dll"
	HINSTANCE hInstance = LoadLibrary(_T("Kernel32.dll"));
	// 如果加载成功
	if (hInstance)
	{
		// 得到函数地址
		pfnWow64DisableWow64FsRedirection = (lpfnWow64DisableWow64FsRedirection)GetProcAddress(hInstance, "Wow64DisableWow64FsRedirection");
		pfnWow64RevertWow64FsRedirection = (lpfnWow64RevertWow64FsRedirection)GetProcAddress(hInstance, "Wow64RevertWow64FsRedirection");
		// 释放系统动态链接库句柄
		FreeLibrary(hInstance);
	}
	return TRUE;
}

static void GetTrueFilePathName(LPTSTR lpszPath)
{
	size_t len = lstrlen(lpszPath);
	if (!len)
	{
		return;
	}
	if (_tcsnicmp(lpszPath, _T("\\SystemRoot"), 11) == 0)
	{
		TCHAR szWindowsRoot[_MAX_PATH];
		GetWindowsDirectory(szWindowsRoot, sizeof(szWindowsRoot));
		memmove(lpszPath + lstrlen(szWindowsRoot), lpszPath + 11, (len - 10) * sizeof(TCHAR));
		memcpy(lpszPath, szWindowsRoot, lstrlen(szWindowsRoot) * sizeof(TCHAR));
	}
	else if (_tcsnicmp(lpszPath, _T("\\\\??\\"), 4) == 0)
	{
		memmove(lpszPath, lpszPath + 4, (len - 3) * sizeof(TCHAR));
	}
	else if (lpszPath[0] == '\\')
	{
		DWORD dwAllDisk = GetLogicalDrives();
		TCHAR szDriverName[_MAX_PATH];
		TCHAR szDiskPath[_MAX_DRIVE];

		while (dwAllDisk)
		{
			for (INT i=0; i<=26; ++i)
			{
				if ((dwAllDisk & 1))
				{
					_stprintf(szDiskPath, _T("%c:"), 'A' + i);

					QueryDosDevice(szDiskPath, szDriverName, _MAX_PATH);

					if (GetDriveType(szDiskPath) == DRIVE_REMOTE)
					{
						TCHAR szDeviceName[_MAX_PATH];
						TCHAR szSharedName[_MAX_PATH];

						// Like: \Device\HGFS\;z:0000000000010289\vmware-host\Shared Folders
						// szDeviceName = HGFS, szSharedName = vmware-host\Shared Folders
						_stscanf(szDriverName, _T("\\%*[^\\]\\%[^\\]\\%*[^\\]\\%[^\0]"), szDeviceName, szSharedName);
						// \Device\HGFS\vmware-host\Shared Folders\F\X.exe -> Z:\F\X.exe
						// \Device\HGFS\vmware-host\Shared Folders
						_stprintf(szDriverName, _T("\\Device\\%s\\%s"), szDeviceName , szSharedName);
					}
					if (_tcsnicmp(lpszPath, szDriverName, _tcslen(szDriverName)) == 0)
					{
						memmove(lpszPath + lstrlen(szDiskPath), lpszPath + lstrlen(szDriverName), (len - lstrlen(szDriverName) + 1) * sizeof(TCHAR));
						memcpy(lpszPath, szDiskPath, lstrlen(szDiskPath) * sizeof(TCHAR));
						return;
					}
				}
				dwAllDisk = dwAllDisk >> 1;
			}
		}
	}
}

class PrivilegeHelper
{
public:
	PrivilegeHelper(void)
	{
		SetPrivilege(SE_DEBUG_NAME, TRUE);
	}
	~PrivilegeHelper(void)
	{
		SetPrivilege(SE_DEBUG_NAME, FALSE);
	}

private:
	static BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
	{
		TOKEN_PRIVILEGES tp;
		LUID luid;
		HANDLE hToken;

		OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
		if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
		{
			return FALSE;
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;

		if (bEnablePrivilege)
		{
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		}
		else
		{
			tp.Privileges[0].Attributes = 0;
		}

		AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES) NULL, 0); 

		return ((GetLastError() != ERROR_SUCCESS) ? FALSE : TRUE);
	}
};

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

extern "C" BOOL knlFindProcByPath(LPCTSTR lpszExePath)
{
	PrivilegeHelper ph;

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	LPCTSTR lpszFileName = PathFindFileName (lpszExePath);

	PROCESSENTRY32 info = {sizeof(PROCESSENTRY32)};
	if (Process32First(hSnapShot, &info)) 
	{
		do 
		{
			// 文件名一样，再检查是否是相同路径下的
			if (_tcsicmp(info.szExeFile, lpszFileName) == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.th32ProcessID);
				if (hProcess != INVALID_HANDLE_VALUE)
				{
					TCHAR szPath[_MAX_PATH] = {0};
					if (GetProcessImageFileName(hProcess, szPath, _MAX_PATH))
					{
						GetTrueFilePathName(szPath);
						if (_tcsicmp(szPath, lpszExePath) == 0)
						{
							CloseHandle(hProcess);
							return TRUE;
						}
					}
					CloseHandle(hProcess);
				}
			}
		} while (Process32Next(hSnapShot, &info));
	}
	CloseHandle(hSnapShot);
	return FALSE;
}

// static BOOL knlFindProcByName(LPCTSTR lpszExeName)
// {
// 	PrivilegeHelper ph;
// 
// 	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
// 	if (hSnapShot == INVALID_HANDLE_VALUE)
// 	{
// 		return FALSE;
// 	}
// 	
// 	PROCESSENTRY32 info = {sizeof(PROCESSENTRY32)};
// 	if (Process32First(hSnapShot, &info)) 
// 	{
// 		do 
// 		{
// 			if (_tcsicmp(info.szExeFile, lpszExeName) == 0)
// 			{
// 				CloseHandle(hSnapShot);
// 				return TRUE;
// 			}
// 		} while (Process32Next(hSnapShot, &info));
// 	}
// 	CloseHandle(hSnapShot);
// 	return FALSE;
// }

static BOOL knlKillProcByPath(LPCTSTR lpszExePath)
{
	PrivilegeHelper ph;
	
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	LPCTSTR lpszFileName = PathFindFileName (lpszExePath);
	
	PROCESSENTRY32 info = {sizeof(PROCESSENTRY32)};
	if (Process32First(hSnapShot, &info)) 
	{
		do 
		{
			// 文件名一样，再检查是否是相同路径下的
			if (_tcsicmp(info.szExeFile, lpszFileName) == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.th32ProcessID);
				if (hProcess != INVALID_HANDLE_VALUE)
				{
					TCHAR szPath[_MAX_PATH] = {0};
					if (GetProcessImageFileName(hProcess, szPath, _MAX_PATH))
					{
						GetTrueFilePathName(szPath);
						if (_tcsicmp(szPath, lpszExePath) == 0)
						{
							if (TerminateProcess(hProcess, 1))
							{
								WaitForSingleObject(hProcess, INFINITE);
							}
						}
					}
					CloseHandle(hProcess);
				}
			}
		} while (Process32Next(hSnapShot, &info));
	}
	CloseHandle(hSnapShot);
	return FALSE;
}

// static BOOL knlKillProcByName(LPCTSTR lpszExeName)
// {
// 	PrivilegeHelper ph;
// 	
// 	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
// 	if (hSnapShot == INVALID_HANDLE_VALUE)
// 	{
// 		return FALSE;
// 	}
// 	
// 	PROCESSENTRY32 info = {sizeof(PROCESSENTRY32)};
// 	if (Process32First(hSnapShot, &info)) 
// 	{
// 		do 
// 		{
// 			if (_tcsicmp(info.szExeFile, lpszExeName) == 0)
// 			{
// 				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, info.th32ProcessID);
// 				if (hProcess != INVALID_HANDLE_VALUE)
// 				{
// 					if (TerminateProcess(hProcess, 1))
// 					{
// 						WaitForSingleObject(hProcess, INFINITE);
// 						CloseHandle(hProcess);
// 						CloseHandle(hSnapShot);
// 						return TRUE;
// 					}
// 					CloseHandle(hProcess);
// 				}
// 			}
// 		} while (Process32Next(hSnapShot, &info));
// 	}
// 	CloseHandle(hSnapShot);
// 	return FALSE;
// }
// 
// static BOOL knlKillProcByPID(DWORD dwProcessID)
// {
// 	PrivilegeHelper ph;
// 
// 	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwProcessID);
// 	if (hProcess != INVALID_HANDLE_VALUE)
// 	{
// 		if (TerminateProcess(hProcess, 1))
// 		{
// 			WaitForSingleObject(hProcess, INFINITE);
// 			CloseHandle(hProcess);
// 			return TRUE;
// 		}
// 	}
// 	CloseHandle(hProcess);
// 	return FALSE;
// }

extern "C" ANYSHAREDLL_API void FindProcByPath(HWND hwndParent, int string_size,
											   TCHAR *variables, stack_t **stacktop)
{
	TCHAR parameter[MAX_PATH];
	TCHAR temp[10];
	int value;
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	{
		popstring(parameter);
		value = knlFindProcByPath(parameter);
		setuservariable(INST_R1, _itot(value, temp, 10));
	}
}

// extern "C" ANYSHAREDLL_API void FindProcByName(HWND hwndParent, int string_size,
// 												  TCHAR *variables, stack_t **stacktop)
// {
// 	TCHAR parameter[MAX_PATH];
// 	TCHAR temp[10];
// 	int value;
// 	g_hwndParent = hwndParent;
// 	EXDLL_INIT();
// 	{
// 		popstring(parameter);
// 		value = knlFindProcByName(parameter);
// 		setuservariable(INST_R1, _itot(value, temp, 10));
// 	}
// }

extern "C" ANYSHAREDLL_API void KillProcByPath(HWND hwndParent, int string_size,
											   TCHAR *variables, stack_t **stacktop)
{
	TCHAR parameter[MAX_PATH];
	TCHAR temp[10];
	int value;
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	{
		popstring(parameter);
		value = knlKillProcByPath(parameter);
		setuservariable(INST_R1, _itot(value, temp, 10));
	}
}

// extern "C" ANYSHAREDLL_API void KillProcByName(HWND hwndParent, int string_size,
// 												  TCHAR *variables, stack_t **stacktop)
// {
// 	TCHAR parameter[MAX_PATH];
// 	TCHAR temp[10];
// 	int value;
// 	g_hwndParent = hwndParent;
// 	EXDLL_INIT();
// 	{
// 		popstring(parameter);
// 		value = knlKillProcByName(parameter);
// 		setuservariable(INST_R1, _itot(value, temp, 10));
// 	}
// }
// 
// extern "C" ANYSHAREDLL_API void KillProcByPID(HWND hwndParent, int string_size,
// 												 TCHAR *variables, stack_t **stacktop)
// {
// 	TCHAR parameter[MAX_PATH];
// 	TCHAR temp[10];
// 	int value;
// 	g_hwndParent = hwndParent;
// 	EXDLL_INIT();
// 	{
// 		popstring(parameter);
// 		value = atoi(parameter);
// 		value = knlKillProcByPID(value);
// 		setuservariable(INST_R1, _itot(value, temp, 10));
// 	}
// }

enum E_SERVICE_CONTROL
{
	E_SC_START = 0,
	E_SC_STOP,
	E_SC_DELETE
};

static DWORD knlServiceControl(LPCTSTR lpszServiceName, ULONG Type)
{
	SC_HANDLE controlHandle;
	SC_HANDLE serviceHandle;
	SERVICE_STATUS ss;
	DWORD result = NO_ERROR;

	controlHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (controlHandle == NULL)
	{
		result = GetLastError();
		TRACE(_T("Failed to open SCM: %d\n"), result);
		return result;
	}

	serviceHandle = OpenService(controlHandle, lpszServiceName,
		SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);

	if (serviceHandle == NULL)
	{
		result = GetLastError();
		TRACE(_T("Failed to open Service (%s): %d\n"), lpszServiceName, result);
		CloseServiceHandle(controlHandle);
		return result;
	}

	QueryServiceStatus(serviceHandle, &ss);
	if (Type == E_SC_DELETE)
	{
		if (DeleteService(serviceHandle))
		{
			result = NO_ERROR;
			TRACE(_T("Service (%s) deleted\n"), lpszServiceName);
		}
		else
		{
			result = GetLastError();
			TRACE(_T("Failed to delete service (%s): %d\n"), lpszServiceName, result);
		}
	}
	else if (ss.dwCurrentState != SERVICE_RUNNING && Type == E_SC_START)
	{
		if (StartService(serviceHandle, 0, NULL))
		{
			result = NO_ERROR;
			TRACE(_T("Service (%s) started\n"), lpszServiceName);
		}
		else
		{
			result = GetLastError();
			TRACE(L"failed to start service (%s): %d\n", lpszServiceName, result);
		}
	}
	else if ((ss.dwCurrentState != SERVICE_STOP_PENDING || ss.dwCurrentState != SERVICE_STOPPED)
		&& Type == E_SC_STOP)
	{
		if (ControlService(serviceHandle, SERVICE_CONTROL_STOP, &ss))
		{
			result = NO_ERROR;
			TRACE(_T("Service (%s) stopped\n"), lpszServiceName);
		}
		else
		{
			result = GetLastError();
			TRACE(L"Failed to stop service (%s): %d\n", lpszServiceName, result);
		}
	}

	CloseServiceHandle(serviceHandle);
	CloseServiceHandle(controlHandle);

	Sleep(100);
	return result;
}

static DWORD knlServiceExists(LPCTSTR lpszServiceName)
{
	SC_HANDLE controlHandle;
	SC_HANDLE serviceHandle;
	DWORD result = NO_ERROR;

	controlHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (controlHandle == NULL)
	{
		result = GetLastError();
		TRACE(_T("Failed to open SCM: %d\n"), result);
		return result;
	}

	serviceHandle = OpenService(controlHandle, lpszServiceName,
		SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);

	if (serviceHandle == NULL)
	{
		result = GetLastError();
		CloseServiceHandle(controlHandle);
		return result;
	}

	CloseServiceHandle(serviceHandle);
	CloseServiceHandle(controlHandle);
	return result;
}

static DWORD knlInstallService(LPCTSTR lpszServiceName, LPCTSTR lpszDisplayName, DWORD dwServiceType, LPCTSTR lpszServiceFullPath, DWORD isWOW64)
{
	DWORD result = NO_ERROR;
	if (ERROR_SUCCESS != knlServiceExists (lpszServiceName))
	{
		SC_HANDLE	controlHandle;
		SC_HANDLE	serviceHandle;

		controlHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (controlHandle == NULL)
		{
			result = GetLastError();
			TRACE(_T("Failed to open SCM: %d\n"), result);
			return result;
		}

		serviceHandle = CreateService(controlHandle, lpszServiceName, lpszDisplayName, 0,
			dwServiceType, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE,
			lpszServiceFullPath, NULL, NULL, NULL, NULL, NULL);

		if (serviceHandle == NULL)
		{
			result = GetLastError();
			if (result == ERROR_SERVICE_EXISTS)
			{
				TRACE(_T("Service (%s) is already installed\n"), lpszServiceName);
				result = NO_ERROR;
			}
			else
			{
				TRACE(_T("Failed to install service (%s): %d\n"), lpszServiceName, result);
			}
			CloseServiceHandle(controlHandle);
			return result;
		}

		CloseServiceHandle(serviceHandle);
		CloseServiceHandle(controlHandle);

		if (Wow64Helper::IsWow64() && !isWOW64)
		{
			HKEY hKey;
			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\services"), 0, KEY_ALL_ACCESS, &hKey))
			{
				HKEY hSubKey;
				if (ERROR_SUCCESS == RegOpenKeyEx(hKey, lpszServiceName, 0, KEY_ALL_ACCESS, &hSubKey))
				{
					RegDeleteValue(hSubKey, _T("WOW64"));
				}
				RegCloseKey(hSubKey);
			}
			RegCloseKey(hKey);
		}
		TRACE(L"Service (%s) installed\n", lpszServiceName);
	}

	result = knlServiceControl(lpszServiceName, E_SC_START);
	if (result == ERROR_SUCCESS)
	{
		TRACE(_T("Service (%s) started\n"), lpszServiceName);
	}
	else
	{
		TRACE(_T("Service (%s) start failed\n"), lpszServiceName);
	}
	return result;
}

static void DeleteSubKeyTree(HKEY hKey, LPCTSTR lpSubKey)
{
	HKEY hSubKey;
	TCHAR szSubKey[_MAX_PATH] = {0};
	if (ERROR_SUCCESS != RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hSubKey))
	{
		RegCloseKey(hSubKey);
		return;
	}
	DWORD dwIndex = 0;
	DWORD cbName = sizeof(szSubKey) / sizeof(szSubKey[0]);
	while (ERROR_SUCCESS == RegEnumKey(hSubKey, dwIndex, szSubKey, cbName))
	{
		DeleteSubKeyTree(hSubKey, szSubKey);
	}
	RegCloseKey(hSubKey);
	RegDeleteKey(hKey, lpSubKey);
}

static DWORD knlDeleteService(LPCTSTR lpszServiceName)
{
	DWORD result = NO_ERROR;
	result = knlServiceExists(lpszServiceName);
	if (!result)
	{
		knlServiceControl(lpszServiceName, E_SC_STOP);
		TCHAR szBinaryPath[_MAX_PATH] = {0};
		BOOL needDisableRedirection = FALSE;
		HKEY hKey;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\services"), 0, KEY_ALL_ACCESS, &hKey))
		{
			HKEY hSubKey;
			if (ERROR_SUCCESS == RegOpenKeyEx(hKey, lpszServiceName, 0, KEY_ALL_ACCESS, &hSubKey))
			{
				DWORD dwType;
				DWORD dwSize = _MAX_PATH * sizeof(TCHAR);
				if (ERROR_SUCCESS == RegQueryValueEx(hSubKey, _T("ImagePath"), NULL, &dwType, (LPBYTE)szBinaryPath, &dwSize))
				{
					// TODO: if dwType is REG_EXPAND_SZ, use ExpandEnvironmentStrings
					GetTrueFilePathName(szBinaryPath);

					if (Wow64Helper::IsWow64())
					{
						HKEY hKey;
						if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\services"), 0, KEY_ALL_ACCESS, &hKey))
						{
							HKEY hSubKey;
							if (ERROR_SUCCESS == RegOpenKeyEx(hKey, lpszServiceName, 0, KEY_ALL_ACCESS, &hSubKey))
							{
								TCHAR szWOW64[_MAX_PATH] = {0};
								DWORD dwType;
								DWORD dwSize = _MAX_PATH * sizeof(TCHAR);
								LSTATUS status = RegQueryValueEx(hSubKey, _T("WOW64"), NULL, &dwType, (LPBYTE)szWOW64, &dwSize);
								if (status == ERROR_SUCCESS)
								{
									if (_tcscmp (_T("0"), szWOW64) == 0)
									{
										needDisableRedirection = TRUE;
									}
								}
								else if (status == ERROR_FILE_NOT_FOUND)
								{
									needDisableRedirection = TRUE;
								}
							}
							RegCloseKey(hSubKey);
						}
						RegCloseKey(hKey);
					}
				}
			}
			RegCloseKey(hSubKey);
		}
		result = knlServiceControl(lpszServiceName, E_SC_DELETE);
		DeleteSubKeyTree(hKey, lpszServiceName);
		RegCloseKey(hKey);

		Wow64Helper wh (needDisableRedirection);
		DeleteFile(szBinaryPath);
	}
	return result;
}

extern "C" ANYSHAREDLL_API void InstallService(HWND hwndParent, int string_size,
											   TCHAR *variables, stack_t **stacktop)
{
	TCHAR serviceName[MAX_PATH];
	TCHAR displayName[MAX_PATH];
	int type;
	TCHAR binaryPath[MAX_PATH];
	TCHAR temp[10];
	DWORD isWOW64;
	int value;
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	{
		popstring(serviceName);
		popstring(displayName);
		type = popint();
		popstring(binaryPath);
		isWOW64 = popint();
		value = knlInstallService(serviceName, displayName, type, binaryPath, isWOW64);
		setuservariable(INST_R1, _itot(value, temp, 10));
	}
}

extern "C" ANYSHAREDLL_API void StopService(HWND hwndParent, int string_size,
											TCHAR *variables, stack_t **stacktop)
{
	TCHAR serviceName[MAX_PATH];
	TCHAR temp[10];
	int value;
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	{
		popstring(serviceName);
		value = knlServiceControl(serviceName, E_SC_STOP);
		setuservariable(INST_R1, _itot(value, temp, 10));
	}
}

extern "C" ANYSHAREDLL_API void RemoveService(HWND hwndParent, int string_size,
											  TCHAR *variables, stack_t **stacktop)
{
	TCHAR serviceName[MAX_PATH];
	TCHAR temp[10];
	int value;
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	{
		popstring(serviceName);
		value = knlDeleteService(serviceName);
		setuservariable(INST_R1, _itot(value, temp, 10));
	}
}
