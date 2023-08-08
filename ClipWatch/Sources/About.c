/* ClipWatch - 'About' Dialog Box */
#include <ClipWatch.h>

static byte CW_Counter;
static const wchar* CW_LicenseText = L"MIT License\r\n\
\r\n\
Copyright(c) 2023 Tom Arnesen\r\n\
\r\n\
Permission is hereby granted, free of charge, to any person obtaining a copy \
of this software and associated documentation files(the \"Software\"), to deal \
in the Software without restriction, including without limitation the rights \
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell \
copies of the Software, and to permit persons to whom the Software is \
furnished to do so, subject to the following conditions:\r\n\
\r\n\
The above copyright notice and this permission notice shall be included in all \
copies or substantial portions of the Software.\r\n\
\r\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE \
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER \
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, \
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE \
SOFTWARE.";

#pragma warning(suppress: 4100)
intptr CALLBACK CWLicenseDialog(HWND Dialog, uint Message, WPARAM WParam, LPARAM LParam) {
	switch(Message) {
	case WM_INITDIALOG:
		SetDlgItemTextW(Dialog, CW_IDC_LICENSE, CW_LicenseText);
		SendMessageW(Dialog, WM_SETICON, ICON_BIG, (LPARAM)CW.UI.Icon);
		SendMessageW(Dialog, WM_SETICON, ICON_SMALL, (LPARAM)CW.UI.Icon);
		return true;

	case WM_COMMAND:
		if(LOWORD(WParam) != IDCLOSE) return true;

	case WM_CLOSE:
		EndDialog(Dialog, 0);
		return true;

	default:
		return false;
	}
}

intptr CALLBACK CWAboutDialog(HWND Dialog, uint Message, WPARAM WParam, LPARAM LParam) {
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

				wcscpy_s(CW.Config.Text, ARRAYSIZE(CW.Config.Text), L"Please stop");
				SendMessageW(CW.Windows.Main, WM_CLIPBOARDUPDATE, 0, 0);
				CW.Config.Flags |= CW_CFG_RESET_TEXT;
			}
			else CW_Counter++;
		}
		return false;

	case WM_COMMAND:
		switch(LOWORD(WParam)) {
		case CW_IDC_LICENSE:
			DialogBoxParamW(CW.ProcessModule, MAKEINTRESOURCEW(CW_IDD_LICENSE), Dialog, CWLicenseDialog, 0);

		default:
			return true;

		case IDOK:;
		}

	case WM_CLOSE:
		CW_Counter = 0;
		DestroyWindow(Dialog);
		CW.Windows.About = NULL;
		return true;

	case WM_NOTIFY:
		if(((NMHDR*)LParam)->code == NM_CLICK && ((NMHDR*)LParam)->idFrom == CW_IDC_URL) ShellExecuteW(NULL, L"open", L"https://github.com/fyrstik268/ClipWatch", NULL, NULL, SW_SHOWNORMAL);
		return true;

	default:
		return false;
	}
}