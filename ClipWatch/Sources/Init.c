#include <ClipWatch.h>

struct CW CW;

static __forceinline void CWInit(void) {
	CW.ProcessModule = GetModuleHandleW(NULL);
	CW.BackgroundColourBrush = CreateSolidBrush(RGB(0x33, 0x33, 0x33));

	NONCLIENTMETRICSW Metrics;
	Metrics.cbSize = sizeof(Metrics);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(Metrics), &Metrics, 0);
	CW.Font = CreateFontIndirectW(&Metrics.lfMessageFont);

	CWCreateWindowClass();
	CWCreateWindow();
	AddClipboardFormatListener(CW.WindowHandle);
	CW.AnimatorEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	CW.AnimatorThread = CreateThread(NULL, 1, CWWindowAnimator, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);

	CW.NotifyIcon.cbSize = sizeof(CW.NotifyIcon);
	CW.NotifyIcon.hWnd = CW.WindowHandle;
	CW.NotifyIcon.uID = 1;
	CW.NotifyIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	CW.NotifyIcon.uCallbackMessage = WM_SHLICON;
	CW.NotifyIcon.hIcon = LoadIconW(CW.ProcessModule, MAKEINTRESOURCEW(IDI_ICON));

	#ifdef NDEBUG
	lstrcpyW(CW.NotifyIcon.szTip, L"ClipWatch");
	#else
	lstrcpyW(CW.NotifyIcon.szTip, L"ClipWatch DEBUG BUILD");
	#endif

	Shell_NotifyIconW(NIM_ADD, &CW.NotifyIcon);

	CW.Menu = CreatePopupMenu();
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, L"ClipWatch");
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING | MF_SEPARATOR, 0, NULL);
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, TIM_SETTINGS, L"Settings");
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, TIM_ABOUT, L"About");
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, TIM_EXIT, L"Exit");

	CW.Running = TRUE;
	return;
}

DECLSPEC_NORETURN
__forceinline void CWQuit(void) {
	CW.Running = FALSE;
	SetEvent(CW.AnimatorEvent);

	RemoveClipboardFormatListener(CW.WindowHandle);
	DestroyWindow(CW.WindowHandle);
	UnregisterClassW(MAKEINTATOM(CW.WindowClass), CW.ProcessModule);

	DeleteObject(CW.BackgroundColourBrush);
	DeleteObject(CW.Font);
	DestroyMenu(CW.Menu);
	Shell_NotifyIconW(NIM_DELETE, &CW.NotifyIcon);

	WaitForSingleObject(CW.AnimatorThread, INFINITE);
	CloseHandle(CW.AnimatorThread);
	CloseHandle(CW.AnimatorEvent);
	ExitProcess(0);
}

DECLSPEC_NORETURN
void CWMain(void) {
	CWInit();

	/* The window proc calls the shutdown. I originally had the shutting down in here,
	but for some reason the WM_QUIT messages never made it in. */
	while(TRUE) {
		MSG Message;
		GetMessageW(&Message, NULL, 0, 0);
		TranslateMessage(&Message);
		DispatchMessageW(&Message);
	}
}
