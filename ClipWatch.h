#pragma once

#define _WIN32_WINNT 0x0600
#include <Windows.h>

extern struct CW {
	HMODULE ProcessModule;
	HANDLE AnimatorThread;
	HANDLE AnimatorEvent;
	HBRUSH BackgroundColourBrush;
	HFONT Font;
	HMENU Menu;
	HWND Window;
	ATOM Class;
	NOTIFYICONDATAW NotifyIcon;
} CW;

extern __forceinline void CWCreateWindowClass(void);
extern __forceinline void CWCreateWindow(void);
DWORD WINAPI CWWindowAnimator(void* Unused);
extern __forceinline void CWQuit(void);
