// ChatServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ChatServer.h"
#include <iostream>

#include "stdio.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

#define MAX_LOADSTRING 100

#define WM_SOCKET WM_USER + 1

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

SOCKET registeredClients[64];   // Chua cac client da dang nhap
int numRegisteredClients = 0;
char *ids[64];

void RemoveClient(SOCKET, HWND);

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CHATSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHATSERVER));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHATSERVER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CHATSERVER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 370, 450, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL,
		10, 10, 160, 350, hWnd, (HMENU)IDC_STATIC, GetModuleHandle(NULL), NULL);

	CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL,
		180, 10, 160, 350, hWnd, (HMENU)IDC_STATIC, GetModuleHandle(NULL), NULL);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);

	WSAAsyncSelect(listener, hWnd, WM_SOCKET, FD_ACCEPT);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket(wParam);
			RemoveClient(wParam, hWnd);
			break;
		}

		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKET client = accept(wParam, NULL, NULL);
			WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);

			char msg[] = "New client connected";
			SendDlgItemMessageA(hWnd, IDC_STATIC, LB_ADDSTRING, 0, (LPARAM)msg);
			SendDlgItemMessageA(hWnd, IDC_STATIC, WM_VSCROLL, SB_BOTTOM, 0);
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			char buf[256];
			int ret = recv(wParam, buf, sizeof(buf), 0);
			if (ret <= 0)
			{
				closesocket(wParam);
				RemoveClient(wParam, hWnd);
				break;
			}

			// Xu ly du lieu
			buf[ret] = 0;
			SendDlgItemMessageA(hWnd, IDC_STATIC, LB_ADDSTRING, 0, (LPARAM)buf);
			SendDlgItemMessageA(hWnd, IDC_STATIC, WM_VSCROLL, SB_BOTTOM, 0);

			// Kiem tra trang thai cua client
			// Va xu ly du lieu theo trang thai tuong ung

			int j = 0;
			for (; j < numRegisteredClients; j++)
				if (wParam == registeredClients[j])
					break;

			char cmd[64];
			char id[64];
			char tmp[64];

			char errorMsg[] = "Loi cu phap. Hay nhap lai\n";

			char sendBuf[256];
			char targetId[64];

			if (j == numRegisteredClients)
			{
				// Trang thai chua dang nhap
				// Kiem tra cu phap client_id: [id]
				ret = sscanf(buf, "%s %s %s", cmd, id, tmp);
				if (ret == 2)
				{
					if (strcmp(cmd, "client_id:") == 0)
					{
						char okMsg[] = "Dung cu phap. Hay nhap thong diep muon gui.\n";
						send(wParam, okMsg, strlen(okMsg), 0);

						// Luu client dang nhap thanh cong vao mang
						registeredClients[numRegisteredClients] = wParam;
						ids[numRegisteredClients] = (char *)malloc(strlen(id) + 1);
						memcpy(ids[numRegisteredClients], id, strlen(id) + 1);
						numRegisteredClients++;

						SendDlgItemMessageA(hWnd, IDC_STATIC, LB_ADDSTRING, 0, (LPARAM)id);
						SendDlgItemMessageA(hWnd, IDC_STATIC, WM_VSCROLL, SB_BOTTOM, 0);
					}
					else
						send(wParam, errorMsg, strlen(errorMsg), 0);
				}
				else
					send(wParam, errorMsg, strlen(errorMsg), 0);
			}
			else
			{
				// Trang thai da dang nhap
				ret = sscanf(buf, "%s", targetId);
				if (ret == 1)
				{
					if (strcmp(targetId, "all") == 0)
					{
						sprintf(sendBuf, "%s: %s", ids[j], buf + strlen(targetId) + 1);

						for (int j = 0; j < numRegisteredClients; j++)
							if (registeredClients[j] != wParam)
								send(registeredClients[j], sendBuf, strlen(sendBuf), 0);
					}
					else
					{
						sprintf(sendBuf, "%s: %s", ids[j], buf + strlen(targetId) + 1);

						for (int j = 0; j < numRegisteredClients; j++)
							if (strcmp(ids[j], targetId) == 0)
								send(registeredClients[j], sendBuf, strlen(sendBuf), 0);
					}
				}
			}
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
			closesocket(wParam);
			RemoveClient(wParam, hWnd);
			break;
		}
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void RemoveClient(SOCKET client, HWND hWnd)
{
	// Xoa socket client khoi mang registeredClients
	int i = 0;
	for (; i < numRegisteredClients; i++)
		if (registeredClients[i] == client)
			break;

	if (i < numRegisteredClients)
	{
		// Xoa khoi mang
		if (i < numRegisteredClients - 1)
		{
			for (int j = i; j < numRegisteredClients - 1; j++)
			{
				registeredClients[j] = registeredClients[j + 1];
				ids[j] = ids[j + 1];
			}
		}

		numRegisteredClients--;

		// Xoa khoi list box
		SendDlgItemMessageA(hWnd, IDC_STATIC, LB_DELETESTRING, i, 0);
	}
}