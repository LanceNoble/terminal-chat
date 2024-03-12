// Windows Socket API
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <string.h>

int main() {
	// Ask Windows to use their Socket API
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo* results = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	printf("Setting up server...\n");
	iResult = getaddrinfo(NULL, "3490", &hints, &results);
	if (iResult != 0) {
		printf("Server is not a suitable host: %d\nExiting...\n", iResult);
		WSACleanup();
		return 1;
	}

	printf("Server is a suitable host! Creating entry point for communication...\n");
	SOCKET serverSocket = INVALID_SOCKET;
	serverSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	if (serverSocket == INVALID_SOCKET) {
		printf("Can't get socket: %d\nExiting...\n", WSAGetLastError());
		freeaddrinfo(results);
		WSACleanup();
		return 1;
	}

	printf("Entry point made! Attaching it to the server...\n");
	iResult = bind(serverSocket, results->ai_addr, (int)results->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("Can't bind: %d\nExiting...\n", WSAGetLastError());
		freeaddrinfo(results);
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(results);

	printf("Entry point attached! Opening entry point...\n");
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Can't listen: %d\nExiting...\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	printf("Entry point opened! Listening for connections...\n");
	SOCKET clientSocket = INVALID_SOCKET;
	clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		printf("There was an incoming connection request, but the server failed to accept it: %d\nExiting...\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
	}

	printf("A client successfully connected!\n");
	char sendBuffer[256];
	char recvBuffer[256];
	do {
		iResult = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (iResult > 0) {
			printf("Client says: %s\n", recvBuffer);
			memset(&recvBuffer, 0, sizeof(recvBuffer));
		}
		else {
			printf("Can't receive message: %d\nExiting...\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		printf("Response: ");
		if (fgets(sendBuffer, sizeof(sendBuffer), stdin) == NULL) {
			printf("Input error. Try again.\n");
			memset(&sendBuffer, 0, sizeof(sendBuffer));
			continue;
		}
		iResult = send(clientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
		if (iResult == SOCKET_ERROR) {
			printf("Can't send message: %d\nExiting...\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		memset(&sendBuffer, 0, sizeof(sendBuffer));
		printf("Message sent! Waiting for client response...\n");
	} while (strcmp(sendBuffer, "exit") != 0);

	printf("The client disconnected. Exiting...\n");
	closesocket(clientSocket);
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
