
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define PAD (8)
#define PAD2 (PAD+PAD)
#define SIDEBARSIZE (150)
#define SIDEBARSIZEEX (SIDEBARSIZE + PAD2)

#define IDC_TEXT 101
#define IDC_COPY 102
#define IDC_PAST 103
#define IDC_LABL 104

HWND hEdit, hLabel;
WNDPROC actualEditWndProc;

/* purely so all text gets selected on ctrl+a */
LRESULT CALLBACK EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CHAR && wParam == 1) {
		SendMessage(hwnd, EM_SETSEL, 0, -1);
		return 1;
	}
	return CallWindowProc(actualEditWndProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		HFONT hfDefault;
		NONCLIENTMETRICS ncm;
		HMODULE modulehandle;
		HWND control;
		int y = PAD, h;

		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
		hfDefault = CreateFontIndirect(&(ncm.lfMessageFont));

		modulehandle = GetModuleHandleA(NULL);
		hLabel = CreateWindowExA(0, "Static", "-", WS_CHILD | WS_VISIBLE | SS_LEFT,
			PAD, y, SIDEBARSIZE, h = 18, hwnd, (HMENU) IDC_LABL, modulehandle, NULL);
		SendMessage(hLabel, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
		control = CreateWindowExA(0, "Button", "Paste from clipboard", WS_CHILD | WS_VISIBLE,
			PAD, y += h + PAD, SIDEBARSIZE, h = 25, hwnd, (HMENU) IDC_PAST, modulehandle, NULL);
		SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
		control = CreateWindowExA(0, "Button", "Copy to clipboard", WS_CHILD | WS_VISIBLE,
			8, y += h + PAD, SIDEBARSIZE, h = 25, hwnd, (HMENU) IDC_COPY, modulehandle, NULL);
		SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
		hEdit = CreateWindowExA(
			WS_EX_CLIENTEDGE, "Edit", "",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
			ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
			SIDEBARSIZEEX, 8, 100, 100, hwnd, (HMENU) IDC_TEXT, modulehandle, NULL);
		SendMessage(hEdit, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
		actualEditWndProc = (WNDPROC) SetWindowLong(hEdit, GWL_WNDPROC, (LONG) &EditWndProc);
		/* send a msg that paste from clipboard button was clicked */
		PostMessageA(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PAST, 0), 0);
		break;
	}
	case WM_SIZE:
	{
		RECT rcClient;

		GetClientRect(hwnd, &rcClient);
		/*hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);*/
		SetWindowPos(hEdit, NULL,
			0, 0, rcClient.right - SIDEBARSIZEEX - 8, rcClient.bottom - 16,
			SWP_NOZORDER | SWP_NOMOVE);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TEXT:
		{
			int linecount;
			char buf[40];
			if (HIWORD(wParam) == EN_CHANGE) {
				linecount = SendMessageA(hEdit, EM_GETLINECOUNT, 0, 0);
				sprintf_s(buf, sizeof(buf), "%d object(s)", linecount);
				SetWindowTextA(hLabel, buf);
			}
			break;
		}
		case IDC_PAST:
		{
			HANDLE hData;
			char *txt;
			if (OpenClipboard(NULL) &&
				(hData = GetClipboardData(CF_TEXT)) &&
				(txt = GlobalLock(hData)))
			{
				GlobalUnlock(txt);
				SetWindowTextA(hEdit, txt);
				/* send a msg that the text was changed */
				PostMessageA(hwnd, WM_COMMAND, MAKEWPARAM(IDC_TEXT, EN_CHANGE), 0);
				CloseClipboard();
			}
			break;
		}
		case IDC_COPY:
		{
			HGLOBAL hMem;
			char *txt;
			int len;
			len = GetWindowTextLengthA(hEdit) + 1;
			if ((hMem = GlobalAlloc(GMEM_MOVEABLE, len)) &&
				(txt = GlobalLock(hMem)))
			{
				GetWindowTextA(hEdit, txt, len);
				GlobalUnlock(hMem);
				if (OpenClipboard(NULL)) {
					EmptyClipboard();
					SetClipboardData(CF_TEXT, hMem);
					CloseClipboard();
				}
			}
			break;
		}
		}
		break;
	case WM_CLOSE: DestroyWindow(hwnd); break;
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); /*large icon (alt tab)*/
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "mcetoolclass";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION); /*small icon (taskbar)*/

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "failed window reg", "err", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		"title",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 448,
		NULL, NULL, hInstance, NULL
	);
	if (hwnd == NULL) {
		MessageBox(NULL, "failed window creation", "err", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}