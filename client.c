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
	unsigned char toNameSize;

	if (argc != 6) 	{
		printf("\nIncorrect Number of Arguments\n\n");
		printf("Usage:");
		printf("\t%s [server IP] [server port number] [file path] [to_format] [to_name] \n\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	// Parsing the command line arguments
	serverIP = argv[1];
	printf("Server IP: %s\n", serverIP);

	int temp = atoi(argv[2]);
	if (temp < 0 || temp > 65535) {
		printf("\nInvalid Port Number: ");
		printf("Port number range is 0 to 65535\n\n");
		exit(EXIT_SUCCESS);
	}
	serverPort = temp;
	printf("Server Port: %d\n", serverPort);

	filePath = argv[3];
	printf("File Path: %s\n", filePath);

	toFormat = atoi(argv[4]);
	if (toFormat < 0 || toFormat > 3) {
		printf("\nInvalid value for to_format\n\n");
		printf("to_format Options:\n");
		printf("\t0 - Write units to [to_name] without performing any conversion\n");
		printf("\t1 - Convert Type 0 Units to Type 1 (leaving Type 1 units unchanged), before writing units to [to_name]\n");
		printf("\t2 - Convert Type 1 Units to Type 0 (leaving Type 0 units unchanged), before writing units to [to_name]\n");
		printf("\t3 - Convert Type 0 Units to Type 1 and Type 1 Units to Type 0, before writing units to [to_name]\n\n");
		exit(EXIT_SUCCESS);
	}
	printf("To Format: %d\n", toFormat);

	toName = argv[5];
	printf("To Name : %s\n", toName);
	temp = strlen(toName);
	printf("Temp size = %d\n", temp);
	if (temp > 256) {
		printf("\nFILE NAME TOO LONG: to_name cannot be greater than 256 characters\n\n");
		exit(EXIT_SUCCESS); 
	}
	toNameSize = temp;
	printf("Size of toName: %d\n", toNameSize);

	int sock;									/* Socket descriptor */
	struct sockaddr_in serverAddress;			/* Server address */

	// Create a reliable, stream socket using TCP 
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)	
		DieWithError("socket() failed");

	// Construct the server address structure
	memset(&serverAddress, 0, sizeof(serverAddress));		/* Zero out struct */
	serverAddress.sin_family = AF_INET;						/* Internet address family */
	serverAddress.sin_addr.s_addr = inet_addr(serverIP);	/* Server IP address */
	serverAddress.sin_port = htons(serverPort);				/* Server port */

	// Establish connection to the server
	if (connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
		DieWithError("connect() failed");
	printf("Connection with server established ... \n");

	// Try to open the file at file path 
	FILE* in = fopen(filePath, "rb");
	if (in == NULL) {
		perror("Failed to open file");
		return -1;
	}

	// Get the size of the file in bytes
	fseek(in, 0L, SEEK_END);
	unsigned long fileSize = ftell(in);
	printf("File Size: %lu\n", fileSize);
	rewind(in);

	// Create a options buffer
	char optionsBuffer[2 + toNameSize+ sizeof(long)];

	// Populate the options buffer
	memcpy(optionsBuffer, &toFormat, 1);			// Place toFormat in the optionsBuffer
	memcpy(optionsBuffer + 1, &toNameSize, 1);		// Place toNameSize in the optionsBuffer
	memcpy(optionsBuffer + 2, toName, toNameSize);	// Place toName in the optionsBuffer
	// Place fileSize in the optionsBuffer
	memcpy(optionsBuffer + 2 + toNameSize, &fileSize, sizeof(long));	

	// Send options to server
	if (send(sock, optionsBuffer, 2 + toNameSize + sizeof(long), 0) 
		!= (toNameSize + 2 + sizeof(long)))
		DieWithError("send() sent a different number of bytes than expected");
	printf("Sent options to server ...\n");

	// Wait for Options acknowledgement from server
	int bytesReceived = 0;
	int optionsSizeAck;
	while (bytesReceived == 0) {
		if((bytesReceived = recv(sock, &optionsSizeAck, sizeof(int), 0)) <= 0)
			DieWithError("recv() failed or connection closed permanently");
		printf("Received Options ACK from server ... \n");
	}

	// Check the response
	if (optionsSizeAck == (toNameSize + 2 + sizeof(long)))
		printf("Server correctly received the options\n");
	else {
		printf("ERROR SENDING OPTIONS: Server did not receive the correct options\n");
		exit(1);
	}

	// Create a buffer to buffer one byte at a time
	char buffer[1000];

	// Intialize bytes sent to 0
	unsigned long bytesSent = 0;
	unsigned long remainingBytes = fileSize;	/* Remaining bytes to send */

	// Send the file to the server
	while (remainingBytes > 0) {
		if (remainingBytes >= 1000) {
			fread(buffer, 1, 1000, in);
			if ((bytesSent = send(sock, buffer, 1000, 0)) != 1000) {
				printf("Sent bytes = %lu\n", bytesSent);
				DieWithError("send() failed");	
			}
		} else {
			fread(buffer, 1, remainingBytes, in);
			if ((bytesSent = send(sock, buffer, remainingBytes, 0)) != remainingBytes)
				DieWithError("send() failed");	
		}
		remainingBytes -= bytesSent;
	}

	printf("Sent file to server ...\n");
	
	// Close the file
	fclose(in);

	printf("Waiting for server to process file .... \n");
	// Wait for response from server
	char serverResponse;
	bytesReceived = 0;
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
	
	// Allow some time before closing the socket
	sleep(1);

	// Close the Socket
	close(sock);

	return 0;
	
}

