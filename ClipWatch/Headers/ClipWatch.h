/* ClipWatch - 
*/
#pragma once

#define _WIN32_WINNT 0x0600
#include <Windows.h>
#include <Resources.h>

#define WM_SHLICON WM_APP

#define TIM_SETTINGS 1
#define TIM_ABOUT 2
#define TIM_EXIT 3

extern struct CW {
	HMODULE ProcessModule;
	HANDLE AnimatorEvent;
	HANDLE AnimatorThread;
	HBRUSH BackgroundColourBrush;
	HFONT Font;
	HANDLE Menu;
	HWND WindowHandle;
	ATOM WindowClass;
	_Bool Running;
	NOTIFYICONDATAW NotifyIcon;
} CW;

extern __forceinline void CWCreateWindowClass(void);
extern __forceinline void CWCreateWindow(void);
DWORD WINAPI CWWindowAnimator(void* Unused);

DECLSPEC_NORETURN
extern __forceinline void CWQuit(void);
