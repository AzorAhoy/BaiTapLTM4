// Client1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

#define WM_SOCKET WM_USER + 1

BOOL CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(9000);

	connect(client, (SOCKADDR *)&addr, sizeof(addr));

	WNDCLASS wndclass;
	CHAR providerClass[] = "AsyncSelect";
	HWND window;

	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = (LPCWSTR)providerClass;

	if (RegisterClass(&wndclass) == 0)
		return NULL;

	// Create a window
	if ((window = CreateWindow((LPCWSTR)providerClass, L"", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL)) == NULL)
		return -1;

	WSAAsyncSelect(client, window, WM_SOCKET, FD_READ | FD_CLOSE);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

BOOL CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SOCKET)
	{
		// Cac su kien cua socket

		// Kiem tra loi
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket(wParam);
			return -1;
		}

		// Kiem tra cac su kien
		if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			char buf[256];
			int ret = recv(wParam, buf, sizeof(buf), 0);

			buf[ret] = 0;
			printf("Received: %s\n", buf);
		}

		else if (WSAGETSELECTEVENT(lParam) == FD_WRITE)
		{
			//int ret = recv(client, buf, sizeof(buf), 0);

			//buf[ret] = 0;
			//printf("Received: %s\n", buf);
			char buf[256];
			printf("Nhap du lieu: ");
			gets_s(buf, sizeof(buf));
			//char * pch;
			//pch = strchr(buf, ':');
			//if (pch != NULL)
			//{
			//	printf("found at %d\n", pch - buf + 1);
			//	//pch = strchr(pch + 1, ':');
			//	
			//}
			send(wParam, buf, strlen(buf), 0);
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
			closesocket(wParam);
			return -1;
		}
	}

	// Xu ly cac su kien khac
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
