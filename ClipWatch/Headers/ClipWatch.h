/* ClipWatch - Main header file */
#pragma once

#define _WIN32_WINNT 0x0600
#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h> /* _wcsspnp() is used once in the config dialog. */
#include <Resources.h>
#include <Types.h>

#define BIT(Position) (1 << Position)

/* Window Messages */
enum {
	WM_SHLICON = WM_APP,
	WM_UPDATE_PREVIEW_COLOURS,
	WM_APPLY_CONFIG
};

/* Config Flags*/
enum {
	CW_CFG_ADVANCED_TIMING = BIT(0),
	CW_CFG_POS_TO_CURSOR = BIT(1),
	CW_CFG_POS_TO_NEAREST = BIT(2),
	CW_CFG_POS_TO_PRIMARY = BIT(3),
	CW_CFG_RESET_TEXT = BIT(4)
};

/* Tray Icon Menu IDs */
enum {
	CW_TIM_CLEAR = 1,
	CW_TIM_SETTINGS,
	CW_TIM_ABOUT,
	CW_TIM_EXIT
};

extern struct CW {
	HMODULE ProcessModule;
	HANDLE PresentPopup;

	struct {
		NOTIFYICONDATAW Data;
		HANDLE Menu;
	} NotifyIcon;

	struct {
		HWND Main, Settings, About;
		HANDLE PopupPresenterThread; /* AnimateWindow() does not return before animation is finished, keep processing messages in that time. */
	} Windows;

	struct {
		HBRUSH BackgroundColourBrush, UpdateBGBrush;
		HICON Icon;
		HFONT Font, UpdateFont;
	} UI;

	struct {
		word FadeInDuration, MidFadeDelay, FadeOutDuration;
		wchar Text[33];
		COLORREF TextColour, BackgroundColour;
		LOGFONTW LogicalFontData;
		byte Flags;
	} Config;
} CW;

/* Main Window.c */
LRESULT CALLBACK CWWindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

/* About.c */
intptr CALLBACK CWAboutDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam);

/* Config Dialog.c */
intptr CALLBACK CWSettingsDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam);

/* Config Utils.c */
void CWLoadConfig(void);
bool CWSaveConfig(void);