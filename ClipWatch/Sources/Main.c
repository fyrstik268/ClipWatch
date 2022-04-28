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
	CW.AnimatorEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	CW.AnimatorThread = CreateThread(NULL, 1, CWWindowAnimator, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);

	CW.NotifyIcon.cbSize = sizeof(CW.NotifyIcon);
	CW.NotifyIcon.hWnd = CW.WindowHandle;
	CW.NotifyIcon.uID = 1;
	CW.NotifyIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	CW.NotifyIcon.uCallbackMessage = WM_SHLICON;
	CW.NotifyIcon.hIcon = LoadIconW(CW.ProcessModule, MAKEINTRESOURCEW(IDI_ICON));
	lstrcpyW(CW.NotifyIcon.szTip, L"ClipWatch");
	Shell_NotifyIconW(NIM_ADD, &CW.NotifyIcon);

	CW.Menu = CreatePopupMenu();
	InsertMenuW(CW.Menu, 0, MF_BYPOSITION | MF_STRING, 1, L"Exit ClipWatch");

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

	while(TRUE) {
		MSG Message;
		while(GetMessageW(&Message, NULL, 0, 0)) {
			TranslateMessage(&Message);
			DispatchMessageW(&Message);
		}
	}
}
