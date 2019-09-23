
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#define ERRMSG(parent,text) MessageBoxA(parent,text,"oops",MB_ICONWARNING|MB_OK)
#define spr(...) sprintf_s(message,sizeof(message),__VA_ARGS__)

#define PAD (8)
#define PAD2 (PAD+PAD)
#define BTNHEIGHT (25)
#define SIDEBARSIZE (150)
#define SIDEBARSIZEEX (SIDEBARSIZE + PAD2)
#define BULKWINWIDTH 400

#define WIN_MAIN 50
#define WIN_BULK 51

#define IDC_TEXT 101
#define IDC_COPY 102
#define IDC_PAST 103
#define IDC_LABL 104
/* id, x, y, z must be consecutive numbers (in the same order as struct OBJECT values */
#define IDC_CHAI 105
#define IDC_CHAX 106
#define IDC_CHAY 107
#define IDC_CHAZ 108
#define IDC_BULK_LABL 201
/* imm, min, max, avg must be consecutive numbers (see doBulkEdit) */
#define IDC_BULK_IMMV 202
#define IDC_BULK_MINV 203
#define IDC_BULK_MAXV 204
#define IDC_BULK_AVGV 205
#define IDC_BULK_DIST 206
#define IDC_BULK_IMMT 207

struct OBJECT
{
	float values[7];
};

struct BULKVALUES
{
	float actual, min, max, avg;
	float *values[4];
};

char message[200];

int window_being_created;
int bulk_edit_value_idx;
int numobjects;
HINSTANCE hInstance4windows;
HFONT hfDefault;
HMODULE modulehandle;
WNDCLASSEX wc;
HWND hMain, hBulkedit = NULL, hEdit, hLabel, hBulkimmvalue;
WNDPROC actualEditWndProc;
struct BULKVALUES bulkvalues;
struct OBJECT *objects;

void doBulkEdit()
{
	MSG msg;
	RECT mainpos;
	int selstart, selend, linestart, lineend;
	int i;
	int linelen;
	char *linecontent;
	int parseidx;
	char parsingvalue[100], *c, *p;
	int valueidx, invalue;
	const int numvalues = sizeof(((struct OBJECT*) 0)->values)/sizeof(((struct OBJECT*) 0)->values[0]);
	struct OBJECT *o;

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
					o->values[++valueidx] = (float) atof(parsingvalue);
					if (valueidx >= numvalues) {
						break;
					}
				}
			}

		}
		free(linecontent);
		if (valueidx != numvalues - 1) {
			spr("invalid line, expected %d values but got %d"
				" (don't select empty lines)", numvalues, valueidx);
			ERRMSG(hMain, message);
			free(objects);
			return;
		}
		o++;
	}

	bulkvalues.max = -FLT_MAX;
	bulkvalues.min = FLT_MAX;
	bulkvalues.avg = 0.0f;
	o = objects + numobjects;
	while (o-- != objects) {
		if (o->values[bulk_edit_value_idx] < bulkvalues.min) {
			bulkvalues.min = o->values[bulk_edit_value_idx];
		}
		if (o->values[bulk_edit_value_idx] > bulkvalues.max) {
			bulkvalues.max = o->values[bulk_edit_value_idx];
		}
		bulkvalues.avg += o->values[bulk_edit_value_idx];
	}
	bulkvalues.avg /= numobjects;
	bulkvalues.values[0] = &bulkvalues.actual;
	bulkvalues.values[1] = &bulkvalues.min;
	bulkvalues.values[2] = &bulkvalues.max;
	bulkvalues.values[3] = &bulkvalues.avg;

	window_being_created = WIN_BULK;
	hBulkedit = CreateWindowEx(
		0,
		wc.lpszClassName,
		"Bulk edit",
		WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX),
		0, 0, BULKWINWIDTH, BULKWINWIDTH, /*x y pos will get adjusted in WM_CREATE*/
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
				(int) o->values[0], o->values[1], o->values[2], o->values[3],
				o->values[4], o->values[5], o->values[6]);
		}
		SetFocus(hEdit);
		SendMessage(hEdit, EM_SETSEL, selstart, selend + 2);
		SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM) c);
		/*re-select lines*/
		SendMessage(hEdit, EM_SETSEL, selstart, selstart + parseidx - 1);
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
	HWND control;
	int y = PAD, h;
	char buf[150];
	RECT rect, mainpos;
	int controlwidth, halfwidth;

	GetClientRect(hwnd, &rect);
	controlwidth = rect.right - rect.left - PAD2;
	halfwidth = (controlwidth - PAD) / 2;

	sprintf_s(buf, sizeof(buf), "Editing %d objects", numobjects);
	control = CreateWindowExA(0, "Static", buf, WS_CHILD | WS_VISIBLE | SS_LEFT,
		PAD, y, controlwidth, h = 18, hwnd, (HMENU) IDC_BULK_LABL, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	sprintf_s(buf, sizeof(buf), "%.4f", bulkvalues.min);
	hBulkimmvalue = CreateWindowExA(
		WS_EX_CLIENTEDGE, "Edit", buf,
		WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, halfwidth, h = 18,
		hwnd, (HMENU) IDC_BULK_IMMT, modulehandle, NULL);
	SendMessage(hBulkimmvalue, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Set", WS_CHILD | WS_VISIBLE, PAD2 + halfwidth, y,
		halfwidth, h = 18, hwnd, (HMENU) IDC_BULK_IMMV, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	sprintf_s(buf, sizeof(buf), "Use min %.4f", bulkvalues.min);
	control = CreateWindowExA(0, "Button", buf, WS_CHILD | WS_VISIBLE, PAD, y += h + PAD,
		controlwidth, h = BTNHEIGHT, hwnd, (HMENU) IDC_BULK_MINV, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	sprintf_s(buf, sizeof(buf), "Use max %.4f", bulkvalues.max);
	control = CreateWindowExA(0, "Button", buf, WS_CHILD | WS_VISIBLE, PAD, y += h + PAD,
		controlwidth, h = BTNHEIGHT, hwnd, (HMENU) IDC_BULK_MAXV, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	sprintf_s(buf, sizeof(buf), "Use avg %.4f", bulkvalues.avg);
	control = CreateWindowExA(0, "Button", buf, WS_CHILD | WS_VISIBLE, PAD, y += h + PAD,
		controlwidth, h = BTNHEIGHT, hwnd, (HMENU) IDC_BULK_AVGV, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	sprintf_s(buf, sizeof(buf), "Distribute between min && max");
	control = CreateWindowExA(0, "Button", buf, WS_CHILD | WS_VISIBLE, PAD, y += h + PAD,
		controlwidth, h = BTNHEIGHT, hwnd, (HMENU) IDC_BULK_DIST, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));

	GetWindowRect(hMain, &mainpos);
	rect.bottom = rect.top + y + h + PAD;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_OVERLAPPED, FALSE);
	SetWindowPos(hwnd, NULL,
		(mainpos.left + mainpos.right - (rect.right - rect.left)) / 2,
		(mainpos.top + mainpos.bottom - (rect.bottom - rect.top)) / 2,
		rect.right - rect.left, rect.bottom - rect.top, 0);
}

void CreateMainWindow(HWND hwnd)
{
	HWND control;
	int y = PAD, h;

	hLabel = CreateWindowExA(0, "Static", "-", WS_CHILD | WS_VISIBLE | SS_LEFT,
		PAD, y, SIDEBARSIZE, h = 18, hwnd, (HMENU) IDC_LABL, modulehandle, NULL);
	SendMessage(hLabel, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Paste from clipboard", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = BTNHEIGHT, hwnd, (HMENU) IDC_PAST, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Copy to clipboard", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = BTNHEIGHT, hwnd, (HMENU) IDC_COPY, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Bulk change ID", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = BTNHEIGHT, hwnd, (HMENU) IDC_CHAI, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Bulk change X", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = BTNHEIGHT, hwnd, (HMENU) IDC_CHAX, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Bulk change Y", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = BTNHEIGHT, hwnd, (HMENU) IDC_CHAY, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	control = CreateWindowExA(0, "Button", "Bulk change Z", WS_CHILD | WS_VISIBLE,
		PAD, y += h + PAD, SIDEBARSIZE, h = BTNHEIGHT, hwnd, (HMENU) IDC_CHAZ, modulehandle, NULL);
	SendMessage(control, WM_SETFONT, (WPARAM) hfDefault, MAKELPARAM(FALSE, 0));
	hEdit = CreateWindowExA(
		WS_EX_CLIENTEDGE, "Edit", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
		ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
		SIDEBARSIZEEX, PAD, 100, 100, hwnd, (HMENU) IDC_TEXT, modulehandle, NULL);
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
		case IDC_BULK_IMMV:
		{
			char buf[50];
			GetWindowText(hBulkimmvalue, buf, sizeof(buf));
			bulkvalues.actual = atof(buf);
		}
		case IDC_BULK_MINV:
		case IDC_BULK_MAXV:
		case IDC_BULK_AVGV:
		{
			struct OBJECT *o = objects + numobjects;
			while (o-- != objects) {
				o->values[bulk_edit_value_idx] =
					*bulkvalues.values[LOWORD(wParam) - IDC_BULK_IMMV];
			}
			PostMessage(hBulkedit, WM_CLOSE, 0, 0);
			break;
		}
		case IDC_BULK_DIST:
		{
			struct OBJECT *o = objects + numobjects;
			float increment;

			if (numobjects == 1) {
				ERRMSG(hBulkedit, "Cannot distribute for only one object");
				break;
			}

			increment = (bulkvalues.max - bulkvalues.min) / (numobjects - 1);
			while (o-- != objects) {
				o->values[bulk_edit_value_idx] =
					bulkvalues.min + increment * (o - objects);
			}
			PostMessage(hBulkedit, WM_CLOSE, 0, 0);
			break;
		}
		case IDC_CHAI:
		case IDC_CHAX:
		case IDC_CHAY:
		case IDC_CHAZ:
			bulk_edit_value_idx = LOWORD(wParam) - IDC_CHAI;
			doBulkEdit();
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