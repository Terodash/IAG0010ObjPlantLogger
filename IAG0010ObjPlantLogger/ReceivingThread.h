#pragma once
#include "afxwin.h"
#include "afxmt.h"
#include "ClientSocket.h"

#if defined(UNICODE) || defined(_UNICODE)
#define _tcout std::wcout
#else
#define _tcout std::cout
#endif

using namespace std;

class ReceivingThread :
	public CWinThread
{
public:
	ReceivingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, BOOL* ptrDownloadingCompleted, HANDLE* ptrFile);
	virtual ~ReceivingThread(void);
	virtual int Run(void);
	virtual BOOL InitInstance() { return TRUE; }
	void helloProcessing(void);
	void fileProcessing(void);

private:
	ClientSocket* ptrClientSocket;
	CEvent* ptrDataRecvEvent;
	CEvent* ptrDataSentEvent;
	CEvent* ptrStopEvent;
	CSyncObject* ptrDataSentEvents[2];
	CMultiLock* ptrDataSentLock;
	//std::ofstream * fichier;
	HANDLE* ptrFile;
	DWORD nWrittenBytes;
	BOOL* ptrDownloadingCompleted; // This variable indicate if the downloading has completed.
	BOOL firstRecv;
};

