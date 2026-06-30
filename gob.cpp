
#include <iostream>
#include <fstream>
#include <cmath>
#include <shobjidl.h>       //  for file dialog, requires win vista+

#include "framework.h"
#include "gob.h"
#include "rnn.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GOB, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GOB));
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);	DispatchMessage(&msg);
	}	}
	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GOB));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GOB);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
   hInst = hInstance;
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd) return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		switch (wmId) {
			case IDM_NEWDATA:	newmodel();	break;
			case IDM_OPENDATA:	openmodel();	break;
			case IDM_SAVEDATA:	savemodel();	break;
			case IDM_TRAIN:		train(hWnd);	break;
			case IDM_STOPTRAIN: stop = 1;   break;
			case IDM_TRAIN10X: do10x = !do10x;   break;
			case IDM_USEGRADNORM:	GRADNORM = !GRADNORM;
				CheckMenuItem(GetMenu(hWnd), IDM_USEGRADNORM, MF_BYCOMMAND | (GRADNORM ? MF_CHECKED : MF_UNCHECKED));	break;
			case IDM_USEADAM:	ADAM = !ADAM;
				CheckMenuItem(GetMenu(hWnd), IDM_USEADAM, MF_BYCOMMAND | (ADAM ? MF_CHECKED : MF_UNCHECKED));	break;
			case IDM_OUTPUT:	sampleoutput(hWnd);	break;
			case IDM_ABOUT:	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);	break;
			case IDM_EXIT:	DestroyWindow(hWnd);	break;
			default:	return DefWindowProc(hWnd, message, wParam, lParam);
		}
    }	break;
	case WM_CREATE: {
		HDC hdc;
		BITMAPINFO bitmapinfo;
		hdc = CreateCompatibleDC(NULL);
		bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapinfo.bmiHeader.biWidth = dispx;
		bitmapinfo.bmiHeader.biHeight = -dispy;
		bitmapinfo.bmiHeader.biPlanes = 1;
		bitmapinfo.bmiHeader.biBitCount = 32;
		bitmapinfo.bmiHeader.biCompression = BI_RGB;
		bitmapinfo.bmiHeader.biSizeImage = 0;
		bitmapinfo.bmiHeader.biClrUsed = 256;
		bitmapinfo.bmiHeader.biClrImportant = 256;
		framebmp = CreateDIBSection(hdc, &bitmapinfo, DIB_RGB_COLORS, (void**)&framebuf, 0, 0);
		hdcframe = CreateCompatibleDC(NULL);
		oldframe = (HBITMAP)SelectObject(hdcframe, framebmp);
		backgroundbmp = CreateDIBSection(hdc, &bitmapinfo, DIB_RGB_COLORS, (void**)&backgroundbuf, 0, 0);
		hdcbackground = CreateCompatibleDC(NULL);
		oldbackground = (HBITMAP)SelectObject(hdcbackground, backgroundbmp);

		//hfont0 = CreateFont(25, 0, 0, 0, 0, 0, 0, 0, 0, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, 0);
		hfont0 = CreateFont(24, 0, 0, 0, 0, 0, 0, 0, 0, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 0, 0);//MS Sans Serif
		hfont1 = CreateFont(16, 0, 0, 0, 0, 0, 0, 0, 0, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 0, 0);

		SelectObject(hdcbackground, CreateSolidBrush(RGB(16, 48, 24)));
		PatBlt(hdcbackground, 0, 0, dispx, dispy, PATCOPY);
		DeleteObject(SelectObject(hdcbackground, GetStockObject(NULL_BRUSH)));

		for (unsigned int i = 64; i < 192; i++) for (unsigned int j = 104; j < 132; j++) SetPixel(hdcbackground, i, j, RGB(48, 56, 56));
		BitBlt(hdcbackground, 64, 176, 128, 28, hdcbackground, 64, 104, SRCCOPY);	//	model param display selector box 232
		BitBlt(hdcbackground, 814, 212, 128, 28, hdcbackground, 64, 104, SRCCOPY);	//	generate button
		for (unsigned int i = 118; i < 138; i++) for (unsigned int j = 116; j < 120; j++) SetPixel(hdcbackground, i, j, RGB(0, 0, 0));//-
		BitBlt(hdcbackground, 64, 140, 128, 28, hdcbackground, 64, 104, SRCCOPY);	//	168
		BitBlt(hdcbackground, 256, 140, 128, 28, hdcbackground, 64, 104, SRCCOPY);
		for (unsigned int i = 126; i < 130; i++) for (unsigned int j = 108; j < 128; j++) SetPixel(hdcbackground, i, j, RGB(0, 0, 0));//+
		BitBlt(hdcbackground, 256, 104, 128, 28, hdcbackground, 64, 104, SRCCOPY);
		BitBlt(hdcbackground, 448, 104, 128, 68, hdcbackground, 64, 104, SRCCOPY);
		
		for (unsigned int i = 256; i < 768; i++) {	//	top and bottom lines for hidden
			SetPixel(hdcbackground, i, 175, RGB(120, 100, 60));	//228, 292
			SetPixel(hdcbackground, i, 240, RGB(120, 100, 60));
		}

		for (unsigned int i = 64; i < 942; i++) for (unsigned int j = 251; j < 283; j++) SetPixel(hdcbackground, i, j, RGB(16, 64, 48));
		HINSTANCE hInst = GetModuleHandle(NULL);	//	this app instance..
		hEdit = CreateWindowEx(
			WS_EX_TRANSPARENT, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
			64, 251, 878, 32, hWnd, (HMENU)1010, hInst, NULL);	//304	878 wide
		ShowWindow(hEdit, SW_HIDE);

		SendMessage(hEdit, WM_SETFONT, (WPARAM)hfont0, MAKELPARAM(TRUE, 0));

		SelectObject(hdcbackground, hfont0);
		SetTextColor(hdcbackground, RGB(125, 125, 92)); SetBkMode(hdcbackground, TRANSPARENT);
		TextOutA(hdcbackground, 67, 75, "learn:", 6);
		TextOutA(hdcbackground, 264, 75, "temp:", 5);
		TextOutA(hdcbackground, 456, 75, "reg:", 4);

		TextOutA(hdcbackground, 837, 213, "generate", 9);	//814,212+3+7
		SetTimer(hWnd, 1, timerate, NULL);
		CheckMenuItem(GetMenu(hWnd), IDM_USEADAM, MF_BYCOMMAND | (ADAM ? MF_CHECKED : MF_UNCHECKED));
	}	break;
	case WM_KEYDOWN: {
		if ((HIWORD(lParam) & KF_REPEAT) < 1) {
			switch (LOWORD(wParam)) {
				case VK_ESCAPE:	PostMessage(hWnd, WM_CLOSE, 0, 0);	break;
				case VK_SPACE:  break;
		}	}
	}   break;
	case WM_LBUTTONDOWN: {
		POINT mouloc;	GetCursorPos(&mouloc);	ScreenToClient(hWnd, &mouloc);

		//814, 212, 128, 28
		if (mouloc.y > 251 && mouloc.y < 283) {
			showinput = !showinput;
			showinput ? ShowWindow(hEdit, SW_SHOW) : ShowWindow(hEdit, SW_HIDE);
			GetWindowTextA(hEdit, intext, sizeof(intext));
		}
		else if (mouloc.y > 100 && mouloc.y < 176 && mouloc.y > 20 && mouloc.x > 56 && mouloc.x < 592) {
			if (mouloc.x < 212) mouloc.y < 136 ? learnix = max(0, learnix - 1) : learnix = min(33, learnix + 1);
			else {
				if (mouloc.x < 404) mouloc.y < 136 ? tempix = max(0, tempix - 1) : tempix = min(9, tempix + 1);
				else mouloc.y < 136 ? regix = max(0, regix - 1) : regix = min(30, regix + 1);
		}	}
		else if (mouloc.y < 207 && mouloc.y > 173 && mouloc.x > 56 && mouloc.x < 212) displayindex = (displayindex + 1) % 7;
		else if (mouloc.y < 243 && mouloc.y > 209 && mouloc.x > 813 && mouloc.x < 943) sampleoutput(hWnd);
		else if (mouloc.y > 284) {
			if (OpenClipboard(NULL)) {
				char ttext[1024] = { 0 };
				for (int i = 0; i < 8; i++) for (int j = 0; j < 128; j++) {
					if (outext[i][j] > 31 && outext[i][j] < 126) ttext[(i << 7) + j] = outext[i][j];
					else ttext[(i << 7) + j] = 32;
				}
				EmptyClipboard();
				size_t length = sizeof(ttext);
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, length + 1);
				if (hMem == NULL) CloseClipboard();
				char* pMem = (char*)GlobalLock(hMem);
				memcpy(pMem, ttext, length);
				pMem[length] = '\0';
				GlobalUnlock(hMem);
				SetClipboardData(CF_TEXT, hMem);
				CloseClipboard();
			}
		}
	}   break;
    case WM_PAINT: {
            PAINTSTRUCT ps;
                        BitBlt(hdcframe, 0, 0, dispx, dispy, NULL, 0, 0, BLACKNESS);    SetBkMode(hdcframe, TRANSPARENT);
            BitBlt(hdcframe, 0, 0, dispx, dispy, hdcbackground, 0, 0, SRCCOPY);
            SelectObject(hdcframe, hfont0); SetTextColor(hdcframe, RGB(92, 112, 112));

            char str[32];
            float tf = learnarr[learnix];
            sprintf_s(str, "%.8f", tf);    TextOutA(hdcframe, 121, 75, str, strlen(str));  //  130	
            tf = temparr[tempix];
            sprintf_s(str, " %.3f", tf);    TextOutA(hdcframe, 324, 75, str, strlen(str));
			tf = regarr[regix];
			sprintf_s(str, " %1.0e", tf);    TextOutA(hdcframe, 516, 75, str, strlen(str));

            out1[127] = '\0';
            TextOutA(hdcframe, 4, 20, out1, strlen(out1));

            SetTextColor(hdcframe, RGB(92, 112, 112));
            sprintf_s(str, "tests:   %d", tests);    TextOutA(hdcframe, 648, 75, str, strlen(str));
            sprintf_s(str, "error:   %.6f", edisp);    TextOutA(hdcframe, 812, 75, str, strlen(str));

            SetTextColor(hdcframe, RGB(112, 160, 176));	//(92, 112, 112));
            out0[127] = '\0';
            TextOutA(hdcframe, 4, 48, out0, strlen(out0));

            if (do10x) {
                sprintf_s(str, "of 10:  %d", thismany);    TextOutA(hdcframe, 648, 99, str, strlen(str));
            }

			if (ADAM) TextOutA(hdcframe, 648, 123, "use ADAM", 8);

			switch (displayindex) {
				case 1:	TextOutA(hdcframe, 87, 177, "hidden 0", 8);
					for (int i = 0; i < hidd; i++) {
						int gx = 256 + (i << 1);	int gy = 207 - (h0[winm][i] * 32.f);//	260
						SetPixel(hdcframe, gx, gy, RGB(180, 100, 220));	SetPixel(hdcframe, gx + 1, gy, RGB(180, 100, 220));
						gy = 207 - (h0[0][i] * 32.f);
						SetPixel(hdcframe, gx, gy, RGB(100, 200, 250));	SetPixel(hdcframe, gx + 1, gy, RGB(100, 200, 250));
					}	break;
				case 2:	TextOutA(hdcframe, 87, 177, "hidden", 6);
					for (int i = 0; i < hidd; i++) {
						int gx = 256 + (i << 1);	int gy = 207 - (h[winm][i] * 32.f);
						SetPixel(hdcframe, gx, gy, RGB(180, 100, 220));	SetPixel(hdcframe, gx + 1, gy, RGB(180, 100, 220));
						gy = 207 - (h[0][i] * 32.f);
						SetPixel(hdcframe, gx, gy, RGB(100, 200, 250));	SetPixel(hdcframe, gx + 1, gy, RGB(100, 200, 250));
					}	break;
				case 3:	TextOutA(hdcframe, 87, 177, "h0bias", 5);
					for (int i = 0; i < hidd; i++) {
						int gx = 256 + (i << 1);	int gy = 207 - (h0bias[i] * 32.f);
						SetPixel(hdcframe, gx, gy, RGB(50, 100, 220));	SetPixel(hdcframe, gx + 1, gy, RGB(50, 100, 220));}	break;
				case 4:	TextOutA(hdcframe, 87, 177, "hbias", 5);
					for (int i = 0; i < hidd; i++) {
						int gx = 256 + (i << 1);	int gy = 207 - (hbias[i] * 32.f);
						SetPixel(hdcframe, gx, gy, RGB(50, 100, 220));	SetPixel(hdcframe, gx + 1, gy, RGB(50, 100, 220));}	break;
				case 5:	TextOutA(hdcframe, 87, 177, "obias", 5);
					for (int i = 0; i < 96; i++) {
						int gx = 256 + (i << 1);	int gy = 207 - (obias[i] * 32.f);
						SetPixel(hdcframe, gx, gy, RGB(50, 100, 220));	SetPixel(hdcframe, gx + 1, gy, RGB(50, 100, 220));}	break;
				case 6:	TextOutA(hdcframe, 87, 177, "netout", 6);
					for (int i = 0; i < 96; i++) {
						int gx = 256 + (i << 1);	int gy = 207 - (netout[0][i] * 32.f);
						SetPixel(hdcframe, gx, gy, RGB(50, 100, 220));	SetPixel(hdcframe, gx + 1, gy, RGB(50, 100, 220));}	break;
				default:	break;
			}

			unsigned char rch[10] = { 255,	192,	128,	64,		0,		0,		0,		0,		0,		64 };
			unsigned char gch[10] = { 0,	64,		128,	192,	255,	192,	128,	64,		0,		0 };
			unsigned char bch[10] = { 0,	0,		0,		0,		0,		64,		128,	192,	255,	192 };
		//	SetPixel(hdcbackground, 64 + (tests >> 9), 460 - (int)((edisp - 0.8f) * 384), RGB(rch[thismany], gch[thismany], bch[thismany]));
			SetPixel(hdcbackground, 64 + (tests >> 9), dispy - (int)(edisp * 512.f), RGB(rch[dispix], gch[dispix], bch[dispix]));



			SetTextColor(hdcframe, RGB(112, 160, 168));	//(92, 112, 112));
			TextOutA(hdcframe, 64, 251, intext, strlen(intext));	//	304 user input display
			if (sizeof(outext) > 0) {
				for (int i = 0; i < 8; i++) TextOutA(hdcframe, 4, 283 + i * 24, outext[i], strlen(outext[i]));	//	287 30
			}

            HDC hdc = BeginPaint(hWnd, &ps);
            BitBlt(hdc, 0, 0, dispx, dispy, hdcframe, 0, 0, SRCCOPY);
            EndPaint(hWnd, &ps);
	}	break;
	case WM_TIMER: {
		InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
	}   break;
    case WM_DESTROY:	stop = 1;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
		case WM_INITDIALOG:	return (INT_PTR)TRUE;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
    }
    return (INT_PTR)FALSE;
}
