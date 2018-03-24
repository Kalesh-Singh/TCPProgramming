#include "helper.h"

long getFileSize(FILE* fp) {
	/* Return the file size; does not change the current position in the file. */
	long currPos = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	fseek(fp, currPos, SEEK_SET);
	return size;
}

uint8_t getType(FILE* fp) {
	/* Returns the Type of the Unit */
	return fgetc(fp);
}

uint8_t getT0Amount(FILE* fp) {
	/* Returns the amount of a Type 0 Unit */
	return fgetc(fp);
}

void getT1Amount(FILE* fp, char* t1Amount) {
	/* Reads the amount of a Type 1 Unit into t1Amount */
	fread(t1Amount, sizeof(char), 3, fp);
}

int validateT1Amount(char* t1Amount) {
	/* Checks that the amount is only made up of numeric ASCII digits */
	int i;
	for(i = 0; i < 3; ++i) {
		if (t1Amount[i] < 48 && t1Amount[i] > 57) {
			return -1;		// If the byte is not a valid numeric ASCII digit
		}
	}
	return 0;
}

void t0AmountTot1Amount(uint8_t t0Amount, char* t1Amount) {
	/* Converts t0Amount to its corresponding Type 1 Amount and stores it into t1Amount */ 
	int i;
	for (i = 2; i >= 0; --i) {
		t1Amount[i] = (t0Amount % 10) + 48;
		t0Amount /= 10;
	}
}

uint16_t t1AmountTot0Amount(char* t1Amount) {
	/* Returns the Type 0 equivalent of a t1Amount */
    uint16_t t0Amount = 0;
    int i;
    int placeValue = 1;
    for (i = 2; i >= 0; --i) {
        t0Amount += (t1Amount[i] - 48) * placeValue;
        placeValue *= 10;
    }
    return t0Amount;
}

void populateT0Buffer(FILE* fp, uint16_t* buffer, uint8_t amount) {
	/* Reads the Numbers of a Type 0 Unit into buffer */
	fread(buffer, sizeof(uint16_t), amount, fp);
}

void printT0Numbers(uint16_t* buffer, uint8_t amount) {
	/* Prints the Type 0 Numbers of a Unit to the screen */
	int i;
	for (i = 0; i < amount; ++i) {
		// Converting from Big to Little Endian
		uint16_t number = (buffer[i] << 8) | (buffer[i] >> 8);
		if (i == amount-1)
			printf("%d", number);
		else
			printf("%d,", number);
	}
}

int sizeOfT1Numbers(FILE* fp, long fileSize) {
	/* Returns the size in bytes of the Numbers of a Type 1 Unit */
	long currPos = ftell(fp);
	int size = 0;
	uint8_t currByte;
	int prevCommaPos = -1;
	while (1) {
		currByte = fgetc(fp);
		if(currByte == 0 || currByte == 1) {
			fseek(fp, -1, SEEK_CUR);
			break;
		}

		if ((currByte < 48 && currByte > 57) && (currByte != ','))
			return -1;	// Digit not numeric or a comma
		if (currByte == ',') {
			if ((ftell(fp) - prevCommaPos) < 2) {
				return -2;	// Comma followed by another comma
			}
		}

		size++;
		if(ftell(fp) == fileSize)
			break;
	}
	fseek(fp, currPos, SEEK_SET);
	return size;
}

void populateT1Buffer(FILE* fp, uint8_t* buffer, uint8_t amount) {
	/* Reads the Numbers of a Type 1 Unit into buffer */
	fread(buffer, sizeof(uint8_t), amount, fp);
}

void printT1Numbers(uint8_t* buffer, int unitSize) {
	/* Prints the Type 1 Numbers of a Unit to the screen */
	int i;
	for (i = 0; i < unitSize; ++i) {
		printf("%c", buffer[i]);
	}
}

void writeType0(FILE* out, uint8_t amount, uint16_t* buffer) {
	/* Writes a Type 0 Unit to the out file, given Type 0 data */

	// Set the type to 0
	uint8_t type = 0;

	// Determine the size of data to be written
	long size = 1 + 1 + (amount * 2);

	// Create a write buffer 
	uint8_t writeBuffer[size];

	// Populate the write buffer
	memcpy(writeBuffer, &type, 1);
	memcpy(writeBuffer + 1, &amount, 1);
	memcpy(writeBuffer + 2, buffer, amount * 2);


	// Write the contents of the write buffer to the file
	fwrite(writeBuffer, sizeof(uint8_t), size, out);

}

void writeType0FromType1(FILE* out, uint8_t amount, uint8_t* buffer, int unitSize) {
	/* Writes a Type 0 Unit to the out file, given Type 1 data */

	// Set the type to 0
	uint8_t type = 0;


	// Convert the Type 0 Numbers to Type 1 Numbers
	char t1Nums[amount][5];
	memset(t1Nums, 48, sizeof(char) * amount * 5);

	int pos = unitSize - 1;
	int i, j;
	int prevJ;
	for (int i = amount - 1; i >= 0; --i) {
		for (int j = 4; j >= 0; --j) {
			if (buffer[pos] == ',') {
				pos--;
				if (prevJ == 0) {
					i++;
				}
				prevJ = j;
				break;
			} else {
				t1Nums[i][j] = buffer[pos];
				pos--;
				if (pos < 0) {
					prevJ = j;
					break;
				}
			}
			prevJ = j;
		}
	}

	uint16_t t0Nums[amount];

	for (int i = 0; i < amount; ++i) {
		uint16_t t0Num = 0;
		int placeValue = 1;
		for (int j = 4; j >= 0; --j) {
			t0Num += (t1Nums[i][j] - 48) * placeValue;
			placeValue *= 10;
		}
		// Change the endianess to from Little to Big for writing to the file
		t0Nums[i] = (t0Num << 8) | (t0Num >> 8);
	}


	// Determine the size of data to be written
	long size = 1 + 1 + (amount * 2);

	// Create a write buffer 
	uint8_t writeBuffer[size];

	// Populate the write buffer
	memcpy(writeBuffer, &type, 1);
	memcpy(writeBuffer + 1, &amount, 1);
	memcpy(writeBuffer + 2, t0Nums, amount * 2);


	// Write the contents of the write buffer to the file
	fwrite(writeBuffer, sizeof(uint8_t), size, out);

}

