#include <ClipWatch.h>

static LRESULT CALLBACK CWWindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
	bool Painting = false;
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
		SetForegroundWindow(CW.Windows.Main);
		switch(TrackPopupMenu(CW.Menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION, CursorPos.x, CursorPos.y, 0, Window, NULL)) {
		case CW_TIM_CLEAR:
			OpenClipboard(Window);
			EmptyClipboard();
			CloseClipboard();
			return 0;

		case CW_TIM_SETTINGS:
			if(!CW.Windows.Settings) CW.Windows.Settings= CreateDialogParamW(CW.ProcessModule, MAKEINTRESOURCEW(IDD_SETTINGS), NULL, CWSettingsDialog, 0);
			else SetForegroundWindow(CW.Windows.Settings);
			return 0;

		case CW_TIM_ABOUT:
			if(!CW.Windows.About) CW.Windows.About= CreateDialogParamW(CW.ProcessModule, MAKEINTRESOURCEW(IDD_ABOUT), NULL, CWAboutDialog, 0);
			else SetForegroundWindow(CW.Windows.About);
			return 0;

		case CW_TIM_EXIT:
			CWQuit();

		default:
			return 0;
		}

	case WM_PAINT:
		Painting = true;
		PAINTSTRUCT PaintStruct;
		WParam = (WPARAM)BeginPaint(Window, &PaintStruct);

	case WM_PRINTCLIENT:
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);

		/* Set window pos, get text size via DrawTextW() with DT_CALCRECT. */

		HDC const DC = (HDC)WParam;
		SetBkColor(DC, RGB(0x33, 0x33, 0x33));
		SetTextColor(DC, RGB(0xFF, 0xFF, 0xFF));
		SelectObject(DC, CW.UI.Font);
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
		.hbrBackground = CW.UI.BackgroundColourBrush,
		.lpszMenuName = NULL,
		.lpszClassName = L"ClipWatchWndClass"
	};
	CW.Windows.Class = RegisterClassW(&WindowClass);
	return;
}

#pragma warning(suppress: 4100)
dword WINAPI CWWindowAnimator(void* Unused) {
	while(true) {
		WaitForSingleObject(CW.AnimatorEvent, INFINITE);
		if(!CW.Running) ExitThread(0);

		AnimateWindow(CW.Windows.Main, CW.Config.FadeInDuration, AW_BLEND);
		Sleep(CW.Config.MidFadeDelay);
		AnimateWindow(CW.Windows.Main, CW.Config.FadeOutDuration, AW_BLEND | AW_HIDE);
		ResetEvent(CW.AnimatorEvent);
	}
}
