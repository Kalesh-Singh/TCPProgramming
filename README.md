# The Application Design #
The TCP Application Protocol is used to send a file containing Units (as described below) from the client to server. The server receives the file and if it is correctly formatted it saves the file making any necessary conversions between units as specified by the client. The server sends a response to the client indicating whether the operation was successuful or not.

### File Format ###
The content of the input file is a sequence of units. Each unit has one of the following two formats.
+ ```Type``` ```Amount``` ```Number1 Number2 ... NumberN```
+ ```Type``` ```Amount``` ```Number1, Number2, ... , NumberN```
```Type``` is one byte with the binary value 0 or 1. The first format always has ```Type``` as 0, and the second always has ```Type``` as 1.
#### Type 0 Units ####
+ If ```Type``` is 0, the ```Amount``` is one byte. The binary value is the amount of numbers in the unit.
+ ```Number1``` through ```NumberN``` are the binary numbers, each taking 2 bytes.
#### Type 1 Units ####
+ If ```Type``` is 1, the ```Amount``` is 3 bytes. The three ASCII characters shows is the amount of numbers in the unit. 
+ ```Number1``` through ```NumberN``` are unsigned integers no more than 65535 represented in
ASCII, separated by comma. There is no comma after the last number.

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
After inidcating that the options were correctly received, the server continuously receives data from the client until it has received a total amount of bytes equal to that specified by ```fileSize``` in the received options. If the file is correctly formatted the server performs the specified operations and responds with a byte containing the value 0 to the server. Else it responds with a negative number depending on the type of format error detected.
