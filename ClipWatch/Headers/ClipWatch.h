/* ClipWatch - Main header file */
#pragma once

#define _WIN32_WINNT 0x0600
#include <Windows.h>
#include <CommCtrl.h>
#include <Resources.h>
#include <Types.h>

#define BIT(Position) (1 << Position)

/* Window Message sent by the tray icon */
#define WM_SHLICON WM_APP

/* Tray Icon Menu IDs */
enum {
	CW_TIM_CLEAR = 1,
	CW_TIM_SETTINGS,
	CW_TIM_ABOUT,
	CW_TIM_EXIT
};

/* Config Flags*/
enum {
	CW_CFG_ADVANCED_TIMING = BIT(0),
	CW_CFG_POS_TO_CARET = BIT(1),
	CW_CFG_POS_TO_CURSOR = BIT(2),
	CW_CFG_POS_TO_NEAREST = BIT(3),
	CW_CFG_POS_TO_PRIMARY = BIT(4)
};

extern struct CW {
	HMODULE ProcessModule;
	HANDLE AnimatorEvent, AnimatorThread;

	NOTIFYICONDATAW NotifyIcon;
	HANDLE Menu;

	struct {
		HBRUSH BackgroundColourBrush;
		HICON Icon;
		HFONT Font;
	} UI;

	struct {
		HWND Main; /* Popup window, tray icon, and messages */
		HWND Settings, About;
		ATOM Class;
	} Windows;

	struct {
		word FadeInDuration, MidFadeDelay, FadeOutDuration;
		COLORREF TextColour, BackgroundColour;
		LOGFONTW LogFont;
		byte Flags;
	} Config;

	bool Running;
} CW;

/* Window.c */
extern __forceinline void CWCreateWindowClass(void);
dword WINAPI CWWindowAnimator(void* Unused);

/* Init.c */
DECLSPEC_NORETURN
void CWQuit(void);

/* About.c */
INT_PTR CALLBACK CWAboutDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam);

/* Config.c */
INT_PTR CALLBACK CWSettingsDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam);
void CWLoadConfig(void);
void CWSaveConfig(void);
