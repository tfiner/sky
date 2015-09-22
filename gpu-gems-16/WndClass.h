/*
s_p_oneil@hotmail.com
Copyright (c) 2000, Sean O'Neil
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* Neither the name of this project nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __WndClass_h__
#define __WndClass_h__

#include "Resource.h"


// Window and control wrapper classes
class CRect : public RECT
{
public:
	CRect()											{}
	CRect(int x1, int y1, int x2, int y2)			{ left = x1; top = y1; right = x2; bottom = y2; }
	CRect(POINT tl, POINT br)						{ left = tl.x; top = tl.y; right = br.x; bottom = br.y; }
	int Width()										{ return right - left; }
	int Height()									{ return bottom - top; }
	void SetRect(int x1, int y1, int x2, int y2)	{ left = x1; top = y1; right = x2; bottom = y2; }
	void SetRect(POINT tl, POINT br)				{ left = tl.x; top = tl.y; right = br.x; bottom = br.y; }
	void SetRectEmpty()								{ left = top = right = bottom = 0; }
};

class CWnd
{
public:
	HWND m_hWnd;

	CWnd(HWND hWnd=NULL)							{ m_hWnd = hWnd; }
	virtual ~CWnd()									{}

	operator HWND()									{ return m_hWnd; }
	bool operator==(HWND hWnd)						{ return hWnd == m_hWnd; }
	bool operator!=(HWND hWnd)						{ return hWnd != m_hWnd; }
	void operator=(HWND hWnd)						{ m_hWnd = hWnd; }

	BOOL CreateEx(HINSTANCE hInstance, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwExStyle, DWORD dwStyle, LPCRECT pRect, HWND hParent=NULL, HMENU hMenu=NULL, LPVOID lpParam=NULL)
	{
		m_hWnd = ::CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, pRect->left, pRect->top, pRect->right-pRect->left, pRect->bottom-pRect->top, hParent, hMenu, hInstance, lpParam);
		return (m_hWnd != NULL);
	}
	BOOL DestroyWindow()							{ return ::DestroyWindow(m_hWnd); }

	long SetWindowLong(int nIndex, long nValue)		{ return ::SetWindowLong(m_hWnd, nIndex, nValue); }
	long GetWindowLong(int nIndex)					{ return ::GetWindowLong(m_hWnd, nIndex); }
	DWORD GetStyle()								{ return (DWORD)::GetWindowLong(m_hWnd, GWL_STYLE); }
	void SetStyle(DWORD dw)							{ ::SetWindowLong(m_hWnd, GWL_STYLE, dw); }
	DWORD GetExStyle()								{ return (DWORD)::GetWindowLong(m_hWnd, GWL_EXSTYLE); }
	void SetExStyle(DWORD dw)						{ ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dw); }

	LRESULT SendMessage(UINT n, WPARAM w=0, LPARAM l=0)	{ return ::SendMessage(m_hWnd, n, w, l); }
	BOOL PostMessage(UINT n, WPARAM w=0, LPARAM l=0){ return ::PostMessage(m_hWnd, n, w, l); }
	void SetWindowText(LPCTSTR psz)					{ ::SetWindowText(m_hWnd, psz); }
	int GetWindowText(LPTSTR psz, int n)			{ return ::GetWindowText(m_hWnd, psz, n); }
	int GetWindowTextLength()						{ return ::GetWindowTextLength(m_hWnd); }
	
	void UpdateWindow()								{ ::UpdateWindow(m_hWnd); }
	BOOL ShowWindow(int nCmdShow)					{ return ::ShowWindow(m_hWnd, nCmdShow); }
	BOOL EnableWindow(BOOL bEnable)					{ return ::EnableWindow(m_hWnd, bEnable); }
	BOOL IsWindowEnabled()							{ return ::IsWindowEnabled(m_hWnd); }
	BOOL IsWindowVisible()							{ return ::IsWindowVisible(m_hWnd); }
	BOOL IsIconic()									{ return ::IsIconic(m_hWnd); }
	BOOL IsZoomed()									{ return ::IsZoomed(m_hWnd); }
	void MoveWindow(LPCRECT pRect, BOOL b=TRUE)		{ MoveWindow(pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, b); }
	void MoveWindow(int x, int y, int w, int h, BOOL b=TRUE)	{ ::MoveWindow(m_hWnd, x, y, w, h, b); }
	BOOL SetWindowPos(HWND hWnd, LPCRECT pRect, UINT n=0)	{ return ::SetWindowPos(m_hWnd, hWnd, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, n); }
	BOOL SetWindowPos(HWND hWnd, int x, int y, int w, int h, UINT n=0)	{ return ::SetWindowPos(m_hWnd, hWnd, x, y, w, h, n); }
	void BringWindowToTop()							{ ::BringWindowToTop(m_hWnd); }
	void GetWindowRect(LPRECT pRect)				{ ::GetWindowRect(m_hWnd, pRect); }
	void GetClientRect(LPRECT pRect)				{ ::GetClientRect(m_hWnd, pRect); }
	void ClientToScreen(LPPOINT pPoint)				{ ::ClientToScreen(m_hWnd, pPoint); }
	void ClientToScreen(LPRECT pRect)				{ ::ClientToScreen(m_hWnd, (LPPOINT)pRect); ::ClientToScreen(m_hWnd, ((LPPOINT)pRect)+1); }
	void ScreenToClient(LPPOINT pPoint)				{ ::ScreenToClient(m_hWnd, pPoint); }
	void ScreenToClient(LPRECT pRect)				{ ::ScreenToClient(m_hWnd, (LPPOINT)pRect); ::ScreenToClient(m_hWnd, ((LPPOINT)pRect)+1); }
	void CalcWindowRect(LPRECT pRect, UINT n=0)		{ ::AdjustWindowRect(pRect, GetStyle(), FALSE); }

	HWND GetActiveWindow()							{ return ::GetActiveWindow(); }
	HWND SetActiveWindow()							{ return ::SetActiveWindow(m_hWnd); }
	HWND GetCapture()								{ return ::GetCapture(); }
	HWND SetCapture()								{ return ::SetCapture(m_hWnd); }
	HWND GetFocus()									{ return ::GetFocus(); }
	HWND SetFocus()									{ return ::SetFocus(m_hWnd); }
	HWND GetParent()								{ return ::GetParent(m_hWnd); }
	HWND SetParent(HWND hWnd)						{ return ::SetParent(m_hWnd, hWnd); }
	HICON SetIcon(HICON h, BOOL b)					{ return (HICON)::SendMessage(m_hWnd, WM_SETICON, b, (LPARAM)h); }
	HICON GetIcon(BOOL b)							{ return (HICON)::SendMessage(m_hWnd, WM_GETICON, b, 0); }
	HMENU GetMenu()									{ return ::GetMenu(m_hWnd); }
	BOOL SetMenu(HMENU hMenu)						{ return ::SetMenu(m_hWnd, hMenu); }
	void DrawMenuBar()								{ ::DrawMenuBar(m_hWnd); }
	void RedrawWindow(LPCRECT pRect, UINT n)		{ ::RedrawWindow(m_hWnd, pRect, NULL, n); }
	int GetDlgCtrlID()								{ return ::GetDlgCtrlID(m_hWnd); }

	void SetRedraw(BOOL b)							{ ::SendMessage(m_hWnd, WM_SETREDRAW, b, 0); }
	void Invalidate(BOOL b=TRUE, LPCRECT pRect=NULL){ ::InvalidateRect(m_hWnd, pRect, b); }
	void Validate(LPCRECT pRect=NULL)				{ ::ValidateRect(m_hWnd, pRect); }
	UINT SetTimer(UINT nID, UINT nElapse)			{ return ::SetTimer(m_hWnd, nID, nElapse, NULL); }
	BOOL KillTimer(int nID)							{ return ::KillTimer(m_hWnd, nID); }
};


class CWinApp {
// Attributes
protected:
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	const char *m_pszCmdLine;
	int m_nShowCmd;
	char m_szAppName[_MAX_PATH];
	char m_szAppPath[_MAX_PATH];
	char m_szStartupPath[_MAX_PATH];
	char m_szRegistryKey[_MAX_PATH];

   CWnd wnd_;

public:
	static CWinApp *m_pMainApp;

// Operations
protected:

public:	
	CWinApp(HINSTANCE hInstance, HINSTANCE hPrevInstance=NULL, char *pszCmdLine="", int nShowCmd=SW_SHOWNORMAL)
	{
		m_pMainApp = this;
		m_hInstance = hInstance;
		m_hPrevInstance = hPrevInstance;
		m_pszCmdLine = pszCmdLine;
		m_nShowCmd = nShowCmd;
		LoadString(IDS_APPLICATION, m_szAppName);
		::GetModuleFileName(NULL, m_szAppPath, _MAX_PATH);
		::GetCurrentDirectory(_MAX_PATH, m_szStartupPath);
		sprintf(m_szRegistryKey, "Software\\%s", m_szAppName);
	}
	~CWinApp()
	{
		m_pMainApp = NULL;
	}

	virtual bool InitInstance()					{ return false; }
	virtual int ExitInstance()					{ return 0; }
	virtual bool OnIdle()						{ return false; }
	virtual bool PreTranslateMessage(MSG *pMsg)	{ return false; }
	virtual int Run()
	{
		MSG msg;
		msg.message = 0;
		while(msg.message != WM_QUIT)
		{
			OnIdle();
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if(msg.message == WM_QUIT)
					break;
				if(!PreTranslateMessage(&msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		return 0;
	}

	HINSTANCE GetInstanceHandle()				{ return m_hInstance; }
	const char *GetAppName()					{ return m_szAppName; }
	const char *GetAppPath()					{ return m_szAppPath; }
	const char *GetStartupPath()				{ return m_szStartupPath; }
	const char *GetRegistryKey()				{ return m_szRegistryKey; }
	void SetAppName(const char *psz)			{ strcpy(m_szAppName, psz); }
	void SetAppPath(const char *psz)			{ strcpy(m_szAppPath, psz); }
	void SetStartupPath(const char *psz)		{ strcpy(m_szStartupPath, psz); }
	void SetRegistryKey(const char *psz)		{ strcpy(m_szRegistryKey, psz); }

	int MessageBox(const char *psz, UINT uType=MB_OK)		{ return ::MessageBox(NULL, psz, m_szAppName, uType); }
	int LoadString(int nID, char *psz, int nMax=_MAX_PATH)	{ return ::LoadString(m_hInstance, nID, psz, nMax); }
	int GetProfileInt(const char *pszSection, const char *pszEntry, int nDefault=0)
	{
		HKEY hKey;
		DWORD dw, dwType, dwValue;
		char szBuffer[_MAX_PATH];
		sprintf(szBuffer, "%s\\%s", m_szRegistryKey, pszSection);
		if(RegCreateKeyEx(HKEY_CURRENT_USER, szBuffer, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw) == ERROR_SUCCESS)
		{
			dw = sizeof(DWORD);
			if(RegQueryValueEx(hKey, pszEntry, NULL, &dwType, (unsigned char *)&dwValue, &dw) == ERROR_SUCCESS && dwType == REG_DWORD)
				nDefault = dwValue;
			RegCloseKey(hKey);
		}
		return nDefault;
	}
	bool WriteProfileInt(const char *pszSection, const char *pszEntry, int nValue)
	{
		HKEY hKey;
		DWORD dw;
		char szBuffer[_MAX_PATH];
		sprintf(szBuffer, "%s\\%s", m_szRegistryKey, pszSection);
		bool bSuccess = false;
		if(RegCreateKeyEx(HKEY_CURRENT_USER, szBuffer, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw) == ERROR_SUCCESS)
		{
			bSuccess = (RegSetValueEx(hKey, pszEntry, 0, REG_DWORD, (unsigned char *)&nValue, sizeof(DWORD)) == ERROR_SUCCESS);
			RegCloseKey(hKey);
		}
		return bSuccess;
	}
	const char *GetProfileString(const char *pszSection, const char *pszEntry, const char *pszDefault="")
	{
		HKEY hKey;
		DWORD dw, dwType;
		static char szBuffer[_MAX_PATH];
		sprintf(szBuffer, "%s\\%s", m_szRegistryKey, pszSection);
		if(RegCreateKeyEx(HKEY_CURRENT_USER, szBuffer, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw) == ERROR_SUCCESS)
		{
			dw = _MAX_PATH;
			if(RegQueryValueEx(hKey, pszEntry, NULL, &dwType, (unsigned char *)szBuffer, &dw) == ERROR_SUCCESS && dwType == REG_SZ)
				pszDefault = szBuffer;
			RegCloseKey(hKey);
		}
		return pszDefault;
	}
	bool WriteProfileString(const char *pszSection, const char *pszEntry, const char *pszValue)
	{
		HKEY hKey;
		DWORD dw;
		char szBuffer[_MAX_PATH];
		sprintf(szBuffer, "%s\\%s", m_szRegistryKey, pszSection);
		bool bSuccess = false;
		if(RegCreateKeyEx(HKEY_CURRENT_USER, szBuffer, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw) == ERROR_SUCCESS)
		{
			bSuccess = pszValue ? (RegSetValueEx(hKey, pszEntry, 0, REG_SZ, (unsigned char *)pszValue, strlen(pszValue)+1) == ERROR_SUCCESS) :
								  (RegDeleteValue(hKey, pszEntry) == ERROR_SUCCESS);
			RegCloseKey(hKey);
		}
		return bSuccess;
	}
};

inline CWinApp *GetApp()		{ return CWinApp::m_pMainApp; }

#endif // __WndClass_h__
