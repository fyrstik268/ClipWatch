/* ClipWatch - Configuration dialog box and registry code */
#include <ClipWatch.h>

/* WM_CTLCOLORSTATIC
wParam
	Handle to the device context for the static control window.
lParam
	Handle to the static control. */

/* ClipWatch Registry Keys (HKCU\Software\ClipWatch\)
- Text Colour (DWORD, COLORREF format)
- Background Colour (DWORD, COLORREF format)
- FadeIn duration (DWORD, milliseconds. Cast to WORD)
- Mid-Fade Delay (DWORD, milliseconds. Cast to WORD)
- FadeOut duration (DWORD, milliseconds. Cast to WORD)
- Flags (DWORD, see the CW_CFG_* enums)
- Font (Byte array of LOGFONTW struct, *only* 92 bytes.) */

HWND CW_PreviewTextControl;

#pragma warning(suppress: 4100)
intptr CALLBACK CWSettingsDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam) {
	switch(Message) {
	case WM_INITDIALOG:
		#pragma region DialogInit
		SendMessageW(Dialog, WM_SETICON, ICON_BIG, (LPARAM)CW.UI.Icon);
		SendMessageW(Dialog, WM_SETICON, ICON_SMALL, (LPARAM)CW.UI.Icon);
		CW_PreviewTextControl = GetDlgItem(Dialog, IDC_TEXT_PREVIEW);

		SendDlgItemMessageW(Dialog, IDC_EDIT_TEXT_COLOUR, EM_SETLIMITTEXT, 6, 0);
		SendDlgItemMessageW(Dialog, IDC_EDIT_BG_COLOUR, EM_SETLIMITTEXT, 6, 0);
		return true;
		#pragma endregion

	case WM_CTLCOLORSTATIC:
		#pragma region TextPreviewColouring
		if((HWND)LParam == CW_PreviewTextControl) {
			SetBkColor((HDC)WParam, CW.Config.BackgroundColour);
			SetTextColor((HDC)WParam, CW.Config.TextColour);
			return (intptr)CW.UI.BackgroundColourBrush;
		}
		return false;
		#pragma endregion

	case WM_COMMAND:
		switch(LOWORD(WParam)) {
		case IDC_CHOOSE_FONT:
			#pragma region ChooseFont
			CHOOSEFONTW FontInfo = {
				.lStructSize = sizeof(FontInfo),
				.hwndOwner = Dialog,
				.lpLogFont = &CW.Config.LogFont,
				.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS
			};
			if(ChooseFontW(&FontInfo)) {
				HFONT Font = CreateFontIndirectW(&CW.Config.LogFont);
				DeleteObject(CW.UI.Font);
				CW.UI.Font = Font;
			}
			return true;
			#pragma endregion

		case IDC_EDIT_TEXT_COLOUR:
			#pragma region EditTextColour
			if(HIWORD(WParam) != EN_UPDATE) return false;
			wchar TextColour[7];
			GetDlgItemTextW(Dialog, IDC_EDIT_TEXT_COLOUR, TextColour, 7);

			byte InvalidCharIndex = (byte)wcsspn(TextColour, L"0123456789ABCDEF");
			/* Shift chars backwards after the invalid char back one, and keep the caret position the same */
			if(InvalidCharIndex) {
				int CaretPos = SendDlgItemMessageW(Dialog, IDC_EDIT_TEXT_COLOUR, EM_GETSEL, 0, 0);
				TextColour[InvalidCharIndex] = TextColour[InvalidCharIndex - 1];
				TextColour[InvalidCharIndex - 1] = 0;
				SendDlgItemMessageW(Dialog, IDC_EDIT_TEXT_COLOUR, EM_SETSEL, CaretPos, CaretPos);
			}

			#pragma endregion

		default:
			return true;
		}

	default:
		return false;
	}
}

static inline void CWLoadConfigElement(HKEY const restrict Key, wchar* const restrict ValueName, dword* const restrict Value, const dword Default) {
	dword Size = sizeof(dword);
	if(RegGetValueW(Key, NULL, ValueName, RRF_RT_DWORD, NULL, &Value, &Size)) *Value = Default;
	return;
}

void CWLoadConfig(void) {
	HKEY restrict Key;
	if(RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ClipWatch", 0, KEY_QUERY_VALUE, &Key) != ERROR_SUCCESS) {
		CW.Config.FadeInDuration = 1250;
		CW.Config.MidFadeDelay = 500;
		CW.Config.FadeOutDuration = 1250;
		CW.Config.TextColour = RGB(0xFF, 0xFF, 0xFF);
		//CW.Config.BackgroundColour = RGB(0x33, 0x33, 0x33);
		CW.Config.BackgroundColour = RGB(0x33, 0x33, 0xCC);
		CW.Config.Flags = CW_CFG_POS_TO_NEAREST;

		NONCLIENTMETRICSW Metrics;
		Metrics.cbSize = sizeof(Metrics);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(Metrics), &Metrics, 0);
		CW.Config.LogFont = Metrics.lfMessageFont;

		CW.UI.Font = CreateFontIndirectW(&CW.Config.LogFont);
		CW.UI.BackgroundColourBrush = CreateSolidBrush(CW.Config.BackgroundColour);
		return;
	}

	dword CastingBuffer;
	CWLoadConfigElement(Key, L"Fade-In Duration", &CastingBuffer, 1250);
	CW.Config.FadeInDuration = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;
	CWLoadConfigElement(Key, L"Mid-Fade Delay", &CastingBuffer, 500);
	CW.Config.MidFadeDelay = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;
	CWLoadConfigElement(Key, L"Fade-Out Duration", &CastingBuffer, 1250);
	CW.Config.FadeOutDuration = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;

	CWLoadConfigElement(Key, L"Text Colour", &CW.Config.TextColour, RGB(0xFF, 0xFF, 0xFF));
	CWLoadConfigElement(Key, L"Background Colour", &CW.Config.BackgroundColour, RGB(0x33, 0x33, 0x33));

	CWLoadConfigElement(Key, L"Flags", &CastingBuffer, CW_CFG_POS_TO_NEAREST);
	CW.Config.Flags = (byte)CastingBuffer;

	dword Size = sizeof(CW.Config.LogFont);
	if(RegGetValueW(Key, NULL, L"Font", RRF_RT_REG_BINARY, NULL, &CW.Config.LogFont, &Size)) {
		NONCLIENTMETRICSW Metrics;
		Metrics.cbSize = sizeof(Metrics);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(Metrics), &Metrics, 0);
		CW.Config.LogFont = Metrics.lfMessageFont;
	}

	RegCloseKey(Key);
	CW.UI.Font = CreateFontIndirectW(&CW.Config.LogFont);
	CW.UI.BackgroundColourBrush = CreateSolidBrush(CW.Config.BackgroundColour);
	return;
}

void CWSaveConfig(void) {
	return;
}
