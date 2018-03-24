#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>


// Seeks the next byte based on the offset from the beginning of the file.
// Upon finding this byte, this function gets each bit and stores it in 
// an array.
void GetNextEightBits(char* infile, int* offset, unsigned char* bits) {
  int c;
  unsigned char buffer[1] = { 0 };
  FILE* file;
  file = fopen(infile, "r");
  
  fseek(file, *offset, SEEK_SET);
  fread(buffer, sizeof(unsigned char), sizeof(buffer)/sizeof(buffer[0]), file);
   
  for(int i = 0; i < 8; i++) {
    bits[7-i] = (buffer[0] & 1);
    buffer[0] = buffer[0] >> 1;
  }
   
  fclose(file);
  (*offset) ++;
}

// Converts a binary number to decimal by raising each index with value 1
// by 2 to the power of that index then summing the values.
int GetValueOfBits(unsigned char* bits, int size) {
  int value = 0;
  for(int i = 0; i < size; i++) {
    if(bits[i] == 1) 
      value += pow(2, (size - 1 - i));
  }
  return value;
}

// Get total size of infile.
int GetFileSize(char* infile) {
  FILE* file;
  file = fopen(infile, "r");
  fseek(file, 0L, SEEK_END);
  int file_size = ftell(file);  
  fseek(file, 0, SEEK_SET);
  fclose(file);
  return file_size;
}

// Convert three bits of ascii to decimal based on the fact that 0 is 48 in
// ASCII.
int ASCIIToDecimal(int first, int second, int third){

  int value = 0;
  value += 100 * (first-48);
  value += 10 * (second-48);
  value += 1 * (third-48);
  return value;

}

void ProcessTypeZero(int* offset, char* infile) {
  int amount = 0;
  int value = 0;
  unsigned char twobytes[16];
  unsigned char highbits[8];
  unsigned char lowbits[8];

  FILE* file;
  file = fopen(infile, "r");
  fseek(file, *offset, SEEK_SET);

  GetNextEightBits(infile, offset, highbits);

  amount = GetValueOfBits(highbits, sizeof(highbits)/sizeof(highbits[0]));
  printf("%d ", amount);

  for(int i = 0; i < amount; i++) {
    GetNextEightBits(infile, offset,highbits);
    GetNextEightBits(infile, offset, lowbits);

    int tempone = GetValueOfBits(highbits, sizeof(highbits)/sizeof(highbits[0]));
    int temptwo = GetValueOfBits(lowbits, sizeof(lowbits)/sizeof(lowbits[0]));

    memcpy(twobytes, highbits, 8 * sizeof(unsigned char));
    memcpy(twobytes + 8, lowbits, 8 * sizeof(unsigned char));

    value = GetValueOfBits(twobytes, sizeof(twobytes)/sizeof(twobytes[0]));

    printf("%d", value);

    if(i != amount-1)
          printf(",");
  }

    printf("\n");
}

void ProcessTypeOne(int* offset, char* infile) {
  
  int amount = 0;
  int one,two,three;
  int value = 0;
  unsigned char temp[8];

  FILE* file;
  file = fopen(infile, "r");
  fseek(file, *offset, SEEK_SET);
    
  int count = 0;
  bool found = 0;
  int detected = 0;
  int original_offset = 0;
  int curr = 0;

  GetNextEightBits(infile, offset, temp);
  one = GetValueOfBits(temp, sizeof(temp)/sizeof(temp[0]));

  GetNextEightBits(infile, offset, temp);
  two = GetValueOfBits(temp, sizeof(temp)/sizeof(temp[0]));

  GetNextEightBits(infile, offset, temp);
  three = GetValueOfBits(temp, sizeof(temp)/sizeof(temp[0]));

  amount = ASCIIToDecimal(one,two,three);
  printf("%d ", amount);

  while(1){

    original_offset = (*offset);

    while(!found) { 
      GetNextEightBits(infile, offset, temp);
      curr = GetValueOfBits(temp, sizeof(temp)/sizeof(temp[0]));

      if(curr == 44){
        found = true;
        detected = 1;
      }
      else if (curr == 0 || curr == 1 || curr == 10) {
        found = true;
        detected = 0;
      }
 
    }
    
    count = (*offset) - original_offset;
    (*offset) = original_offset;

    for(int i = 0; i < count-1; i++) {
      GetNextEightBits(infile, offset, temp);
      one = GetValueOfBits(temp, sizeof(temp)/sizeof(temp[0]));
      amount = ASCIIToDecimal(48,48,one);
      value += amount * pow(10,(count-2-i));
    }
     
    printf("%d", value);
    if(detected == 1)
      printf(",");

    GetNextEightBits(infile, offset, temp);
    one = GetValueOfBits(temp, sizeof(temp)/sizeof(temp[0]));
    if(detected == 0) {  
      (*offset)--; 
      printf("\n");
      break; 
    }
    value = 0;
    found = false;
  }

}

int main(int argc, char *argv[]) {

  printf("Executing %s : Reading from %s Writing to %s \n", argv[0], argv[1], argv[2]);
  FILE* file;

  int offset = 0;
  unsigned char bits[8];  
  int file_size = GetFileSize(argv[1]);

  int temp = 0;

  while(offset < file_size-1) { 
    GetNextEightBits(argv[1], &offset, bits);

    if(GetValueOfBits(bits, sizeof(bits)/sizeof(bits[0])) == 0) {
      ProcessTypeZero(&offset, argv[1]);
    } 
    else {
      ProcessTypeOne(&offset, argv[1]);
    }
  }

  return 0;
}
