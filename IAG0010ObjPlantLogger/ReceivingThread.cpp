#include "stdafx.h"
#include "ReceivingThread.h"

int receivingDataToWrite;

ReceivingThread::ReceivingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, HANDLE* ptrFile) :
	ptrClientSocket(ptrClientSocket), ptrDataRecvEvent(ptrDataRecvEvent), ptrDataSentEvent(ptrDataSentEvent), ptrStopEvent(ptrStopEvent), ptrFile(ptrFile)
{
	ptrDataSentEvents[0] = ptrStopEvent;
	ptrDataSentEvents[1] = ptrDataSentEvent;
	ptrDataSentLock = new CMultiLock(ptrDataSentEvents, 2);
	firstRecv = true;
}

ReceivingThread::~ReceivingThread(void)
{
	delete ptrDataSentLock;
}

int ReceivingThread::Run(void)
{
	DWORD lockResult;
	int recvResult;

	//while (TRUE)???
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
		//if (firstRecv) { helloProcessing(); }
		// Main processing.
		if (receivingDataToWrite) {
			fileProcessing();
		}

		//else { helloProcessing(); }
	}
	return 0;
}

int ReceivingThread::writeToFile(char *dataToWrite) {
	int length = strlen(dataToWrite);
	
	if (!WriteFile(ptrFile, dataToWrite, length * sizeof(char), &nWrittenBytes, NULL)) {
		//(_tprintf(_T("Unable to write to file. Received error %d"), GetLastError());
		return 1;
	}
	/*if (ptrClientSocket->getReceivedBytes != length) {
		_tprintf(_T("Write failed, only %d bytes written\n"), ptrClientSocket->getReceivedBytes);
	}*/
	return 0;
}

void ReceivingThread::fileProcessing() {
	
	char * data = ptrClientSocket->getRecvMessage();
	// Write in file
	if (!WriteFile(*ptrFile, ptrClientSocket->getRecvMessage() + 4, *(int*)ptrClientSocket->getRecvMessage(), &nWrittenBytes, NULL)) {
		_tcout << "Unable to write into file, error " << GetLastError() << endl;
	}
	//Début des modifications
	int position = 0; //positionition in the data package

	int length;
	memcpy_s(&length, sizeof(int), data, sizeof(char *));
	position = sizeof(int); //our position
	_tcout << "Number of bytes in package: " << length;

	int channels_number; //number of channels in the data package
	memcpy_s(&channels_number, sizeof(int), data + position, sizeof(char *));
	//memcpy(&channels_number, data + position, sizeof(int));
	position = position + sizeof(int);
	_tcout << "Number of channels in package: " << channels_number;

	char dataToWrite[2048]; //data that needs to be written in the file

	time_t timeNow = time(NULL);
	struct tm t;
	localtime_s(&t, &timeNow);
	char currentTime[2048];
	sprintf_s(currentTime, "Measurements at %d-%02d-%02d %02d:%02d:%02d \n", t.tm_year + 1900, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	printf(currentTime);
	strcpy_s(dataToWrite, currentTime);	//we copy the strings about time to the data needed to be written in the file
	strcat_s(dataToWrite, "\n");			//and concatenate a \n for a proper presentation
	writeToFile(dataToWrite); //data is written to our file

	for (int i = 0; i < channels_number; i++) { //we repeat the procedure for each channel
		int pointsNumber; //number of measurement points in the active channel
		memcpy_s(&pointsNumber, sizeof(int), data + position, sizeof(char*));
		position = position + sizeof(int);
		cout << "\nNumber of measurement points for the active channel: " << pointsNumber;

		char channelName[2048]; //name of the active channel
		_memccpy(channelName, data + position, '\0', 20);
		printf("Channel name: %s\n", channelName);
		strcpy_s(dataToWrite, channelName);
		strcat_s(dataToWrite, ":\n");
		writeToFile(dataToWrite);
		position = position + strlen(channelName) + 1;

		for (int j = 0; j < pointsNumber; j++) { //we repeat the procedure for each point
			char pointName[2048]; //name of the active point
			_memccpy(pointName, data + position, '\0', 35);
			printf("  %s", pointName);
			strcpy_s(dataToWrite, pointName);
			strcat_s(dataToWrite, ": ");
			writeToFile(dataToWrite);
			position = position + strlen(pointName) + 1;

			char stringMeasurements[2048]; //data that contains numerical values

			if (!strcmp(pointName, "Input turbidity") ||
				!strcmp(pointName, "Output turbidity")) {
				double measurement;
				memcpy;
				//memcpy_s(&measurement, sizeof(double), data + position, sizeof(char *));
				memcpy_s(&measurement, sizeof(double), data + position, sizeof(double));
				//memcpy_s(&measurement, data + position, sizeof(double));
				position = position + sizeof(double);
				//_tcout << " :" << measurement << "m^3/s\n"; //Symbole 252 à afficher // %.7f 
				_tprintf(_T(": %.3f NTU\n"), measurement);
				sprintf_s(stringMeasurements, "%.3f NTU\n", measurement);
				strcpy_s(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite);
			}
			else if (!strcmp(pointName, "Level")) {
				int measurement;
				memcpy_s(&measurement, sizeof(int), data + position, sizeof(int));
				position = position + sizeof(int);
				//_tcout << " : " << measurement << "C\n"; //symbole 248 à afficher %c // format  %.6f 
				_tprintf(_T(": %d %%\n"), measurement);
				sprintf_s(stringMeasurements, "%d %%\n", measurement);
				strcpy_s(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite);
			}
			else if (!strcmp(pointName, "Output air pressure")) {
				double measurement;
				memcpy_s(&measurement, sizeof(double), data + position, sizeof(double));
				position = position + sizeof(double);
				//_tcout << " :" << measurement << " atm\n"; // %.7f
				_tprintf(_T(": %.1f atm\n"), measurement);
				sprintf_s(stringMeasurements, "%.1f atm\n", measurement);
				strcpy_s(dataToWrite, stringMeasurements);
				writeToFile(dataToWrite);
			}
		}
	}
	_tcout << "\n";
	strcpy_s(dataToWrite, "\n");
	writeToFile(dataToWrite);

	//wcscpy_s(CommandBuf, _T("Ready")); //sends the command "Ready" automatically to the server. It will break when "Break" is manually send.
	//ptrDataSentEvent->SetEvent();

	//return "";

	_TCHAR recvText[20];
	memcpy(recvText, ptrClientSocket->getRecvMessage() + 4, 40);
	ptrDataSentEvent->SetEvent();
	//_tcout << "Received message : " << recvText << endl;
}