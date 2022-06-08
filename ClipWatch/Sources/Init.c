/* ClipWatch - Init & Shutdown code. Entrypoint 'CWMain' is here too. */
#include <ClipWatch.h>

struct CW CW = {
	.Config.FadeInDuration = 1250,
	.Config.MidFadeDelay = 500,
	.Config.FadeOutDuration = 1250,
	.Config.TextColour = RGB(0xFF, 0xFF, 0xFF),
	.Config.BackgroundColour = RGB(0x55, 0x55, 0x55),
	.Config.Flags = CW_CFG_POS_TO_NEAREST
	/* Default text set in CWLoadConfig(). Default font aquired through SystemParametersInfoW() -> NONCLIENTMETRICSW.lfMessageFont in CWLoadConfig(). */
};

/* CWMainEntrypoint - The program's entrypoint. Quite self-explanatory, really. */
DECLSPEC_NORETURN void CWMainEntrypoint(void) {
	#pragma region Initialization
	CW.ProcessModule = GetModuleHandleW(NULL);
	CW.UI.Icon = LoadIconW(CW.ProcessModule, MAKEINTRESOURCEW(CW_IDI_ICON));
	CWLoadConfig();

	const WNDCLASSW WindowClass = {
		.style = 0,
		.lpfnWndProc = CWWindowProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = CW.ProcessModule,
		.hIcon = NULL,
		.hCursor = LoadCursorW(NULL, IDC_ARROW),
		.hbrBackground = CW.UI.BackgroundColourBrush,
		.lpszMenuName = NULL,
		.lpszClassName = L"ClipWatchWndClass"
	};
	const ATOM WindowsClass = RegisterClassW(&WindowClass);
	CW.Windows.Main = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_NOACTIVATE | WS_EX_TOPMOST, MAKEINTATOM(WindowsClass), NULL, WS_POPUP, 0, 0, 1, 1, NULL, NULL, CW.ProcessModule, NULL);
	AddClipboardFormatListener(CW.Windows.Main);

	CW.NotifyIcon.Data.cbSize = sizeof(CW.NotifyIcon.Data);
	CW.NotifyIcon.Data.hWnd = CW.Windows.Main;
	CW.NotifyIcon.Data.uID = 1;
	CW.NotifyIcon.Data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	CW.NotifyIcon.Data.uCallbackMessage = WM_SHLICON;
	CW.NotifyIcon.Data.hIcon = CW.UI.Icon;

	#ifdef NDEBUG
	lstrcpyW(CW.NotifyIcon.Data.szTip, L"ClipWatch");
	#else
	lstrcpyW(CW.NotifyIcon.Data.szTip, L"ClipWatch DEBUG BUILD");
	#endif

	Shell_NotifyIconW(NIM_ADD, &CW.NotifyIcon.Data);

	CW.NotifyIcon.Menu = CreatePopupMenu();
	InsertMenuW(CW.NotifyIcon.Menu, MAXUINT, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, L"ClipWatch");
	InsertMenuW(CW.NotifyIcon.Menu, MAXUINT, MF_BYPOSITION | MF_STRING | MF_SEPARATOR, 0, NULL);
	InsertMenuW(CW.NotifyIcon.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, CW_TIM_CLEAR, L"Clear Clipboard");
	InsertMenuW(CW.NotifyIcon.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, CW_TIM_SETTINGS, L"Settings");
	InsertMenuW(CW.NotifyIcon.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, CW_TIM_ABOUT, L"About");
	InsertMenuW(CW.NotifyIcon.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, CW_TIM_EXIT, L"Exit");
	#pragma endregion

	wcscpy_s(CW.Config.Text, ARRAYSIZE(CW.Config.Text), L"ClipWatch Started!");
	SendMessageW(CW.Windows.Main, WM_CLIPBOARDUPDATE, 0, 0);
	CW.Config.Flags |= CW_CFG_RESET_TEXT;

	MSG Message;
	while(GetMessageW(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessageW(&Message);

		if(CW.PresentPopup && WaitForSingleObject(CW.PresentPopup, 0) != WAIT_TIMEOUT) {
			CloseHandle(CW.PresentPopup);
			CW.PresentPopup = NULL;
		}
	}

	#pragma region Shutdown
	Shell_NotifyIconW(NIM_DELETE, &CW.NotifyIcon.Data);
	DestroyMenu(CW.NotifyIcon.Menu);
	RemoveClipboardFormatListener(CW.Windows.Main);
	if(CW.PresentPopup) {
		WaitForSingleObject(CW.PresentPopup, INFINITE);
		CloseHandle(CW.PresentPopup);
	}
	DestroyWindow(CW.Windows.Main);
	UnregisterClassW(MAKEINTATOM(WindowsClass), CW.ProcessModule);
	#pragma endregion
	ExitProcess(0);
}