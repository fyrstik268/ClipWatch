/* ClipWatch - Configuration dialog box and registry code */
#include <ClipWatch.h>

static COLORREF CWGetColourFromEditBox(HWND Dialog, UINT EditBoxID) {
	wchar Buffer[7];
	GetDlgItemTextW(Dialog, EditBoxID, Buffer, 7);
	if(wcslen(Buffer) != 6) return MAXDWORD;

	wchar* Ptr = Buffer + 4;
	byte B = (byte)wcstol(Ptr, NULL, 16);
	*Ptr = L'\0';
	Ptr -= 2;
	byte G = (byte)wcstol(Ptr, NULL, 16);
	*Ptr = L'\0';
	Ptr -= 2;
	byte R = (byte)wcstol(Ptr, NULL, 16);

	return RGB(R, G, B);
}

static void CWSetColourToEditBox(HWND Dialog, UINT EditBoxID, COLORREF Colour) {
	wchar Buffer[7];
	wsprintfW(Buffer, L"%.2X%.2X%.2X", GetRValue(Colour), GetGValue(Colour), GetBValue(Colour));
	SetDlgItemTextW(Dialog, EditBoxID, Buffer);
	return;
}

static HWND CW_PreviewTextControl;
static COLORREF CW_PreviewTextColour;
static COLORREF CW_PreviewBackgroundColour;
static HBRUSH CW_PreviewBackgroundBrush;

static COLORREF CW_DefaultColours[16];

intptr CALLBACK CWSettingsDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam) {
	switch(Message) {
	case WM_INITDIALOG:
		#pragma region DialogInit
		SendMessageW(Dialog, WM_SETICON, ICON_BIG, (LPARAM)CW.UI.Icon);
		SendMessageW(Dialog, WM_SETICON, ICON_SMALL, (LPARAM)CW.UI.Icon);
		SendDlgItemMessageW(Dialog, CW_IDC_EDIT_TEXT_COLOUR, EM_SETLIMITTEXT, 6, 0);
		SendDlgItemMessageW(Dialog, CW_IDC_EDIT_BG_COLOUR, EM_SETLIMITTEXT, 6, 0);
		SendDlgItemMessageW(Dialog, CW_IDC_EDIT_FADEIN, EM_SETLIMITTEXT, 5, 0);
		SendDlgItemMessageW(Dialog, CW_IDC_EDIT_FADEOUT, EM_SETLIMITTEXT, 5, 0);
		SendDlgItemMessageW(Dialog, CW_IDC_EDIT_FADEDELAY, EM_SETLIMITTEXT, 5, 0);

		CW_PreviewTextControl = GetDlgItem(Dialog, CW_IDC_TEXT_PREVIEW);
		CWSetColourToEditBox(Dialog, CW_IDC_EDIT_TEXT_COLOUR, CW.Config.TextColour);
		CWSetColourToEditBox(Dialog, CW_IDC_EDIT_BG_COLOUR, CW.Config.BackgroundColour);

		SetDlgItemInt(Dialog, CW_IDC_EDIT_FADEIN, CW.Config.FadeInDuration, false);
		SetDlgItemInt(Dialog, CW_IDC_EDIT_FADEDELAY, CW.Config.MidFadeDelay, false);
		SetDlgItemInt(Dialog, CW_IDC_EDIT_FADEOUT, CW.Config.FadeOutDuration, false);
		if(!(CW.Config.Flags & CW_CFG_ADVANCED_TIMING)) {
			CW.Config.Flags |= CW_CFG_ADVANCED_TIMING;
			SendMessageW(Dialog, WM_COMMAND, MAKEWPARAM(CW_IDC_TOGGLE_VIEWS, 0), 0);
		}

		int RadioButton = CW_IDC_POSITION_CURSOR;
		if(CW.Config.Flags & CW_CFG_POS_TO_NEAREST) RadioButton++;
		else if(CW.Config.Flags & CW_CFG_POS_TO_PRIMARY) RadioButton++;
		CheckRadioButton(Dialog, CW_IDC_POSITION_CURSOR, CW_IDC_POSITION_PRIMARY_MONITOR, CW_IDC_POSITION_CURSOR);

		return true;
		#pragma endregion

	case WM_UPDATE_PREVIEW_COLOURS:
		#pragma region UpdatePreviewColours
		CW_PreviewTextColour = CWGetColourFromEditBox(Dialog, CW_IDC_EDIT_TEXT_COLOUR);
		CW_PreviewBackgroundColour = CWGetColourFromEditBox(Dialog, CW_IDC_EDIT_BG_COLOUR);
		DeleteObject(CW_PreviewBackgroundBrush);
		CW_PreviewBackgroundBrush = CreateSolidBrush(CW_PreviewBackgroundColour);
		RedrawWindow(Dialog, NULL, NULL, RDW_INVALIDATE);
		return true;
		#pragma endregion

	case WM_CTLCOLORSTATIC:
		#pragma region ColourPreview
		if((HWND)LParam == CW_PreviewTextControl) {
			SetBkColor((HDC)WParam, CW_PreviewBackgroundColour);
			SetTextColor((HDC)WParam, CW_PreviewTextColour);
			return (intptr)CW_PreviewBackgroundBrush;
		}
		return false;
		#pragma endregion

	case WM_COMMAND:
		switch(LOWORD(WParam)) {
		case CW_IDC_CHOOSE_FONT:
			#pragma region ChooseFont
			/* ChooseFontW() is "unsupported" in modern windows. Not that it matters, even in Win11 the fallback notepad.exe uses it! */
			CHOOSEFONTW FontInfo;
			FontInfo.lStructSize = sizeof(FontInfo);
			FontInfo.hwndOwner = Dialog;
			FontInfo.lpLogFont = &CW.Config.LogicalFontData;
			FontInfo.Flags = CF_INITTOLOGFONTSTRUCT;
			if(ChooseFontW(&FontInfo)) {
				if(CW.UI.UpdateFont) DeleteObject(CW.UI.UpdateFont);
				CW.UI.UpdateFont = CreateFontIndirectW(&CW.Config.LogicalFontData);
			}
			return true;
			#pragma endregion

		case CW_IDC_EDIT_TEXT_COLOUR:
		case CW_IDC_EDIT_BG_COLOUR:
			#pragma region EditTextColours
			if(HIWORD(WParam) != EN_UPDATE) return false;
			wchar TextColour[7];
			GetDlgItemTextW(Dialog, LOWORD(WParam), TextColour, 7);

			wchar* InvalidChar = _wcsspnp(TextColour, L"0123456789ABCDEF");
			if(InvalidChar) {
				byte CaretPos = (byte)SendDlgItemMessageW(Dialog, LOWORD(WParam), EM_GETSEL, 0, 0);
				while(*InvalidChar) {
					*InvalidChar = *(InvalidChar + 1);
					InvalidChar++;
				}
				SetDlgItemTextW(Dialog, LOWORD(WParam), TextColour);
				SendDlgItemMessageW(Dialog, LOWORD(WParam), EM_SETSEL, CaretPos - 1, CaretPos - 1);
			}

			if(wcslen(TextColour) == 6) {
				COLORREF NewColour = CWGetColourFromEditBox(Dialog, LOWORD(WParam));
				if(LOWORD(WParam) == CW_IDC_EDIT_TEXT_COLOUR) CW_PreviewTextColour = NewColour;
				else {
					CW_PreviewBackgroundColour = NewColour;
					DeleteObject(CW_PreviewBackgroundBrush);
					CW_PreviewBackgroundBrush = CreateSolidBrush(CW_PreviewBackgroundColour);
				}
				SendMessageW(Dialog, WM_UPDATE_PREVIEW_COLOURS, 0, 0);
			}

			return true;
			#pragma endregion

		case CW_IDC_PICK_TEXT_COLOUR:
		case CW_IDC_PICK_BG_COLOUR:
			#pragma region PickColours
			const word EditBoxID = LOWORD(WParam) - 2; /* As long as the resource IDs remain unchanged this will work. */
			CHOOSECOLORW ColourInfo;
			ColourInfo.lStructSize = sizeof(ColourInfo);
			ColourInfo.hwndOwner = Dialog;
			ColourInfo.rgbResult = CWGetColourFromEditBox(Dialog, EditBoxID);
			ColourInfo.lpCustColors = CW_DefaultColours;
			ColourInfo.Flags = CC_FULLOPEN | CC_RGBINIT;
			if(ChooseColorW(&ColourInfo)) CWSetColourToEditBox(Dialog, EditBoxID, ColourInfo.rgbResult);
			return true;
			#pragma endregion

		case CW_IDC_EDIT_FADEIN:
		case CW_IDC_EDIT_FADEOUT:
		case CW_IDC_EDIT_FADEDELAY:
			#pragma region FadeEditsLimitRange
			if(HIWORD(WParam) != EN_UPDATE) return false;
			dword Value = GetDlgItemInt(Dialog, LOWORD(WParam), NULL, false);
			if(Value > MAXWORD) SetDlgItemInt(Dialog, LOWORD(WParam), MAXWORD, false);
			else if(Value == 0) {
				wchar LengthBuffer[2];
				if(GetDlgItemTextW(Dialog, LOWORD(WParam), LengthBuffer, 2)) {
					SetDlgItemInt(Dialog, LOWORD(WParam), 1, false);
					SendDlgItemMessageW(Dialog, LOWORD(WParam), EM_SETSEL, 1, 1);
				}
			}
			return true;
			#pragma endregion

		case CW_IDC_TOGGLE_VIEWS:
			#pragma region SwitchTimingView
			if(CW.Config.Flags & CW_CFG_ADVANCED_TIMING) {
				CW.Config.Flags &= ~CW_CFG_ADVANCED_TIMING;
				SetDlgItemTextW(Dialog, CW_IDC_FADEIN_DURATION, L"Popup Duration:");
				SetDlgItemInt(Dialog, CW_IDC_EDIT_FADEIN, GetDlgItemInt(Dialog, CW_IDC_EDIT_FADEIN, NULL, false) + GetDlgItemInt(Dialog, CW_IDC_EDIT_FADEOUT, NULL, false) + GetDlgItemInt(Dialog, CW_IDC_EDIT_FADEDELAY, NULL, false), false);
				ShowWindow(GetDlgItem(Dialog, CW_IDC_FADEOUT_DURATION), SW_HIDE);
				ShowWindow(GetDlgItem(Dialog, CW_IDC_EDIT_FADEOUT), SW_HIDE);
				ShowWindow(GetDlgItem(Dialog, CW_IDC_FADEDELAY_DURATION), SW_HIDE);
				ShowWindow(GetDlgItem(Dialog, CW_IDC_EDIT_FADEDELAY), SW_HIDE);
				SetDlgItemTextW(Dialog, CW_IDC_TOGGLE_VIEWS, L"Advanced");
			}
			else {
				CW.Config.Flags |= CW_CFG_ADVANCED_TIMING;
				dword Duration = GetDlgItemInt(Dialog, CW_IDC_EDIT_FADEIN, NULL, false);
				SetDlgItemTextW(Dialog, CW_IDC_FADEIN_DURATION, L"Fade-in Duration:");
				ShowWindow(GetDlgItem(Dialog, CW_IDC_FADEOUT_DURATION), SW_SHOW);
				ShowWindow(GetDlgItem(Dialog, CW_IDC_EDIT_FADEOUT), SW_SHOW);
				ShowWindow(GetDlgItem(Dialog, CW_IDC_FADEDELAY_DURATION), SW_SHOW);
				ShowWindow(GetDlgItem(Dialog, CW_IDC_EDIT_FADEDELAY), SW_SHOW);
				SetDlgItemTextW(Dialog, CW_IDC_TOGGLE_VIEWS, L"Simple");

				SetDlgItemInt(Dialog, CW_IDC_EDIT_FADEDELAY, Duration / 6, false);
				Duration -= Duration / 6;
				SetDlgItemInt(Dialog, CW_IDC_EDIT_FADEIN, Duration /= 2, false);
				SetDlgItemInt(Dialog, CW_IDC_EDIT_FADEOUT, Duration, false);
			}
			return true;
			#pragma endregion

		case CW_IDC_TRIGGER_PREVIEW:
			#pragma region TriggerPopup
			SendMessageW(Dialog, WM_APPLY_CONFIG, 0, 0);
			SendMessageW(CW.Windows.Main, WM_CLIPBOARDUPDATE, 0, 0);
			return true;
			#pragma endregion

		case CW_IDC_CLEAR_AND_EXIT:
			#pragma region ClearConfigAndExit
			RegDeleteKeyExW(HKEY_CURRENT_USER, L"Software\\ClipWatch", 0, 0);
			PostQuitMessage(0);
			DeleteObject(CW_PreviewBackgroundBrush);
			DestroyWindow(Dialog);
			CW.Windows.Settings = NULL;
			return true;
			#pragma endregion

		default:
			return true;

		case IDCLOSE:;
		}

	case WM_CLOSE:
		#pragma region CloseDialog
		SendMessageW(Dialog, WM_APPLY_CONFIG, 0, 0);
		CWSaveConfig();
		DeleteObject(CW_PreviewBackgroundBrush);
		DestroyWindow(Dialog);
		CW.Windows.Settings = NULL;
		return true;
		#pragma endregion

	case WM_APPLY_CONFIG:
		#pragma region ApplyConfig
		CW.Config.TextColour = CW_PreviewTextColour;
		CW.Config.BackgroundColour = CW_PreviewBackgroundColour;
		if(CW.UI.UpdateBGBrush) DeleteObject(CW.UI.UpdateBGBrush);
		CW.UI.UpdateBGBrush = CreateSolidBrush(CW_PreviewBackgroundColour);

		if(CW.Config.Flags & CW_CFG_ADVANCED_TIMING) {
			CW.Config.FadeInDuration = (word)GetDlgItemInt(Dialog, CW_IDC_EDIT_FADEIN, NULL, false);
			CW.Config.MidFadeDelay = (word)GetDlgItemInt(Dialog, CW_IDC_EDIT_FADEDELAY, NULL, false);
			CW.Config.FadeOutDuration = (word)GetDlgItemInt(Dialog, CW_IDC_EDIT_FADEOUT, NULL, false);
		}
		else {
			dword Duration = (dword)GetDlgItemInt(Dialog, CW_IDC_EDIT_FADEIN, NULL, false);
			CW.Config.MidFadeDelay = (word)Duration / 6;
			Duration -= Duration / 6;
			CW.Config.FadeInDuration = (word)(Duration /= 2);
			CW.Config.FadeOutDuration = (word)Duration;
		}

		CW.Config.Flags &= CW_CFG_ADVANCED_TIMING;
		CW.Config.Flags |= IsDlgButtonChecked(Dialog, CW_IDC_POSITION_CURSOR) == BST_CHECKED ? CW_CFG_POS_TO_CURSOR : 0;
		CW.Config.Flags |= IsDlgButtonChecked(Dialog, CW_IDC_POSITION_ACTIVE_MONITOR) == BST_CHECKED ? CW_CFG_POS_TO_NEAREST : 0;
		CW.Config.Flags |= IsDlgButtonChecked(Dialog, CW_IDC_POSITION_PRIMARY_MONITOR) == BST_CHECKED ? CW_CFG_POS_TO_PRIMARY : 0;

		return true;
		#pragma endregion

	default:
		return false;
	}
}