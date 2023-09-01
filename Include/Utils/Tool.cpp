#include "Tool.h"
#include <Shobjidl.h>
#include <tchar.h>
#include <io.h>
#include <direct.h>
#include <Shlobj.h> 
#pragma comment(lib, "shell32.lib")

#include <Tlhelp32.h>
#include <algorithm>
#include <iostream>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <wincrypt.h>
#pragma comment(lib, "Advapi32.lib")

namespace Utils
{
	Tool* Tool::m_instance = NULL;

	Tool::Tool()
	{
	}

	Tool::~Tool()
	{
	}

	Tool* Tool::Instance()
	{
		if (m_instance == NULL)
		{
			m_instance = new Tool();
		}

		return m_instance;
	}

	void Tool::Release()
	{
		if (m_instance)
		{
			delete m_instance;
			m_instance = NULL;
		}
	}

	std::wstring Tool::GetCurentPath()
	{
		if (m_currentPath.size() == 0)
		{
			TCHAR chPath[_MAX_PATH];
			TCHAR sFilename[_MAX_PATH];
			TCHAR sDrive[_MAX_DRIVE];
			TCHAR sDir[_MAX_DIR];
			TCHAR sFname[_MAX_FNAME];
			TCHAR sExt[_MAX_EXT];
			GetModuleFileName(NULL, sFilename, _MAX_PATH);
			_tsplitpath_s(sFilename, sDrive, sDir, sFname, sExt);
			_tcscpy_s(chPath, sDrive);
			_tcscat_s(chPath, sDir);
			long nLen = _tcslen(chPath);
			if (chPath[nLen - 1] != TEXT('\\'))
			{
				_tcscat_s(chPath, TEXT("\\"));
			}

			m_currentPath = chPath;
		}

		return m_currentPath;
	}

	void Tool::GetLocalAppDataPath(const TCHAR tszAppName[], TCHAR tszAppDataPath[])
	{
		TCHAR path[MAX_PATH] = { 0 };


		TCHAR tszDefaultDir[MAX_PATH] = { 0 };
		TCHAR tszDocument[MAX_PATH] = { 0 };

		LPITEMIDLIST pidl = NULL;
		SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl);
		if (pidl && SHGetPathFromIDList(pidl, tszDocument))
		{
			GetShortPathName(tszDocument, tszDefaultDir, _MAX_PATH);
		}

		_tcscpy_s(path, MAX_PATH, tszDefaultDir);

		_tcscat_s(path, _T("\\"));
		_tcscat_s(path, tszAppName);

#ifdef UNICODE
		if (_waccess(path, 0) == -1)
		{
			_wmkdir(path);
		}
#else
		if (_access(path, 0) == -1)
		{
			_mkdir(path);
		}
#endif

		_tcscpy_s(tszAppDataPath, MAX_PATH, path);
	}

	DWORD Tool::CreateProcessX(const TCHAR* process, const TCHAR* cmd, int showCmd)
	{
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));

		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USEPOSITION | STARTF_USESIZE;
		si.wShowWindow = showCmd/*SW_HIDE*//*SW_SHOW*/;
		si.dwX = -1;
		si.dwY = -1;
		si.dwXSize = 1;
		si.dwYSize = 1;
		if (CreateProcess(process, (TCHAR*)cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);

			return pi.dwProcessId;
		}

		return 0;
	}

	DWORD Tool::CreateCSharpProcess(const TCHAR* process, const TCHAR* cmd, int showCmd)
	{
		return CreateProcessX(process, cmd, showCmd);
	}

	BOOL Tool::KillProcessByName(TCHAR* tszProcessName)
	{
		if (tszProcessName == NULL || lstrlen(tszProcessName) == 0)
		{
			return false;
		}

#ifdef UNICODE
		std::wstring strA = tszProcessName;
		transform(strA.begin(), strA.end(), strA.begin(), ::towlower);
#else
		std::string strA = tszProcessName;
		transform(strA.begin(), strA.end(), strA.begin(), ::tolower);
#endif


		//创建进程快照(TH32CS_SNAPPROCESS表示创建所有进程的快照)
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		//PROCESSENTRY32进程快照的结构体
		PROCESSENTRY32 pe;

		//实例化后使用Process32First获取第一个快照的进程前必做的初始化操作
		pe.dwSize = sizeof(PROCESSENTRY32);


		//下面的IF效果同:
		//if(hProcessSnap == INVALID_HANDLE_VALUE)   无效的句柄
		if (!Process32First(hSnapShot, &pe))
		{
			return false;
		}

		bool bIsKill = false;

		//如果句柄有效  则一直获取下一个句柄循环下去
		while (Process32Next(hSnapShot, &pe))
		{
			//pe.szExeFile获取当前进程的可执行文件名称

#ifdef UNICODE
			std::wstring scTmp = pe.szExeFile;
			transform(scTmp.begin(), scTmp.end(), scTmp.begin(), ::towlower);
#else
			std::string scTmp = pe.szExeFile;
			transform(scTmp.begin(), scTmp.end(), scTmp.begin(), ::tolower);
#endif

			//比较当前进程的可执行文件名称和传递进来的文件名称是否相同
			//相同的话Compare返回0
			if (strA.compare(scTmp) == 0)
			{
				//从快照进程中获取该进程的PID(即任务管理器中的PID)
				DWORD dwProcessID = pe.th32ProcessID;
				HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
				::TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);

				bIsKill = true;
			}
		}

		CloseHandle(hSnapShot);

		return bIsKill;
	}

	BOOL Tool::KillProcessByPID(DWORD pid)
	{
		HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pid);
		if (!hProcess)
		{
			return FALSE;
		}

		if (::TerminateProcess(hProcess, 0))
		{
			CloseHandle(hProcess);
			return FALSE;
		}

		CloseHandle(hProcess);
		return TRUE;
	}

	BOOL Tool::IsPIDExist(DWORD pid)
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (hProcess == NULL)
		{
			return FALSE;
		}

		CloseHandle(hProcess);
		return TRUE;
	}

	void Tool::ColorString2RGB(std::string color, int& r, int& g, int& b)
	{
		char* str;
		long color_int = strtol(color.c_str(), &str, 16);

		r = (color_int & 0xff0000) >> 16;
		g = (color_int & 0x00ff00) >> 8;
		b = (color_int & 0x0000ff);
	}

	HRESULT Tool::CloakWindow(BOOL cloakHwnd, HWND hwnd)
	{
		HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloakHwnd, sizeof(cloakHwnd));

		return hr;
	}

	BOOL Tool::GetMD5(std::wstring srcFile, std::wstring& fileMD5)
	{
		HANDLE hFile = CreateFile(srcFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)                                        //如果CreateFile调用失败  
		{
			return FALSE;
		}

		DWORD dwFileSize = GetFileSize(hFile, 0);    //获取文件的大小
		if (dwFileSize == 0xFFFFFFFF)               //如果获取文件大小失败  
		{
			CloseHandle(hFile);
			return FALSE;
		}

		HCRYPTPROV hProv = NULL;
		if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)       //获得CSP中一个密钥容器的句柄
		{
			CloseHandle(hFile);
			return FALSE;
		}

		HCRYPTPROV hHash = NULL;
		//初始化对数据流的hash，创建并返回一个与CSP的hash对象相关的句柄。这个句柄接下来将被    CryptHashData调用。
		if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)
		{
			CloseHandle(hFile);
			CryptReleaseContext(hProv, 0);
			return FALSE;
		}

		if (dwFileSize > 1024 * 1024 * 20)
		{
			// 大于 10M  只读取 20M
			dwFileSize = 1024 * 1024 * 20;
		}

		byte* lpReadFileBuffer = new byte[dwFileSize];
		DWORD lpReadNumberOfBytes;
		if (ReadFile(hFile, lpReadFileBuffer, dwFileSize, &lpReadNumberOfBytes, NULL) == 0)        //读取文件  
		{
			CloseHandle(hFile);
			delete[]lpReadFileBuffer;

			if (hHash)
				CryptDestroyHash(hHash);
			if (hProv)
				CryptReleaseContext(hProv, 0);

			return FALSE;
		}

		if (CryptHashData(hHash, lpReadFileBuffer, lpReadNumberOfBytes, 0) == FALSE)      //hash文件  
		{
			CloseHandle(hFile);
			delete[]lpReadFileBuffer;

			if (hHash)
				CryptDestroyHash(hHash);
			if (hProv)
				CryptReleaseContext(hProv, 0);

			return FALSE;
		}

		delete[] lpReadFileBuffer;
		CloseHandle(hFile);          //关闭文件句柄
		BYTE* pbHash;
		DWORD dwHashLen = sizeof(DWORD);

		if (!CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0)) //我也不知道为什么要先这样调用CryptGetHashParam，这块是参照的msdn       
		{
			return FALSE;
		}

		pbHash = (byte*)malloc(dwHashLen);

		if (CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0))//获得md5值 
		{
			for (DWORD i = 0; i < dwHashLen; i++)         //输出md5值 
			{
				// TCHAR str[2] = { 0 };
				WCHAR str[16] = { 0 };
				swprintf_s(str, _T("%02x"), pbHash[i]);
				fileMD5 += str;
			}
		}

		free(pbHash);

		if (hHash)
			CryptDestroyHash(hHash);
		if (hProv)
			CryptReleaseContext(hProv, 0);

		return TRUE;
	}

	BOOL Tool::GetMD5(std::string srcFile, std::string& fileMD5)
	{
		HANDLE hFile = CreateFileA(srcFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)                                        //如果CreateFile调用失败  
		{
			return FALSE;
		}

		DWORD dwFileSize = GetFileSize(hFile, 0);    //获取文件的大小
		if (dwFileSize == 0xFFFFFFFF)               //如果获取文件大小失败  
		{
			CloseHandle(hFile);
			return FALSE;
		}

		HCRYPTPROV hProv = NULL;
		if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)       //获得CSP中一个密钥容器的句柄
		{
			CloseHandle(hFile);
			return FALSE;
		}

		HCRYPTPROV hHash = NULL;
		//初始化对数据流的hash，创建并返回一个与CSP的hash对象相关的句柄。这个句柄接下来将被    CryptHashData调用。
		if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)
		{
			CloseHandle(hFile);
			CryptReleaseContext(hProv, 0);
			return FALSE;
		}

		if (dwFileSize > 1024 * 1024 * 20)
		{
			// 大于 10M  只读取 20M
			dwFileSize = 1024 * 1024 * 20;
		}

		byte* lpReadFileBuffer = new byte[dwFileSize];
		DWORD lpReadNumberOfBytes;
		if (ReadFile(hFile, lpReadFileBuffer, dwFileSize, &lpReadNumberOfBytes, NULL) == 0)        //读取文件  
		{
			CloseHandle(hFile);
			delete[]lpReadFileBuffer;

			if (hHash)
				CryptDestroyHash(hHash);
			if (hProv)
				CryptReleaseContext(hProv, 0);

			return FALSE;
		}

		if (CryptHashData(hHash, lpReadFileBuffer, lpReadNumberOfBytes, 0) == FALSE)      //hash文件  
		{
			CloseHandle(hFile);
			delete[]lpReadFileBuffer;

			if (hHash)
				CryptDestroyHash(hHash);
			if (hProv)
				CryptReleaseContext(hProv, 0);

			return FALSE;
		}

		delete[] lpReadFileBuffer;
		CloseHandle(hFile);          //关闭文件句柄
		BYTE* pbHash;
		DWORD dwHashLen = sizeof(DWORD);

		if (!CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0)) //我也不知道为什么要先这样调用CryptGetHashParam，这块是参照的msdn       
		{
			return FALSE;
		}

		pbHash = (byte*)malloc(dwHashLen);

		if (CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0))//获得md5值 
		{
			for (DWORD i = 0; i < dwHashLen; i++)         //输出md5值 
			{
				// TCHAR str[2] = { 0 };
				char str[16] = { 0 };
				sprintf_s(str, "%02x", pbHash[i]);
				fileMD5 += str;
			}
		}

		free(pbHash);

		if (hHash)
			CryptDestroyHash(hHash);
		if (hProv)
			CryptReleaseContext(hProv, 0);

		return TRUE;
	}

	void Tool::CenterWindow(HWND hWnd)
	{
		RECT rect{ };
		::GetWindowRect(hWnd, &rect);
		const auto width{ rect.right - rect.left };
		const auto height{ rect.bottom - rect.top };
		const auto cx{ ::GetSystemMetrics(SM_CXFULLSCREEN) }; // 取显示器屏幕高宽
		const auto cy{ ::GetSystemMetrics(SM_CYFULLSCREEN) };
		const auto x{ cx / 2 - width / 2 };
		const auto y{ cy / 2 - height / 2 };
		::MoveWindow(hWnd, x, y, width, height, false); // 移动窗口位置居中
	}
}