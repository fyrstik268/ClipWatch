/* ClipWatch - 'About' Dialog Box */
#include <ClipWatch.h>

static byte CW_Counter;

intptr CALLBACK CWAboutDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam) {
	switch(Message) {
	case WM_INITDIALOG:
		SendMessageW(Dialog, WM_SETICON, ICON_BIG, (LPARAM)CW.UI.Icon);
		SendMessageW(Dialog, WM_SETICON, ICON_SMALL, (LPARAM)CW.UI.Icon);
		return true;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		RECT Rect;
		POINT Cursor;
		GetWindowRect(GetDlgItem(Dialog, CW_IDC_ICON), &Rect);
		GetPhysicalCursorPos(&Cursor);
		if(PtInRect(&Rect, Cursor)) {
			if(CW_Counter >= 10) {
				HICON ErrorIcon = LoadIconW(NULL, IDI_ERROR);
				SendMessageW(Dialog, WM_SETICON, ICON_BIG, (LPARAM)ErrorIcon);
				SendMessageW(Dialog, WM_SETICON, ICON_SMALL, (LPARAM)ErrorIcon);
				SendDlgItemMessageW(Dialog, CW_IDC_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)ErrorIcon);

				wchar BackupText[ARRAYSIZE(CW.Config.Text)];
				wcscpy_s(BackupText, ARRAYSIZE(BackupText), CW.Config.Text);
				wcscpy_s(CW.Config.Text, ARRAYSIZE(CW.Config.Text), L"Please stop");
				SendMessageW(CW.Windows.Main, WM_CLIPBOARDUPDATE, 0, 0);
				CW.Config.Flags |= CW_CFG_RESET_TEXT;
			}
			else CW_Counter++;
		}
		return false;

	case WM_COMMAND:
		if(LOWORD(WParam) != IDOK) return true;

	case WM_CLOSE:
		CW_Counter = 0;
		DestroyWindow(Dialog);
		CW.Windows.About = NULL;
		return true;

	case WM_NOTIFY:
		if(((NMHDR*)LParam)->code == NM_CLICK && ((NMHDR*)LParam)->idFrom == CW_IDC_URL) ShellExecuteW(NULL, L"open", L"https://github.com/PersonMedBrukernavn/ClipWatch", NULL, NULL, SW_SHOWNORMAL);
		return true;

	default:
		return false;
	}
}