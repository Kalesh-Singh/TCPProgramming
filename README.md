# The Application Design #
The TCP Application Protocol is used to send a file containing Units (as described below) from the client to server. The server receives the file and if it is correctly formatted it saves the file making any necessary conversions between units as specified by the client. The server sends a response to the client indicating whether the operation was successuful or not.

## Protocol ##

### Message Types for Client ###

#### Request ####
This request send all of the client specified options to the server. These include ```toFormat```, ```toNameSize```, ```toName```, and ```fileSize```.

#### Response ####
Upon receiving a response from the server. The client compares the received integer to the size of the options sent. If the two values are the same, the client response by sending the file containing the units to the server. The file is sent in chunks of 1 KB. Except for the last chunk which may be less. If the integer received is not the same size as the opions sent, the client throws an error an terminates.

### Message Types for Server ###
The server sends 2 response to the client.

#### Response 1 ####
Upon receiving the options from the client the server send a response to client to indicate wheteher the options were correctly received or not. This is done by sending an integer value which is the amount of bytes received by the server.

#### Response 2 ####
After inidcating that the options were correctly received, the server continuously receives data from the client until it has received a total amount of bytes equal to that specified by ```fileSize``` in the received options.
