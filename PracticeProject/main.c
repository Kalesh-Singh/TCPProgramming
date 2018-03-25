#include "helper.h"

int main() {
    // Open the file to read
    FILE* in = fopen("practice_project_test_file_2", "rb");

	// Open the file to write to 
	FILE* out = fopen("output", "wb+");

	// Hard coding the toFormat Option
	char toFormat = 0;
	int writeStatus = writeUnits(in, out, toFormat);
    
	// Close the files
	fclose(in);
	fclose(out);

	if (writeStatus < 0)
		printf("\nFormat error\n");
	else
		printf("\nWrite successful\n");

    return 0;
}

