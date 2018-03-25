#include <sys/socket.h>		/* socket definitions */	
#include <sys/types.h>		/* socket types */
#include <arpa/inet.h>		/* inet (3) functions */
#include <unistd.h>			/* misc. UNIX functions */

#include "helper.h"			/* Our own helper functions */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
USAGE:

...$ ./echoserv 3357 &
...$ ./echoclnt -a 127.0.0.1 -p 3357
Enter the string to echo: #include Echo me, please.
Echo response: Echo me, please.

...$

*/

/* Global constants */

#define MAX_LINE (1000)


/* Function declarations */

int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort);

/* main */

int main(int argc, char *argv[]) {

	int conn_s;						/* connection socket */
	short int port;					/* port number */
	struct sockaddr_in servaddr;	/* socket address structure */
	char buffer[MAX_LINE];			/* char buffer */
	char *szAddress;				/* Holds remote IP address */
	char *szPort;					/* Holds remote port */
	char *endptr;					/* for strtol */

	/* Get command line arguments */

	ParseCmdLine(argc, argv, &szAddress, &szPort);

	/* Set the remote port */

	port = strtol(szPort, &endptr, 0);

	if ( *endptr ) {
		printf("ECHOCLNT: Invalid port supplied.\n");
		exit(EXIT_FAILURE);
	}

	/* Create the listening socket */

	if ( (conn_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		fprintf(stderr, "ECHOCLNT: Error creating listening socket.\n");
		exit(EXIT_FAILURE);
	}

	/* Set all bytes in socket address structure to
	   zero, and fill in the relevant data members. */

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);


	/* Set the remote IP address */

	if ( inet_aton(szAddress, &servaddr.sin_addr) <= 0 ) {
		printf("ECHOCLNT: Invalid remote IP address.\n");
		exit(EXIT_FAILURE);
	}

	/* connect() to the remote echo server */

	if ( connect(conn_s, (struct sockaddr*) &servaddr, sizeof(servaddr) ) < 0 ) {
		printf("ECHOCLNT: Error calling connect()\n");
		exit(EXIT_FAILURE);
	}

	/* Get string to echo from user */
	
	// printf("Enter the string to echo: ");
	// fgets(buffer, MAX_LINE, stdin);

	//TODO: /* Get the file to send */
/*
	char fileName[1000];
	printf("Enter the path to the file to send: ");
	fgets(fileName, MAX_LINE, stdin);
	printf("Filename = %s\n", fileName);
*/

	FILE* in = fopen("test_file_1", "rb");
	if (in == NULL) {
		perror("Failed to open file: ");
		return -1;
	}

	// Get the sile size
	fseek(in, 0, SEEK_END);
	long fileSize = ftell(in);
	rewind(in);
	

	/* Send string to echo server, and retrieve response */

	Writeline(conn_s, buffer, fileSize);
	printf("Sent the file data\n");
	Readline(conn_s, buffer, MAX_LINE-1);


	/* Output echoed string */

	printf("Echoed response: %s\n", buffer);

	return EXIT_SUCCESS;
}

int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort) {

	int n = 1;

	while ( n < argc ) {
		if ( !strncmp(argv[n], "-a", 2) || !strncmp(argv[n], "-A", 2) ) {
			*szAddress = argv[++n];
		} else if ( !strncmp(argv[n], "-p", 2) || !strncmp(argv[n], "-P", 2) ) {
			*szPort = argv[++n];
		} else if ( !strncmp(argv[n], "-h", 2) || !strncmp(argv[n], "-H", 2) ) {
			printf("Usage:\n\n");
			printf("\ttimeclnt -a (remote IP) -p (remote port)\n\n");
			exit(EXIT_SUCCESS);
		}
		++n;
	}
}
	
