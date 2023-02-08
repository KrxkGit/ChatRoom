// 宿舍聊天室.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "宿舍聊天室.h"


// 全局变量: 
HINSTANCE hInst;	// 当前实例
SOCKET ConnectSocket = INVALID_SOCKET;
SOCKET ServiceSocket = INVALID_SOCKET;

#define DEFAULT_PORT1 "32170"//默认端口1
#define DEFAULT_PORT2 "32171"//默认端口2
#define MAX_SENDLEN 1024 //单次发送最大长度
#define CONNECTSUCCESS "Success"


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  在此放置代码。
	hInst = hInstance;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);

	closesocket(ConnectSocket);
	closesocket(ServiceSocket);
	WSACleanup();

	return GetLastError();
}

VOID OnConnect(HWND hDlg)
{
	char addrIP[20];
	GetDlgItemTextA(hDlg, IDC_EDIT1, addrIP, _countof(addrIP));


	addrinfo hints, *result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (ServiceSocket == INVALID_SOCKET) {
		getaddrinfo(addrIP, DEFAULT_PORT1, &hints, &result);
	}
	else {
		getaddrinfo(addrIP, DEFAULT_PORT2, &hints, &result);
	}

	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	connect(ConnectSocket, result->ai_addr, result->ai_addrlen);
	
	freeaddrinfo(result);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), FALSE);

}

VOID OnSend(HWND hDlg)
{
	int datasize;
	TCHAR sz[MAX_SENDLEN];
	GetDlgItemText(hDlg, IDC_EDIT2, sz, _countof(sz));

	ZeroMemory(sz, sizeof(sz));

	datasize = (lstrlen(sz))*sizeof(TCHAR)-1;
	send(ConnectSocket, (char*)&datasize, sizeof(datasize), 0);
	send(ConnectSocket, (char*)sz, sizeof(sz), 0);
	SetDlgItemText(hDlg, IDC_EDIT2, _T(""));

}

UINT __stdcall WaitToConnect(LPVOID hWnd)
{
	HWND hWR = GetDlgItem((HWND)hWnd,IDC_EDIT3);//对话框接收窗口
	
	addrinfo hints, *result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (ConnectSocket == INVALID_SOCKET) {
		getaddrinfo(NULL, DEFAULT_PORT1, &hints, &result);
	}
	else {
		getaddrinfo(NULL, DEFAULT_PORT2, &hints, &result);
	}

	ServiceSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	bind(ServiceSocket, result->ai_addr, result->ai_addrlen);
	listen(ServiceSocket, 5);
	freeaddrinfo(result);
	MessageBox((HWND)hWnd, _T("等待连接中"), _T("局域网聊天器"), MB_ICONINFORMATION);

	
	SOCKET recvSocket;
	sockaddr_in addr;
	int addrlen = sizeof(addr);
	recvSocket = accept(ServiceSocket, (sockaddr*)&addr, &addrlen);
	SetWindowTextA(GetDlgItem((HWND)hWnd,IDC_EDIT1),inet_ntoa(addr.sin_addr));

	
	MessageBox((HWND)hWnd, _T("连接成功"), _T("局域网聊天器"), MB_ICONINFORMATION);

	//while (true)
	//{
		int datasize;
		recv(recvSocket, (char*)&datasize, sizeof(datasize), 0);
		char* psz = new char[datasize];
		ZeroMemory(psz, datasize);
		recv(recvSocket, (char*)psz, datasize, 0);
		SetWindowText(hWR, (LPTSTR)psz);

		HANDLE hFile = CreateFile(_T("D:\\2.txt"), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		DWORD dwSize=datasize;
		WriteFile(hFile, psz, datasize, &dwSize, 0);
		CloseHandle(hFile);
		delete[]psz;
	//}

	return 0;
}

VOID OnInit(HWND hDlg)
{
	HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MY));
	SendMessage(hDlg, WM_SETICON, 0, (LPARAM)hIcon);
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		OnInit(hDlg);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDM_CONNECT:
			OnConnect(hDlg);
			break;
		case IDOK://发送
			OnSend(hDlg);
			break;
		case IDM_WAITCONNECT:
			_beginthreadex(0, 0, WaitToConnect, hDlg, 0, NULL);
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
