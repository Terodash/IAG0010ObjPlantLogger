#pragma once
#include <string>
#include <iostream>
#include "afxmt.h"
#include "winsock2.h"

#if defined(UNICODE) || defined(_UNICODE)
#define _tcout std::wcout
#else
#define _tcout std::cout
#endif

using namespace std;

class ClientSocket
{
public:
	ClientSocket(CEvent* ptrStopEvent, BOOL* ptrDownloadingCompleted);
	~ClientSocket(void);
	int openConnection(void);
	SOCKET getSocket(void);
	int recv(void);
	char* getRecvMessage(void);
	void setSendMessage(_TCHAR*, int);
	int send(void);
	void closeConnection(void);
	BOOL getDownloadingCompleted(void);
private:
	SOCKET clientSocket; // Socket connected with the server
	CEvent* ptrStopEvent;
	CEvent* ptrWSARecvCompletedEvent;
	CEvent* ptrWSASendCompletedEvent;
	CEvent* ptrDataSentEvent;
	CSyncObject* ptrWSARecvCompletedEvents[2];
	CSyncObject* ptrWSASendCompletedEvents[2];
	CMultiLock* ptrWSARecvLock;
	CMultiLock* ptrWSASendLock;
	WSAOVERLAPPED* ptrRecvOverlapped;
	WSAOVERLAPPED* ptrSendOverlapped;
	DWORD nReceivedBytes;
	DWORD nSentBytes;
	DWORD receiveFlags;
	DWORD sendFlags;
	WSABUF* ptrRecvDataBuffer;
	WSABUF* ptrSendDataBuffer;
	char recvMessage[2048];
	char sendMessage[44];
	BOOL* ptrDownloadingCompleted;
};
