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
	struct addrinfo* iterator = NULL;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	const char* externalIP = "73.119.107.1"; 
	const char* internalIP = "10.0.0.151";

	printf("Getting server info...\n");
	// You can't loopback external IPs without a hairpin NAT (i.e. NAT loopback)
	iResult = getaddrinfo(internalIP, "3490", &hints, &results);
	if (iResult != 0) {
		printf("Can't get server info: %d\n\n", iResult);
		WSACleanup();
		return 1;
	}

	printf("Server exists! Getting client socket...\n");
	SOCKET clientSocket = INVALID_SOCKET;
	clientSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	if (clientSocket == INVALID_SOCKET) {
		printf("Can't get socket: %d\n\n", iResult);
		freeaddrinfo(results);
		WSACleanup();
		return 1;
	}

	printf("Ready for comms! Connecting to server...\n");
	iResult = connect(clientSocket, results->ai_addr, (int)results->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}
	freeaddrinfo(results);
	if (clientSocket == INVALID_SOCKET) {
		printf("Can't connect to server!\n\n");
		WSACleanup();
		return 1;
	}

	printf("Connected to server!");
	char sendBuffer[256];
	char recvBuffer[256];
	do {
		printf("Say something: ");
		if (fgets(sendBuffer, sizeof(sendBuffer), stdin) == NULL) {
			printf("Input error. Try again.\n");
			memset(&sendBuffer, 0, sizeof(sendBuffer));
			continue;
		}
		iResult = send(clientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
		if (iResult == SOCKET_ERROR) {
			printf("Failed to send message. Closing connection: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		memset(&sendBuffer, 0, sizeof(sendBuffer));
		printf("Sent message! Waiting for response from server...\n");
		iResult = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (iResult > 0) {
			printf("Server says: %s\n", recvBuffer);
		}
		else if (iResult == 0) {
			printf("Server shutdown. Bye bye\n");
		}
		else {
			printf("Failed to receive message. Closing connection: %d\n", WSAGetLastError());
		}
	} while (strcmp(sendBuffer, "exit") != 0);

	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
