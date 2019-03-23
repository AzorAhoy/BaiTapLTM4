// TelnetServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TelnetServer.h"
#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

#define MAX_LOADSTRING 100

#define WM_SOCKET WM_USER + 1

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

SOCKET clients[64];
int numClients = 0;

SOCKET registeredClients[64];
int registered = 0;
char * ids[64];

int ret;

char buf[256];

char id[64];
char cmd[64];
char tmp[64];

char errorMsg[] = "Syntax Error. Please try again.\n";

char sendBuf[256];
char fileBuf[256];
char cmdBuf[256];
char targetID[64];

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
	LoadStringW(hInstance, IDC_TELNETSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TELNETSERVER));

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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TELNETSERVER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TELNETSERVER);
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
		CW_USEDEFAULT, 0, 300, 450, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	// Tao list box
	CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL,
		10, 10, 160, 350, hWnd, (HMENU)IDC_STATIC, GetModuleHandle(NULL), NULL);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);

	WSAAsyncSelect(listener, hWnd, WM_SOCKET, FD_ACCEPT);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

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
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKADDR_IN clientAddr;
			int clientAddrLen = sizeof(clientAddr);

			SOCKET client = accept(wParam, (SOCKADDR *)&clientAddr, &clientAddrLen);

			char *ip = inet_ntoa(clientAddr.sin_addr);

			char portBuf[8];
			_itoa(ntohs(clientAddr.sin_port), portBuf, 10);

			char clientBuf[32];
			strcpy(clientBuf, ip);
			strcat(clientBuf, ":");
			strcat(clientBuf, portBuf);

			SendDlgItemMessageA(hWnd, IDC_STATIC, LB_ADDSTRING,
				0, (LPARAM)clientBuf);

			WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
		}

		if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			char buf[256];
			int ret = recv(wParam, buf, sizeof(buf), 0);

			buf[ret] = 0;
			//SendDlgItemMessageA(hWnd, IDC_STATIC, LB_ADDSTRING,
				//0, (LPARAM)buf);
			// Kiem tra trang thai cua client
			// Va xu ly du lieu theo trang thai tuong ung

			int j = 0;
			for (; j < registered; j++) {
				if (wParam == registeredClients[j])
					break;
			}
			if (j == registered)
			{
				// Trang thai chua dang nhap
				// Kiem tra cu phap client_id: [id]
				ret = sscanf(buf, "%s %s %s", cmd, id, tmp);
				if (ret == 2)
				{
					int found = 0;
					FILE *f = fopen("users.txt", "r");
					while (fgets(fileBuf, sizeof(fileBuf), f))
					{
						printf("%s\n", fileBuf);
						if (strcmp(buf, fileBuf) == 0)
						{
							found = 1;
							break;
						}
					}
					fclose(f);

					if (found == 1)
					{
						char msg[] = "Dang nhap thanh cong. Hay nhap lenh.\n";
						send(wParam, msg, strlen(msg), 0);
						registeredClients[registered] = wParam;
						ids[registered] = (char *)malloc(strlen(id) + 1);
						memcpy(ids[registered], id, strlen(id) + 1);
						++registered;
						break;
					}
					else
					{
						char msg[] = "Dang nhap that bai. Hay thu lai.\n";
						send(wParam, msg, strlen(msg), 0);
					}
					clients[numClients] = wParam;
					numClients++;
				}
				else
				{
					char msg[] = "Dang nhap that bai. Hay thu lai.\n";
					send(wParam, msg, strlen(msg), 0);
				}

			}
			else
			{
				// Trang thai da dang nhap
				SendDlgItemMessageA(hWnd, IDC_STATIC, LB_ADDSTRING, 0, (LPARAM)buf);
				if (buf[ret - 1] == '\n')
				{
					buf[ret - 1] = 0;
				}
				//printf("Received from %d: %s\n", clients[i], buf);

				sprintf(cmdBuf, "%s > out1.txt", buf);
				system(cmdBuf);

				FILE *f = fopen("out1.txt", "r");
				while (fgets(fileBuf, sizeof(fileBuf), f))
				{
					send(wParam, fileBuf, strlen(fileBuf), 0);
				}
				fclose(f);
			}
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
			closesocket(wParam);
			return -1;
		}

		//for (int i = 0; i < numClients; i++) {

		//	if (WSAGETSELECTEVENT(clients[i]) == FD_READ)
		//	{
		//		printf("rferg");
		//		ret = recv(clients[i], buf, sizeof(buf), 0);

		//		if (ret <= 0)
		//		{
		//			// Ket noi bi huy
		//			continue;
		//		}

		//		buf[ret] = 0;
		//		if (buf[ret - 1] == '\n')
		//		{
		//			buf[ret - 1] = 0;
		//		}
		//		//printf("Received from %d: %s\n", clients[i], buf);

		//		sprintf(cmdBuf, "%s > out1.txt", buf);
		//		system(cmdBuf);

		//		FILE *f = fopen("out.txt", "r");
		//		while (fgets(fileBuf, sizeof(fileBuf), f))
		//		{

		//			send(clients[i], fileBuf, strlen(fileBuf), 0);
		//		}
		//		fclose(f);
		//	}
		//}
	}
	break;
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