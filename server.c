// Server

#include <stdio.h>				/* for printf() nad fprintf() */
#include <stdlib.h>				/* for atoi() */

#include <sys/socket.h>			/* for socket(), bind(), connect(), send(), and revc() */
#include <arpa/inet.h>			/* for sockaddr_in and inet_ntoa() */
#include <string.h>				/* for memset */
#include <unistd.h>				/* for close */

#define LISTENQ (5)		/* Backlog for listen() */

void DieWithError(char *errorMessage) {
	perror (errorMessage) ;
	exit(1);
}

int main(int argc, char* argv[]) {
	// Parse the command line arguments
	unsigned short serverPort = atoi(argv[1]);

	int serverSocket;					/* Socket descriptor for server */
	int clientSocket;					/* Socket descriptor for client */
	struct sockaddr_in serverAddress;	/* Local address */
	struct sockaddr_in clientAddress;	/* Client address */
	unsigned int clientAddressLen;		/* Length of Client address data structure */ 

	// Create socket for incoming connections
	if((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");
		
	// Construct the local address structure
	memset(&serverAddress, 0, sizeof(serverAddress));		/* Zero out structure */
	serverAddress.sin_family = AF_INET;						/* Internet address family */
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);		/* Any incoming interface */
	serverAddress.sin_port = htons(serverPort);				/* Local port */

	// Bind to the local address
	if (bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
		DieWithError("bind() failed");

	// Mark the socket so that it will listen for incoming connections
	if (listen(serverSocket, LISTENQ) < 0)
		DieWithError("listen() failed");

	// Run Forever	
	while (1) {
		// Set the size of the in-out parameter
		clientAddressLen = sizeof(clientAddress);
	
		// Wait for a client to connect
		if((clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &clientAddressLen)) < 0)
			DieWithError("accept() failed");
		printf("Client socket is now connected to a client ... \n");

		char fileBuffer[1000 + 2 + 256];			/* Buffer to read incoming file */
		int receivedBytes = 0;						/* Size of received message */

		// Receive message from the client
		if ((receivedBytes = recv(clientSocket, fileBuffer, 1000, 0)) < 0)
			DieWithError("recv() failed");
		printf("Received file from client ... \n");

		// Print the received data
		printf("\nReceived data\n\n");
		printf("%s", fileBuffer);
		printf("\n");
		
		if (receivedBytes > 0) {
			char* buffer = NULL;
			size_t bufferSize;

			char toFormat;
			memcpy(&toFormat, fileBuffer, 1);
			printf("toFormat: %d\n", toFormat);
			char toNameSize;
			memcpy(&toNameSize, fileBuffer + 1, 1);
			printf("toNameSize: %d\n", toNameSize);
			char toName[toNameSize + 1];
			memcpy(toName, fileBuffer + 2, toNameSize);
			toName[toNameSize] = '\0';
			printf("%s\n", toName);	
			printf("Data to be written: %s\n", fileBuffer + toNameSize + 2);

			// TODO: Process received data using practice project to get file status

			int fileStatus = 0;
			char response = (char) fileStatus;

			// Send the response to the client
			if (send(clientSocket, &response, 1, 0) != 1)
				DieWithError("send() failed");
			printf("Sent response to client ... \n");
			
		
			if (fileStatus != -1) {
				// Open output file
				FILE* out = fopen(toName, "wb");
				if (out == NULL) {
					perror("Failed to open file: ");
					return -1;
				}
			
				// Write the data to the file
				fwrite(fileBuffer + toNameSize + 2, 1, receivedBytes - (toNameSize + 2), out);
				printf("File written on server...\n");

				// Close the file
				fclose(out);
			}
		}
		
		// Close the client socket
		close(clientSocket);
	}	
	
	return 0;
}
