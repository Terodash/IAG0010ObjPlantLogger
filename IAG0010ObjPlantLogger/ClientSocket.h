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
	ClientSocket(CEvent* ptrStopEvent, CEvent *ptrDataSendEvent);
	~ClientSocket(void);
	int openConnection(void);
	int recv(void);
	char* getRecvMessage(void);
	void setSendMessage(_TCHAR*, int);
	SOCKET getReceivedBytes() { return nReceivedBytes; }
	int send(void);
	void closeConnection(void);
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
};
