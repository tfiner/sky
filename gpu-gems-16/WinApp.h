#pragma once

struct GLFWwindow;
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

class WinApp {
public:
   static WinApp *m_pMainApp;

   WinApp(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine = "") {
      m_pMainApp = this;
      m_hInstance = hInstance;
      m_hPrevInstance = hPrevInstance;
      m_pszCmdLine = pszCmdLine;
   }

   ~WinApp() = default;

   GLFWwindow* GetWindow() const { return window_; }
   virtual bool InitInstance()   { return false; }
   virtual int ExitInstance()		{ return 0; }
   virtual bool OnIdle()			{ return false; }

protected:
   GLFWwindow* window_;

private:
   HINSTANCE m_hInstance;
   HINSTANCE m_hPrevInstance;
   const char *m_pszCmdLine;
   int m_nShowCmd;
};


inline WinApp* GetApp() { return WinApp::m_pMainApp; }

