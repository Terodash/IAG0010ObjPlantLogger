// IAG0010ObjPlantLogger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "afxwin.h"

#include "resource.h"
#include "IAG0010ObjPlantLogger.h"

CWinApp theApp;
using namespace std;

BOOL startOK = FALSE;
BOOL connected = FALSE;
BOOL sendConnectionNotAccepted = TRUE;
//Est-ce une bonne chose de créer ces booléens. N'y a-t-il pas une classe qui a comme argument cela ? 

int _tmain(int argc, TCHAR* argv[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// Initialise MFC et affiche un message d'erreur en cas d'�chec
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: modifiez le code d'erreur selon les besoins
			_tprintf(_T("Error, MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: codez le comportement de l'application � cet emplacement.

			// Windsock initialization.
			WSADATA WSAData;
			WSAStartup(MAKEWORD(2, 2), &WSAData);


			// Events initialization.
			CEvent* ptrDataRecvEvent = new CEvent(false, true, NULL, NULL); // A packet has been received
			CEvent* ptrDataSentEvent = new CEvent(true, true, NULL, NULL); // A packet has been sent
			CEvent* ptrStopEvent = new CEvent(false, true, NULL, NULL); // Client order to stop application
			CEvent* ptrCommandGot = new CEvent(true, true, NULL, NULL); // A command has been entered
			CEvent* ptrSendPassword = new CEvent(true, true, NULL, NULL);// question : true, true choisis de manière arbitraire -> surement à modifier

																		// This variable indicate if the downloading has completed.
			BOOL ptrStartOK = false; // question : valeur choisie arbitrairement -> à vérifier
			BOOL downloadingCompleted = false; //question : utile pour la création d'un clientSocket (paramètre) -> à modifier
			BOOL ptrConnected;
			BOOL sendConnectionNotAccepted;
			HANDLE* ptrFile;// question: qu'est-ce que c'est ? A quoi ça sert ?
			TCHAR CommandBuf[81];

			// ClientSocket creation and connection.
			ClientSocket* ptrClientSocket = new ClientSocket(ptrStopEvent, &downloadingCompleted); // Socket to communicate with the server
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

			//ReceivingThread::ReceivingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, CEvent* ptrSendPassword, BOOL* ptrStartOK, BOOL* ptrConnected, BOOL* sendConnectionNotAccepted, HANDLE* ptrFile, TCHAR* CommandBuf) :

			// Thread creation and starting.
			ReadingKeyboardThread* ptrReadingKeyboardThread = new ReadingKeyboardThread(ptrStopEvent, ptrCommandGot, CommandBuf); // Thread to received the "exit" command throw the terminal.//CommandBuf où es-tu ?
			ptrReadingKeyboardThread->CreateThread(); // Call Run.
			ReceivingThread* ptrReceivingThread = new ReceivingThread(ptrClientSocket, ptrDataRecvEvent, ptrDataSentEvent, ptrStopEvent, ptrSendPassword, &ptrStartOK, &ptrConnected, &sendConnectionNotAccepted, ptrFile, CommandBuf); // Thread to receive data from the server.
			ptrReceivingThread->CreateThread(); // Call Run.
			SendingThread* ptrSendingThread = new SendingThread(ptrClientSocket, ptrDataRecvEvent, ptrDataSentEvent, ptrStopEvent, ptrCommandGot, &downloadingCompleted); // DownloadingCompleted, qu'est-ce qu'il fait là ? | avant:Thread to send data to the server. 
			ptrSendingThread->CreateThread(); // Call Run.

			CSingleLock* ptrStopEventLock = new CSingleLock(ptrStopEvent); // Wait stopEvent.

																		   // Application closing

			ptrStopEventLock->Lock(INFINITE);
			Sleep(5000);
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





