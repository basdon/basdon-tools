
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>

#define ERRMSG(parent,text) MessageBoxA(parent,text,"oops",MB_ICONWARNING|MB_OK)
#define spr(...) sprintf_s(message,sizeof(message),__VA_ARGS__)

#define PAD (8)
#define PAD2 (PAD+PAD)
#define SIDEBARSIZE (150)
#define SIDEBARSIZEEX (SIDEBARSIZE + PAD2)
#define BULKWINWIDTH 400

#define WIN_MAIN 50
#define WIN_BULK 51

#define IDC_TEXT 101
#define IDC_COPY 102
#define IDC_PAST 103
#define IDC_LABL 104
#define IDC_CHAI 105
#define IDC_CHAX 106
#define IDC_CHAY 107
#define IDC_CHAZ 108

struct OBJECT
{
	int id;
	float x, y, z;
	float rx, ry, rz;
};

char message[200];

int window_being_created;
HINSTANCE hInstance4windows;
HFONT hfDefault;
HMODULE modulehandle;
WNDCLASSEX wc;
HWND hMain, hBulkedit = NULL, hEdit, hLabel;
WNDPROC actualEditWndProc;

void showBulkEdit()
{
	MSG msg;
	RECT mainpos;
	int selstart, selend, linestart, lineend;
	int numobjects, i;
	int linelen;
	char *linecontent;
	int parseidx;
	char parsingvalue[100], *c, *p;
	float values[7], x;
	int valueidx, invalue;
	const int numvalues = sizeof(values)/sizeof(values[0]);
	struct OBJECT *objects, *o;

	SendMessage(hEdit, EM_GETSEL, (WPARAM) &selstart, (LPARAM) &selend);
	if (selstart <= 0 && selend <= 0) {
		ERRMSG(hMain, "No selection, select some lines and try again.");
		return;
	}
	linestart = SendMessage(hEdit, EM_LINEFROMCHAR, (WPARAM) selstart, 0);
	lineend = SendMessage(hEdit, EM_LINEFROMCHAR, (WPARAM) selend, 0);
	numobjects = i = lineend - linestart + 1;
	o = objects = (struct OBJECT*) malloc(numobjects * sizeof(struct OBJECT));
	if (!objects) {
		ERRMSG(hMain, "Memory allocation failed");
		return;
	}
	while (i-- > 0) {
		/*meanwhile set selstart to first index of first selected line,
		  selend to last index of last selected line, hence selstart and
		  selend a few lines below*/
		selstart = SendMessage(hEdit, EM_LINEINDEX, linestart + i, 0);
		linelen = SendMessage(hEdit, EM_LINELENGTH, (WPARAM) selstart, 0);
		if (i + 1 == numobjects) {
			selend = selstart + linelen;
		}
		/*linelen + 2 because first DWORD must be set to the size of the buffer
		  that we'll allocate next (also zero terminator)*/
		linelen += 2;
		linecontent = (char*) malloc(linelen * sizeof(TCHAR));
		if (!linecontent) {
			free(objects);
			ERRMSG(hMain, "Memory allocation failed");
			return;
		}
		/*set first WORD in buffer to the buffer size*/
		*((WORD*) linecontent) = (WORD) linelen;
		SendMessage(hEdit, EM_GETLINE, linestart + i, (LPARAM) linecontent);
		linecontent[linelen - 2] = 0; /*EM_GETLINE does not add zero term*/
		parseidx = valueidx = -1;
		invalue = 0;
		while (*(c = linecontent + ++parseidx) != 0) {
			if (*c == '.' || *c == '-' || ('0' <= *c && *c <= '9')) {
				if (!invalue) {
					invalue = 1;
					p = parsingvalue;
				}
				*(p++) = *c;
			} else {
				if (invalue) {
					invalue = 0;
					*p = 0;
					values[++valueidx] = (float) atof(parsingvalue);
					if (valueidx >= numvalues) {
						break;
					}
				}
			}

		}
		free(linecontent);
		if (valueidx != numvalues - 1) {
			spr("invalid line, expected %d values but got %d", numvalues, valueidx);
			free(objects);
			return;
		}
		o->id = (int) values[0];
		o->x = values[1];
		o->y = values[2];
		o->z = values[3];
		o->rx = values[4];
		o->ry = values[5];
		o->rz = values[6];
		o++;
	}

	GetWindowRect(hMain, &mainpos);
	window_being_created = WIN_BULK;
	hBulkedit = CreateWindowEx(
		0,
		wc.lpszClassName,
		"Bulk edit",
		WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX),
		(mainpos.left + mainpos.right - BULKWINWIDTH) / 2,
		(mainpos.top + mainpos.bottom - BULKWINWIDTH) / 2,
		BULKWINWIDTH, BULKWINWIDTH,
		hMain, NULL, hInstance4windows, NULL
	);
	if (hBulkedit == NULL) {
		ERRMSG(NULL, "Window creation failed.");
		free(objects);
		return;
	}

	ShowWindow(hBulkedit, SW_SHOWNORMAL);
	UpdateWindow(hBulkedit);
	EnableWindow(hMain, FALSE);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (hBulkedit == NULL) {
			break; /*go back to other message loop*/
		}
	}

	p = c = (char*) malloc((selend - selstart) * 2 * sizeof(char));
	if (c) {
		o = objects + numobjects;
		parseidx = 0;
		while (o-- != objects) {
			parseidx += sprintf_s(c + parseidx, 100,
				"CreateObject(%d, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f);\r\n",
				o->id, o->x, o->y, o->z, o->rx, o->ry, o->rz);
		}
		SetFocus(hEdit);
		SendMessage(hEdit, EM_SETSEL, selstart, selend + 2);
		SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM) c);
	} else {
		ERRMSG(hMain, "Memory allocation failed");
	}

	free(objects);
}

/* purely so all text gets selected on ctrl+a */
LRESULT CALLBACK EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CHAR && wParam == 'A' - 64) {
		SendMessage(hwnd, EM_SETSEL, 0, -1);
		return 1;
	}
	return CallWindowProc(actualEditWndProc, hwnd, msg, wParam, lParam);
}

void CreateBulkWindow(HWND hwnd)
{
}

void CreateMainWindow(HWND hwnd)
{
	HWND control;
	int y = PAD, h;

	hLabel = CreateWindowExA(0, "Static", "-", WS_CHILD | WS_VISIBLE | SS_LEFT,
		PAD, y, SIDEBARSIZE, h = 18, hwnd, (HMENU) IDC_LABL, modulehandle, NULL);
	SendMessage(hLabel, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Paste from clipboard", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = 25, hwnd, (HMENU) IDC_PAST, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Copy to clipboard", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = 25, hwnd, (HMENU) IDC_COPY, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Bulk change ID", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = 25, hwnd, (HMENU) IDC_CHAI, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Bulk change X", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = 25, hwnd, (HMENU) IDC_CHAX, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Bulk change Y", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = 25, hwnd, (HMENU) IDC_CHAY, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Bulk change Z", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = 25, hwnd, (HMENU) IDC_CHAZ, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	hEdit = CreateWindowExA(
		WS_EX_CLIENTEDGE, "Edit", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
		ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
		SIDEBARSIZEEX, 8, 100, 100, hwnd, (HMENU) IDC_TEXT, modulehandle, NULL);
	SendMessage(hEdit, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	actualEditWndProc = (WNDPROC) SetWindowLongPtr(hEdit, GWL_WNDPROC, (LONG_PTR) &EditWndProc);
	/* send a msg that paste from clipboard button was clicked */
	PostMessageA(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PAST, 0), 0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE && hBulkedit) {
			PostMessage(hBulkedit, WM_CLOSE, 0, 0);
		}
		break;
	case WM_CREATE:
		if (window_being_created == WIN_MAIN) {
			CreateMainWindow(hwnd);
		} else if (window_being_created == WIN_BULK) {
			CreateBulkWindow(hwnd);
		}
		window_being_created = 0;
		break;
	case WM_SIZE:
	{
		RECT rcClient;

		if (hwnd == hMain) {
			GetClientRect(hwnd, &rcClient);
			/*hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);*/
			SetWindowPos(hEdit, NULL,
				0, 0, rcClient.right - SIDEBARSIZEEX - PAD, rcClient.bottom - PAD2,
				SWP_NOZORDER | SWP_NOMOVE);
		}
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHAI:
		case IDC_CHAX:
		case IDC_CHAY:
		case IDC_CHAZ:
			showBulkEdit();
			break;
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
	case WM_CLOSE:
		if (hwnd == hBulkedit) {
			EnableWindow(hMain, TRUE);
		}
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		if (hwnd == hMain) {
			PostQuitMessage(0);
		} else if (hwnd == hBulkedit) {
			hBulkedit = NULL;
		}
		break;
	default: return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	NONCLIENTMETRICS ncm;

	hInstance4windows = hInstance;


	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	hfDefault = CreateFontIndirect(&(ncm.lfMessageFont));
	modulehandle = GetModuleHandleA(NULL);

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
		ERRMSG(NULL, "Window reg failed.");
		return 0;
	}

	window_being_created = WIN_MAIN;
	hMain = CreateWindowEx(
		0,
		wc.lpszClassName,
		"title",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 448,
		NULL, NULL, hInstance, NULL
	);
	if (hMain == NULL) {
		ERRMSG(NULL, "Window creation failed.");
		return 0;
	}

	ShowWindow(hMain, nCmdShow);
	UpdateWindow(hMain);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}