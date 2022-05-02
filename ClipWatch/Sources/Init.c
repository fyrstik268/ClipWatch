/* ClipWatch - Init & Shutdown code. Entrypoint 'CWMain' is here too. */
#include <ClipWatch.h>

struct CW CW;

static __forceinline void CWInit(void) {
	CW.ProcessModule = GetModuleHandleW(NULL);
	CWLoadConfig();

	CW.UI.Icon = LoadIconW(CW.ProcessModule, MAKEINTRESOURCEW(IDI_ICON));

	CW.Windows.Main = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_NOACTIVATE | WS_EX_TOPMOST, MAKEINTATOM(CW.Windows.Class), NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, CW.ProcessModule, NULL);

	AddClipboardFormatListener(CW.Windows.Main);
	CW.AnimatorEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	CW.AnimatorThread = CreateThread(NULL, 1, CWWindowAnimator, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);

	CW.NotifyIcon.cbSize = sizeof(CW.NotifyIcon);
	CW.NotifyIcon.hWnd = CW.Windows.Main;
	CW.NotifyIcon.uID = 1;
	CW.NotifyIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	CW.NotifyIcon.uCallbackMessage = WM_SHLICON;
	CW.NotifyIcon.hIcon = CW.UI.Icon;

	#ifdef NDEBUG
	lstrcpyW(CW.NotifyIcon.szTip, L"ClipWatch");
	#else
	lstrcpyW(CW.NotifyIcon.szTip, L"ClipWatch DEBUG BUILD");
	#endif

	Shell_NotifyIconW(NIM_ADD, &CW.NotifyIcon);

	CW.Menu = CreatePopupMenu();
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, L"ClipWatch");
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING | MF_SEPARATOR, 0, NULL);
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, CW_TIM_CLEAR, L"Clear Clipboard");
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, CW_TIM_SETTINGS, L"Settings");
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, CW_TIM_ABOUT, L"About");
	InsertMenuW(CW.Menu, MAXUINT, MF_BYPOSITION | MF_STRING, CW_TIM_EXIT, L"Exit");

	CW.Running = true;
	return;
}

DECLSPEC_NORETURN
void CWQuit(void) {
	CW.Running = false;
	SetEvent(CW.AnimatorEvent);

	if(CW.Windows.About) DestroyWindow(CW.Windows.About);
	if(CW.Windows.Settings) DestroyWindow(CW.Windows.Settings);

	RemoveClipboardFormatListener(CW.Windows.Main);
	DestroyWindow(CW.Windows.Main);
	UnregisterClassW(MAKEINTATOM(CW.Windows.Class), CW.ProcessModule);

	DeleteObject(CW.UI.BackgroundColourBrush);
	DeleteObject(CW.UI.Font);
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
	but for some reason the WM_QUIT messages never made it in. Shutdown performed within CWWindowProc(). */
	while(TRUE) {
		MSG Message;
		GetMessageW(&Message, NULL, 0, 0);
		TranslateMessage(&Message);
		DispatchMessageW(&Message);
	}
}
