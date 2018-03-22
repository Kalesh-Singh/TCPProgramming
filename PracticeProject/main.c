#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

int asciiNumToDecimal(uint8_t amountBuffer[], uint32_t size) {
    int decimalValue = 0;
    int i;
    int placeValue = 1;
    for (i = size - 1; i >= 0; --i) {
        decimalValue += (amountBuffer[i] - 48) * placeValue;
        placeValue *= 10;
    }
    return decimalValue;
}

uint8_t* getTypeOneNum(uint8_t* number, FILE* fp, long offset) {
    uint8_t counter = 0;
    uint8_t currByte;
    while (1) {
        currByte = (uint8_t) fgetc(fp);
        if (currByte == ',' || currByte == 0 || currByte == 1) {
            fseek(fp, -(counter + 1), SEEK_CUR);
            break;
        }
        counter++;
    }

    int i;

    for (i = 0; i < 5; ++i) {
        if (i < (5 - counter))
            number[i] = 0;
        else
            number[i] = (uint8_t) getc(fp);
    }

    return number;
}
 

int main() {
    // Open the file
    FILE* fp = fopen("practice_project_test_file_1", "rb");
    //assert(fp);

    // Get the size of the File in bytes
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    printf("File Size: %lu\n", fileSize);

    // Go back to the beginning of the file
    rewind(fp);
    printf("Current Offset: %lu\n", ftell(fp));

    // int i = 0;
    while (ftell(fp) < fileSize - 1) {
		//++i;
        // printf("%d. %d\n", i, fgetc(fp));

        // Read the first byte of the file to determine the Type of the Unit
        uint8_t type = (uint8_t) fgetc(fp);
        printf("Type: %d\t", type);

        if (type == 0) {
            // Get the amount in the unit
            uint8_t amount = (uint8_t) fgetc(fp);
            printf("Amount: %d\t", amount);

            // Get the Numbers in the Type 0 Unit
            uint16_t typeZeroBuffer[amount];
            fread(typeZeroBuffer, sizeof(uint16_t), amount, fp);

            // Print the Numbers to the Screen
            int i;
            for (i = 0; i < (sizeof(typeZeroBuffer) / sizeof(uint16_t)); ++i) {
                printf("%d ", typeZeroBuffer[i]);
            }
			printf("\n");

        } else if (type == 1) {
            // Get the amount in the unit
            uint8_t amountBuffer[3];
            fread(amountBuffer, sizeof(uint8_t), 3, fp);

            // Convert the amountBuffer to a integer value
            uint16_t amount = (uint16_t) asciiNumToDecimal(amountBuffer, 3);
			printf("Amount: %d\t", amount);

			// Get the Size of the unit in bytes
			long unitStartIndex = ftell(fp);
			int unitSize = 0;
			uint8_t currByte;
			while (1) {
				currByte = fgetc(fp);
				if(currByte == 0 || currByte == 1) {
					fseek(fp, -1, SEEK_CUR);
					break;
				}
				unitSize++;
			}

			// printf("Number of bytes in Type 1 unit = %d\n", unitSize);

			// Reset the offset to the start of the unit
			fseek(fp, unitStartIndex, SEEK_SET);

			// Populate the buffer
			uint8_t typeOneBuffer[unitSize];
			int i;
			for (i = 0; i < unitSize; ++i) {
				typeOneBuffer[i] = fgetc(fp);
			}

			// Print the values in the buffer
			for (i = 0; i < unitSize; ++i) {
				printf("%c", typeOneBuffer[i]);
			}
			printf("\n");
/*
            // Get the Numbers in the Type 1 Unit
            uint8_t numberBuffer[amount][5];   // Buffer to store all the numbers

            long offset = ftell(fp);
            int i;
            uint8_t currByte;
            for (i = 0;i < amount; ++i) {
                getTypeOneNum(numberBuffer[i], fp, offset);
                currByte = fgetc(fp);
                if (currByte == 0 || currByte == 1) {
                    fseek(fp, -1, SEEK_CUR);
                    break;
                } else if (currByte == ',') {
                    break;
                }
            }
*/

            // Print the Numbers

else {
	print()
}

        }

    } 



    return 0;
}
