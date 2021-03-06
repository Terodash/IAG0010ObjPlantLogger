// IAG0010ObjPlantLogger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "afxwin.h"

#include "resource.h"
#include "IAG0010ObjPlantLogger.h"

CWinApp theApp;
using namespace std;

int _tmain(int argc, TCHAR* argv[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// Initialise MFC
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			_tprintf(_T("Error, MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// Windsock initialization.
			WSADATA WSAData;
			WSAStartup(MAKEWORD(2, 2), &WSAData);

			// Events initialization.
			CEvent* ptrDataRecvEvent = new CEvent(false, true, NULL, NULL); // A packet has been received
			CEvent* ptrDataSentEvent = new CEvent(true, true, NULL, NULL); // A packet has been sent
			CEvent* ptrStopEvent = new CEvent(false, true, NULL, NULL); // Client order to stop application
			CEvent* ptrCommandGot = new CEvent(true, true, NULL, NULL); // A command has been entered
			CEvent* ptrCommandProcessed = new CEvent(false, true, NULL, NULL); // A command has been processed
			

			// ClientSocket creation and connection.
			ClientSocket* ptrClientSocket = new ClientSocket(ptrStopEvent, ptrDataSentEvent); // Socket to communicate with the server
			if (ptrClientSocket->openConnection() == 1) {
				getchar();
				return 1;
			}

			// File creation
			HANDLE file; // File received by downloading
			file = CreateFile(_T("output_logger.txt"), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file == INVALID_HANDLE_VALUE) {
				_tcout << "Unable to create file, error " << GetLastError() << endl;
				getchar();
				return 1;
			}

			// Thread creation and starting.
			ReadingKeyboardThread* ptrReadingKeyboardThread = new ReadingKeyboardThread(ptrStopEvent, ptrCommandGot, ptrCommandProcessed); // Thread to received the "exit" command throw the terminal.
			ptrReadingKeyboardThread->CreateThread(); // Call Run.
			ReceivingThread* ptrReceivingThread = new ReceivingThread(ptrClientSocket, ptrDataRecvEvent, ptrDataSentEvent, ptrStopEvent, &file); // Thread to receive data from the server.
			ptrReceivingThread->CreateThread(); // Call Run.
			SendingThread* ptrSendingThread = new SendingThread(ptrClientSocket, ptrDataRecvEvent, ptrDataSentEvent, ptrStopEvent, ptrCommandGot, ptrCommandProcessed); // Thread to send data to the server.
			ptrSendingThread->CreateThread(); // Call Run.

			CSingleLock* ptrStopEventLock = new CSingleLock(ptrStopEvent); // Wait stopEvent.

																		   // Application closing

			ptrStopEventLock->Lock(INFINITE);
			//Sleep(5000);
			ptrClientSocket->closeConnection();
			CloseHandle(file);
			WSACleanup();
			system("output_logger.txt");


			// TODO: Until there.
		}
	}
	else
	{
		// TODO: modifiez le code d'erreur selon les besoins
		_tprintf(_T("GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}





