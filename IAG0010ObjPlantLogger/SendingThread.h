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

class SendingThread :
	public CWinThread
{
public:
	SendingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, CEvent* ptrCommandGot, BOOL* ptrDownloadingCompleted);
	virtual ~SendingThread(void);
	virtual int Run(void);
	virtual BOOL InitInstance() { return TRUE; }

private:
	ClientSocket* ptrClientSocket;
	CEvent* ptrDataRecvEvent;
	CEvent* ptrDataSentEvent;
	CEvent* ptrStopEvent;
	CEvent* ptrCommandProcessed;
	CEvent* ptrCommandGot;
	CSyncObject* ptrCommandEvents[2];
	CSyncObject* ptrDataRecvEvents[2];
	CMultiLock* ptrDataRecvLock;
	CMultiLock* ptrCommandEventsLock;
	BOOL* ptrDownloadingCompleted; // This variable indicate if the downloading has completed.
	BOOL firstSending;
};

