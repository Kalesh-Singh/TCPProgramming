#include <stdlib.h>
#include "helper.h"

int main() {
    // Open the file to read
    FILE* fp = fopen("practice_project_test_file_1", "rb");

	// Hard coding the toFormat Option
	int toFormat = 2;

	// Open the file to write to 
	FILE* ofp = fopen("output", "wb");

	// Create a write buffer 
	uint8_t writeBuffer[1000];		// File cannot be more than 1MB in either format

    // Get the size of the File in bytes
    long fileSize = getFileSize(fp);
	
    // Go back to the beginning of the file
    rewind(fp);

    while (ftell(fp) < fileSize - 1) {
        // Get the Type of the Unit
		uint8_t type = getType(fp);
        printf("Type: %d\t\t", type);

        if (type == 0) {
            // Get the amount in the unit
            uint8_t amount = getT0Amount(fp);
	
			// Convert the Type 0 Amount to a Type 1 Amount
			char t1Amount[3];
			t0AmountTot1Amount(amount, t1Amount);

			// Print the amount
			char printBuffer[4];
			memcpy(printBuffer, t1Amount, 3);
			printBuffer[3] = '\0';
            printf("Amount: %s\t", printBuffer);

            // Get the Numbers in the Type 0 Unit
            uint16_t buffer[amount];
			populateT0Buffer(fp, buffer, amount);

			// Print the Numbers to the Screen
			printT0Numbers(buffer, amount);
			printf("\n");

			if ((toFormat == 0) || (toFormat == 2)) {
				// Write the Type 0 Unit to the output file
				writeType0(ofp, amount, buffer);;
			} else if ((toFormat == 1) || (toFormat == 3)) {
				// Write Type 1 Unit to the output file
				writeType1FromType0 (ofp, amount, buffer);
			}

        } else if (type == 1) {
            // Get the amount in the unit
            char t1Amount[3];
            getT1Amount(fp, t1Amount);

			// Check for valid amount
			if (validateT1Amount(t1Amount) < 0) {
				fprintf(stderr, "INVALID AMOUNT (TYPE 1): Expects numeric ASCII characters.\n");
				exit(EXIT_FAILURE);
			}

			// Print the amount to the screen
			char printBuffer[4];
			memcpy(printBuffer, t1Amount, 3);
			printBuffer[3] = '\0';
			printf("Amount: %s\t", printBuffer);

            // Convert the t1Amount to an integer value
            uint16_t amount = (uint16_t) t1AmountTot0Amount(t1Amount);

			// Get the size of the Type 1 Unit Numbers in bytes
			int unitSize = sizeOfT1Numbers(fp, fileSize, amount);
			
			// Check for errors in the format of the numbers
			if (unitSize == -1) {
				fprintf(stderr, "TYPE 1 FORMAT ERROR: Each byte is expected to be either a numeric ASCII digit or ASCII comma.\n");
				exit(EXIT_FAILURE);
			} else if (unitSize == -2) {
				fprintf(stderr, "TYPE 1 FORMAT ERROR: Commas must be separated by at least 1 numberic ASCII digit.\n");
				exit(EXIT_FAILURE);
			}

			// Get the Numbers in Type 1 Unit
			uint8_t buffer[unitSize];
			populateT1Buffer(fp, buffer, unitSize);

			// Print the Numbers to the screen
			printT1Numbers(buffer, unitSize);
			printf("\n");

			if ((toFormat == 0) || (toFormat == 1)) {
				// Write Type 1 to the out file
				writeType1(ofp, t1Amount, buffer, unitSize);
			} else if ((toFormat == 2) || (toFormat == 3)) {
				// Write Type 0 to the out file
				writeType0FromType1(ofp, amount, buffer, unitSize);
			}

		} else {
			fprintf(stderr, "INVALID UNIT TYPE: Expects 0 or 1.\n");
			exit(EXIT_FAILURE);
		}

    } 

	// Close the files
	fclose(fp);
	fclose(ofp);

    return 0;
}
