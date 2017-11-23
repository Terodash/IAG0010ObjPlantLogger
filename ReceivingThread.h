#pragma once
#include "afxwin.h"
#include "afxmt.h"
#include "ClientSocket.h"

#if defined(UNICODE) || defined(_UNICODE)
#define _tcout std::wcout
#else
#define _tcout std::cout
#endif

int writeToFile(char * data, HANDLE file); //will write the data received in a .txt file
const char * displayAndWrite(char *data); //reads the received data, displays in the console, and writes it to a file

using namespace std;

class ReceivingThread :
	public CWinThread
{
public:
	ReceivingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, CEvent* ptrSendPassword, BOOL* ptrStartOK, BOOL* ptrConnected, BOOL* sendConnectionNotAccepted, HANDLE* ptrFile, TCHAR* CommandBuf);
	virtual ~ReceivingThread(void);
	virtual int Run(void);
	virtual BOOL InitInstance() { return TRUE; }
	//void helloProcessing(void);
	int WriteToFile(char *dataToWrite, HANDLE file);
	void fileProcessing(char* data);

private:
	ClientSocket* ptrClientSocket;
	CEvent* ptrDataRecvEvent;
	CEvent* ptrDataSentEvent;
	CEvent* ptrSendPassword;
	CEvent* ptrStopEvent;
	CSyncObject* ptrDataSentEvents[2];
	CMultiLock* ptrDataSentLock;
	//std::ofstream * fichier;
	HANDLE* ptrFile;
	DWORD nWrittenBytes;
	BOOL* ptrStartOK;
	BOOL* ptrConnected;
	BOOL* sendConnectionNotAccepted;
	BOOL firstRecv;
	TCHAR* CommandBuf;
};

