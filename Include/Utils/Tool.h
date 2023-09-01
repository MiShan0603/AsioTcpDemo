#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace Utils
{
	class Tool
	{
	public:
		~Tool();

		static Tool* Instance();
		static void Release();

		std::wstring GetCurentPath();
		void GetLocalAppDataPath(const TCHAR tszAppName[], TCHAR tszAppDataPath[]);

		DWORD CreateProcessX(const TCHAR* process, const TCHAR* cmd, int showCmd);
		DWORD CreateCSharpProcess(const TCHAR* process, const TCHAR* cmd, int showCmd);

		BOOL KillProcessByName(TCHAR* tszProcessName);
		BOOL KillProcessByPID(DWORD pid);
		BOOL IsPIDExist(DWORD pid);


		void ColorString2RGB(std::string color, int& r, int& g, int& b);

		/// <summary>
		/// 窗口不参与 DWM 合成
		/// </summary>
		/// <param name="cloakHwnd"></param>
		/// <param name="hwnd"></param>
		/// <returns></returns>
		HRESULT CloakWindow(BOOL cloakHwnd, HWND hwnd);

		/// <summary>
		/// 获取当前文件的md5值
		/// </summary>
		/// <param name="srcFile"></param>
		/// <param name="fileMD5"></param>
		/// <returns></returns>
		BOOL GetMD5(std::wstring srcFile, std::wstring& fileMD5);
		BOOL GetMD5(std::string srcFile, std::string& fileMD5);

		/// <summary>
		/// 窗口移动到屏幕中间
		/// </summary>
		/// <param name="hWnd"></param>
		void CenterWindow(HWND hWnd);

	private:
		Tool();
		
		static Tool* m_instance;

	private:
		std::wstring m_currentPath;
	};
};
