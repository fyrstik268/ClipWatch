#include <ClipWatch.h>

static LRESULT CWWindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
	_Bool Painting = FALSE;
	switch(Message) {
	case WM_CLIPBOARDUPDATE:
		SetEvent(CW.AnimatorEvent);

	case WM_CLOSE:
		return 0;

	case WM_SHLICON:
		switch(LOWORD(LParam)) {
		default:
			return 0;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			break;
		}

		POINT CursorPos;
		GetPhysicalCursorPos(&CursorPos);
		SetForegroundWindow(CW.WindowHandle);
		const BOOL Return = TrackPopupMenu(CW.Menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION,
										   CursorPos.x, CursorPos.y, 0, Window, NULL);

		if(Return) CWQuit();
		return 0;

	case WM_PAINT:
		Painting = TRUE;
		PAINTSTRUCT PaintStruct;
		WParam = (WPARAM)BeginPaint(Window, &PaintStruct);

	case WM_PRINTCLIENT:
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);

		HDC const DC = (HDC)WParam;
		SetBkColor(DC, RGB(0x33, 0x33, 0x33));
		SetTextColor(DC, RGB(0xFF, 0xFF, 0xFF));
		SelectObject(DC, CW.Font);
		DrawTextW(DC, L"Clipboard Updated!", 18, &ClientRect, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

		if(Painting) EndPaint(Window, &PaintStruct);
		return 0;

	default:
		return DefWindowProc(Window, Message, WParam, LParam);
	}
}

__forceinline void CWCreateWindowClass(void) {
	const WNDCLASSW WindowClass = {
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = CWWindowProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = CW.ProcessModule,
		.hIcon = NULL,
		.hCursor = LoadCursorW(NULL, IDC_ARROW),
		.hbrBackground = CW.BackgroundColourBrush,
		.lpszMenuName = NULL,
		.lpszClassName = L"ClipWatchWndClass"
	};
	CW.WindowClass = RegisterClassW(&WindowClass);
	return;
}

__forceinline void CWCreateWindow(void) {
	HMONITOR Monitor = MonitorFromPoint((POINT){0}, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO MonitorInfo;
	MonitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfoW(Monitor, &MonitorInfo);

	const char Width = 113, Height = 28;
	CW.WindowHandle = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_NOACTIVATE | WS_EX_TOPMOST, MAKEINTATOM(CW.WindowClass), NULL, WS_POPUP,
									  (MonitorInfo.rcWork.right - MonitorInfo.rcWork.left) / 2 - (Width / 2),
									  (MonitorInfo.rcWork.bottom - MonitorInfo.rcWork.top) - 50 - (Height / 2),
									  Width, Height, NULL, NULL, CW.ProcessModule, NULL);
	return;
}

#pragma warning(suppress: 4100)
DWORD WINAPI CWWindowAnimator(void* Unused) {
	while(TRUE) {
		WaitForSingleObject(CW.AnimatorEvent, INFINITE);
		if(!CW.Running) ExitThread(0);

		AnimateWindow(CW.WindowHandle, 1250, AW_BLEND);
		Sleep(500);
		AnimateWindow(CW.WindowHandle, 1250, AW_BLEND | AW_HIDE);
		ResetEvent(CW.AnimatorEvent);
	}
}
