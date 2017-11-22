#include "stdafx.h"
#include "ReceivingThread.h"

HANDLE file; //File where we will write the received data
TCHAR * output_file = L"output.txt"; //name and/or path of the txt file to be written

int ReceivingThread::Run() {

	WSABUF DataBuf; // Buffer for received data is a structure
	char ArrayInBuf[2048];
	DataBuf.buf = &ArrayInBuf[0];
	DataBuf.len = 2048;
	DWORD nReceivedBytes = 0; // Pointer to the number, in bytes, of data received by this call
	DWORD ReceiveFlags = 0; // Pointer to flags used to modify the behaviour of the WSARecv function call
	HANDLE NetEvents[2];
	NetEvents[0] = ptrStopEvent;
	WSAOVERLAPPED Overlapped;
	memset(&Overlapped, 0, sizeof(Overlapped));
	Overlapped.hEvent = NetEvents[1] = WSACreateEvent();
	DWORD Result, Error;
	wchar_t *identifier = L"Identify";
	wchar_t *accepted = L"Accepted";
	wchar_t *notAccepted = L"Not accepted";
	wchar_t *DataPointer;
	int n = 0;

	//
	// Initialization of the written file
	//
	DWORD nBytesToWrite, nBytesWritten = 0, nBytesToRead, nReadBytes = 0;
	BYTE *pBuffer;
	file = CreateFile(output_file, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE) //gestion des erreurs
		_tcout << "File not created, error %d" << GetLastError();


	//
	// Receiving loop
	//
	while (TRUE) {
		Result = WSARecv(ptrClientSocket->getSocket, &DataBuf, 1, &nReceivedBytes, &ReceiveFlags, &Overlapped, NULL);
		if (Result == SOCKET_ERROR) {

			if (Error = WSAGetLastError() != WSA_IO_PENDING) {// Unable to continue
				_tcout << "WSARecv() failed, error " <<  Error;
				goto out_receive;
			}

			DWORD WaitResult = WSAWaitForMultipleEvents(2, NetEvents, FALSE, WSA_INFINITE, FALSE); //wait for received data

			switch (WaitResult) {
			case WAIT_OBJECT_0:		//hStopCommandGot signaled, user wants to exit
				goto out_receive;

			case WAIT_OBJECT_0 + 1: //Overlapped.hEvent signaled, received operation is over
				WSAResetEvent(NetEvents[1]); //To be ready for the next data package

				if (WSAGetOverlappedResult(ptrClientSocket->getSocket, &Overlapped, &nReceivedBytes, FALSE, &ReceiveFlags)) {
					printf("Message received from emulator:\n");
					int i = 0;

					if (*ptrStartOK == TRUE) {
						displayAndWrite(&ArrayInBuf[0]);
					}

					else if (sendConnectionNotAccepted) {
						for (i = 4; i <= nReceivedBytes - 2; i = i + 2) {
							printf("%c", ArrayInBuf[i]);
						}
						printf("\n");

						DataPointer = (wchar_t*)(DataBuf.buf + 4);

						if (!wcscmp(DataPointer, identifier)) //We have received an identification request from the Emulator
							ptrSendPassword->SetEvent;

						if (!wcscmp(DataPointer, accepted)) //Good password
							*ptrConnected = TRUE;

						if (!wcscmp(DataPointer, notAccepted)) //wrong password
							Sleep(5);

					}
					break;
				}
				else {// Fatal problems
					_tcout << "WSAGetOverlappedResult() failed, error " <<  GetLastError();
					goto out_receive;
				}
			default: // Fatal problems
				_tcout << "WSAWaitForMultipleEvents() failed, error " <<  WSAGetLastError();
				goto out_receive;
			}
		}
		else {
			if (!nReceivedBytes) {
				_tcout << "Server has closed the connection\n";
				goto out_receive;
			}
			else {
				_tcout << "%d bytes received\n" << nReceivedBytes;
				// Here should follow the processing of received data
			}
		}
	}
out_receive:
	WSACloseEvent(NetEvents[1]);
	return 0;
}


int ReceivingThread::WriteToFile(char *dataToWrite, HANDLE file) {
	int length = strlen(dataToWrite);
	DWORD nBytesWritten;

	if (!WriteFile(file, dataToWrite, length * sizeof(char), &nBytesWritten, NULL)) {
		_tcout << "Unable to write to file. Received error %d" << GetLastError();
		return 1;
	}
	if (nBytesWritten != length) {
		_tcout << "Write failed, only %d bytes written\n" << nBytesWritten;
	}
	return 0;
}



ReceivingThread::ReceivingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, CEvent* ptrSendPassword, BOOL* ptrStartOK, BOOL* ptrConnected, BOOL* sendConnectionNotAccepted, HANDLE* ptrFile, TCHAR* CommandBuf):
	ptrClientSocket(ptrClientSocket), ptrDataRecvEvent(ptrDataRecvEvent), ptrDataSentEvent(ptrDataSentEvent), ptrStopEvent(ptrStopEvent), ptrSendPassword(ptrSendPassword), ptrStartOK(ptrStartOK), ptrConnected(ptrConnected), sendConnectionNotAccepted(sendConnectionNotAccepted), ptrFile(ptrFile), CommandBuf(CommandBuf)//format variable(variable) ne correspond peut-être qu'aux événements
{
	ptrDataSentEvents[0] = ptrStopEvent;
	ptrDataSentEvents[1] = ptrDataSentEvent;
	ptrDataSentLock = new CMultiLock(ptrDataSentEvents, 2);
	firstRecv = true;
}
/*
ReceivingThread::~ReceivingThread(void)
{
	delete ptrDataSentLock;
}

int ReceivingThread::Run(void)
{
	DWORD lockResult;
	int recvResult;

	while (TRUE)
	{


		// We wait that sending message complete
		if ((lockResult = ptrDataSentLock->Lock(INFINITE, false)) == -1) {
			_tcout << "ptrDataSentLock->lock() failed for ptrDataSentEvents" << endl;
			return 1;
		}
		if (lockResult == WAIT_OBJECT_0) // stopEvent raised.
			return 0;
		// Receive packet
		if ((recvResult = ptrClientSocket->recv()) == 1) {
			_tcout << "ptrClientSocket->recv() failed in ReceivingThread" << endl;
			return 1;
		}
		if (recvResult == 2) {
			return 0;
		}

		// Events management
		ptrDataSentEvent->ResetEvent();
		ptrDataRecvEvent->SetEvent();

		// Executed only the first time.
		if (firstRecv) { helloProcessing(); }
		// Main processing.
		else { fileProcessing(); }
	}
	return 0;
}

void ReceivingThread::helloProcessing() {
	_TCHAR recvText[20];
	memcpy(recvText, ptrClientSocket->getRecvMessage() + 4, 40);
	_tcout << "Received message : " << recvText << endl;
	firstRecv = false;
}
*/
void ReceivingThread::fileProcessing(char* data) {
	// Write in file
	if (!WriteFile(*ptrFile, ptrClientSocket->getRecvMessage() + 4, *(int*)ptrClientSocket->getRecvMessage(), &nWrittenBytes, NULL)) {
		_tcout << "Unable to write into file, error " << GetLastError() << endl;
	}
	//Début des modifications
	int position = 0; //positionition in the data package

	int length;
	memcpy(&length, data, sizeof(int)); //we specify sizeof(int) to only select the first information in the package
	position = sizeof(int); //our position
	_tcout << "Number of bytes in package: "<< length;

	int channels_number; //number of channels in the data package
	memcpy(&channels_number, data + position, sizeof(int));
	position = position + sizeof(int);
	_tcout << "Number of channels in package: " <<  channels_number;

	char dataToWrite[2048]; //data that needs to be written in the file

	time_t timeNow = time(NULL);
	struct tm *t = localtime(&timeNow);
	char currentTime[2048];
	sprintf(currentTime, "Measurements at %d-%02d-%02d %02d:%02d:%02d \n",
		t->tm_year + 1900, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	printf(currentTime);
	strcpy(dataToWrite, currentTime);	//we copy the strings about time to the data needed to be written in the file
	strcat(dataToWrite, "\n");			//and concatenate a \n for a proper presentation
	writeToFile(dataToWrite, file); //data is written to our file

	for (int i = 0; i < channels_number; i++) { //we repeat the procedure for each channel
		int pointsNumber; //number of measurement points in the active channel

		memcpy(&pointsNumber, data + position, sizeof(int));
		position = position + sizeof(int);
		cout << "\nNumber of measurement points for the active channel: " <<  pointsNumber;

		char channelName[2048]; //name of the active channel
		memccpy(channelName, data + position, '\0', 20);
		printf("Channel name: %s\n", channelName);
		strcpy(dataToWrite, channelName);
		strcat(dataToWrite, ":\n");
		writeToFile(dataToWrite, file);
		position = position + strlen(channelName) + 1;

		for (int j = 0; j < pointsNumber; j++) { //we repeat the procedure for each point
			char pointName[2048]; //name of the active point
			memccpy(pointName, data + position, '\0', 35);
			printf("  %s", pointName);
			strcpy(dataToWrite, pointName);
			strcat(dataToWrite, ": ");
			writeToFile(dataToWrite, file);
			position = position + strlen(pointName) + 1;

			char stringMeasurements[2048]; //data that contains numerical values

			if (!strcmp(pointName, "Input solution flow") ||
				!strcmp(pointName, "Output solution flow") ||
				!strcmp(pointName, "Input gas flow") ||
				!strcmp(pointName, "Input steam flow")) {
				double measurement;
				memcpy(&measurement, data + position, sizeof(double));
				position = position + sizeof(double);
				_tcout << " :"<< measurement <<"m^3/s\n"; //Symbole 252 à afficher // %.7f 
				sprintf(stringMeasurements, "%.7f m³/s\n", measurement);
				strcpy(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite, file);
			}
			else if (!strcmp(pointName, "Input solution temperature") ||
				!strcmp(pointName, "Input steam temperature")) {
				double measurement;
				memcpy(&measurement, data + position, sizeof(double));
				position = position + sizeof(double);
				_tcout <<" : "<< measurement << "C\n"; //symbole 248 à afficher %c // format  %.6f 
				sprintf(stringMeasurements, "%.6f °C\n", measurement);
				strcpy(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite, file);
			}
			else if (!strcmp(pointName, "Input solution pressure") ||
				!strcmp(pointName, "Input gas pressure")) {
				double measurement;
				memcpy(&measurement, data + position, sizeof(double));
				position = position + sizeof(double);
				_tcout << " :" << measurement << " atm\n"; // %.7f
				sprintf(stringMeasurements, "%.7f atm\n", measurement);
				strcpy(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite, file);
			}
			else if (!strcmp(pointName, "Level")) {
				int measurement;
				memcpy(&measurement, data + position, sizeof(int));
				position = position + sizeof(int);
				_tcout << " :" << measurement << " %%\n"; // %d
				sprintf(stringMeasurements, "%d %%\n", measurement);
				strcpy(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite, file);
			}
			else if (!strcmp(pointName, "Output solution conductivity")) {
				double measurement;
				memcpy(&measurement, data + position, sizeof(double));
				position = position + sizeof(double);
				_tcout << " :" << measurement << " S/m\n"; // %.2f
				sprintf(stringMeasurements, "%.2f S/m\n", measurement);
				strcpy(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite, file);
			}
			else if (!strcmp(pointName, "Output solution concentration")) {
				int measurement;
				memcpy(&measurement, data + position, sizeof(int));
				position = position + sizeof(int);
				_tcout << " :" << measurement << " %%\n"; // %d
				sprintf(stringMeasurements, "%d %%\n", measurement);
				strcpy(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite, file);
			}
		}
	}
	_tcout << "\n";
	strcpy(dataToWrite, "\n");
	writeToFile(dataToWrite, file);

	wcscpy(CommandBuf, _T("Ready")); //sends the command "Ready" automatically to the server. It will break when "Break" is manually send.
	ptrDataSentEvent->SetEvent;

	//return "";
}