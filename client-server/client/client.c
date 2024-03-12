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

	printf("Checking if server is up...\n");
	// You can't loopback external IPs without a hairpin NAT (i.e. NAT loopback)
	iResult = getaddrinfo(internalIP, "3490", &hints, &results);
	if (iResult != 0) {
		printf("Server is not up: %d\nExiting...\n", iResult);
		WSACleanup();
		return 1;
	}

	printf("Server is up! Opening a communication tunnel...\n");
	SOCKET clientSocket = INVALID_SOCKET;
	clientSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	if (clientSocket == INVALID_SOCKET) {
		printf("Can't get socket: %d\nExiting...\n", clientSocket);
		freeaddrinfo(results);
		WSACleanup();
		return 1;
	}

	printf("Tunnel opened! Directing it to server...\n");
	iResult = connect(clientSocket, results->ai_addr, (int)results->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}
	freeaddrinfo(results);
	if (clientSocket == INVALID_SOCKET) {
		printf("Can't connect to server: %d\nExiting...\n", iResult);
		WSACleanup();
		return 1;
	}

	printf("Connected to server!\n");
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
			printf("Can't send message: %d\nExiting...\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		memset(&sendBuffer, 0, sizeof(sendBuffer));
		printf("Message sent! Waiting for server response...\n");
		iResult = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (iResult > 0) {
			printf("Server says: %s\n", recvBuffer);
			memset(&recvBuffer, 0, sizeof(recvBuffer));
		}
		else if (iResult == 0) {
			printf("Server down. Exiting...\n");
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		else {
			printf("Can't receive message: %d\nExiting...\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
	} while (strcmp(sendBuffer, "exit") != 0);

	printf("Exiting...\n");
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
