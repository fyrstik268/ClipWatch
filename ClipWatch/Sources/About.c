/* ClipWatch - 'About' Dialog Box */
#include <ClipWatch.h>

intptr CALLBACK CWAboutDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam) {
	switch(Message) {
	case WM_INITDIALOG:
		SendMessageW(Dialog, WM_SETICON, ICON_BIG, (LPARAM)CW.UI.Icon);
		SendMessageW(Dialog, WM_SETICON, ICON_SMALL, (LPARAM)CW.UI.Icon);
		return TRUE;

	case WM_COMMAND:
		if(WParam != IDOK) return TRUE;

	case WM_CLOSE:
		EndDialog(Dialog, 0);
		CW.Windows.About = NULL;
		return TRUE;

	case WM_NOTIFY:
		if(((NMHDR*)LParam)->code == NM_CLICK && ((NMHDR*)LParam)->idFrom == IDC_URL) ShellExecuteW(NULL, L"open", L"https://github.com/PersonMedBrukernavn/ClipWatch", NULL, NULL, SW_SHOWNORMAL);
		return TRUE;

	default:
		return FALSE;
	}
}
