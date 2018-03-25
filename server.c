// Server
#include <sys/socket.h>			/* for socket(), bind(), connect(), send(), and revc() */
#include <arpa/inet.h>			/* for sockaddr_in and inet_ntoa() */
#include <unistd.h>				/* for close */
#include "helper.h"				/* for server helper functions */

#define LISTENQ (5)				/* Backlog for listen() */

void DieWithError(char *errorMessage) {
	perror (errorMessage) ;
	exit(1);
}

int main(int argc, char* argv[]) {
	// Parse the command line arguments
	if (argc < 2 || argc > 2) 	{
		printf("\nIncorrect Number of Arguments\n\n");
		printf("Usage:");
		printf("\t%s [port number]\n\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	int temp = atoi(argv[1]);

	if (temp < 0 || temp > 65535) {
		printf("\nInvalid Port Number: ");
		printf("Port number range is 0 to 65535\n\n");
		exit(EXIT_SUCCESS);
	}
	unsigned short serverPort = temp;

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
			unsigned char toNameSize;
			memcpy(&toNameSize, fileBuffer + 1, 1);
			printf("toNameSize: %d\n", toNameSize);
			char toName[toNameSize + 1];
			memcpy(toName, fileBuffer + 2, toNameSize);
			toName[toNameSize] = '\0';
			printf("%s\n", toName);	
			printf("Data to be written: %s\n", fileBuffer + toNameSize + 2);
		
			// ---------------------------------------------------------------------------------
			// Create IN file to be passed to practice project then delete it 
			FILE* tempIn = fopen(".tempFile", "wb");
			if (tempIn == NULL) {
				perror("Failed to open file");
				return -1;
			}
		
			// Write the data to the file
			fwrite(fileBuffer + toNameSize + 2, 1, receivedBytes - (toNameSize + 2), tempIn);
			printf("Created temp file for processing...\n");

			// Close the file
			fclose(tempIn);
			//----------------------------------------------------------------------------------
			tempIn = fopen(".tempFile", "rb");
			if (tempIn == NULL) {
				perror("Failed to open IN file");
				return -1;
			}
			
			FILE* out = fopen(toName, "wb+");		// "wb+" because we also need to read to display the written data
			if (out == NULL) {
				perror("Failed to open OUT file");
				return -1;
			}

			// TODO: Pass tempIn, out and toFormat to Practice Project
			// Get File status
			int writeStatus = writeUnits(tempIn, out, toFormat);

			// Close the IN file
			fclose(tempIn);
			
			// Close the OUT file
			fclose(out);
			
			// Delete the .temp file
			int removeStatus = remove(".tempFile");
			printf("Remove status = %d\n", removeStatus);
			
			if (removeStatus == 0)
				printf("Successfully deleted temp file\n");
			else
				printf("Failed to delete file\n");
			//--------------------------------------------------------------------------------

			if (writeStatus < 0) {
				// Delete the partially written file
				removeStatus = remove(toName);
				printf("Remove status = %d\n", removeStatus);
			
				if (removeStatus == 0)
					printf("Successfully deleted file that was partially created\n");
				else
					printf("Failed to delete created file\n");
			} else {
				printf("File written on server...\n");	
			}	

			// Create the server response
			char response = (char) writeStatus;

			// Send the response to the client
			if (send(clientSocket, &response, 1, 0) != 1)
				DieWithError("send() failed");
			printf("Sent response to client ... \n");
		}
		
		// Close the client socket
		close(clientSocket);
	}	
	
	return 0;
}
