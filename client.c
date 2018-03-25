// Client 

#include <stdio.h>				/* for printf() nad fprintf() */
#include <stdlib.h>				/* for atoi() */

#include <sys/socket.h>			/* for socket(), connect(), send(), and revc() */
#include <arpa/inet.h>			/* for sockaddr_in and inet_addr() */
#include <string.h>				/* for memset */
#include <unistd.h>				/* for close */

/*
USAGE:
<client> <server IP> <server port> <file path> <to format> <to name>
*/

void DieWithError(char *errorMessage) {
	perror (errorMessage) ;
	exit(1);
}

int main(int argc, char* argv[]) {
	char* serverIP;			/* Server IP (dotted quad) */
	unsigned short serverPort;	/* Server Port */
	char* filePath;	
	char toFormat;
	char* toName;
	char toNameSize;

	// Parsing the command line arguments
	serverIP = argv[1];
	printf("Server IP: %s\n", serverIP); 
	serverPort = atoi(argv[2]);
	printf("Server Port: %d\n", serverPort);
	filePath = argv[3];
	printf("File Path: %s\n", filePath);
	toFormat = atoi(argv[4]);
	printf("To Format: %d\n", toFormat);
	toName = argv[5];
	printf("To Name : %s\n", toName);
	toNameSize = strlen(toName);
	printf("Size of toName: %d\n", toNameSize);
	

	// Try to open the file at file path 
	FILE* in = fopen(filePath, "rb");
	if (in == NULL) {
		perror("Failed to open file");
		return -1;
	}

	// Get the size of the file in bytes
	fseek(in, 0L, SEEK_END);
	long fileSize = ftell(in);
	printf("File Size: %lu\n", fileSize);
	rewind(in);
	
	// Caculate the total bytes to send
	int bytesToSend = 1 + 1 + toNameSize + fileSize;

	// Create a write buffer
	char fileBuffer[bytesToSend];

	// Populate the buffer with the data from the file
	memcpy(fileBuffer, &toFormat, 1);			// Place toFormat in the fileBuffer
	memcpy(fileBuffer + 1, &toNameSize, 1);		// Place toNameSize in the fileBuffer
	memcpy(fileBuffer + 2, toName, toNameSize);	// Place toName in the fileBuffer
	fread(fileBuffer + 2 + toNameSize, 1, fileSize, in);	// Place file contents in file buffer

	printf("------------------------------------------------------\n");
	printf("\nFile Data\n\n");
	int i;
	for (i = 0; i < bytesToSend; ++i) {
		printf("%c", fileBuffer[i]);
	}
	printf("\n");
	printf("------------------------------------------------------\n");
	
	// Close the file
	fclose(in);

	int sock;									/* Socket descriptor */
	struct sockaddr_in serverAddress;			/* Server address */

	// Create a reliable, stream socket using TCP 
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)	{
		DieWithError("socket() failed");
	}

	// Construct the server address structure
	memset(&serverAddress, 0, sizeof(serverAddress));		/* Zero out struct */
	serverAddress.sin_family = AF_INET;						/* Internet address family */
	serverAddress.sin_addr.s_addr = inet_addr(serverIP);	/* Server IP address */
	serverAddress.sin_port = htons(serverPort);				/* Server port */

	// Establish connection to the server
	if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
		DieWithError("connect() failed");
	}
	printf("Connection with server established ... \n");
		
	// Send file to server
	if (send(sock, fileBuffer, bytesToSend, 0) != bytesToSend)
		DieWithError("send() sent a different number of bytes than expected");
	printf("Sent file to server ...\n");

	// Wait for response from server
	char serverResponse;
	int bytesReceived = 0;
	while (bytesReceived == 0) {
		if((bytesReceived = recv(sock, &serverResponse, 1, 0))<= 0)
			DieWithError("recv() failed or connection closed permanently");
		printf("Received Response from server ... \n");
	}

	// Print the response from the server
	if (serverResponse == 0)
		printf("Success\n");
	else if (serverResponse < 0)
		printf("Format error\n");

	// Close the Socket
	close(sock);

	return 0;
	
}

