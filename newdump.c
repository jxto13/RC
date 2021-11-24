#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "byteStuffing.h"

#define DATASIZE 512 //Testing data size
#define SIZE 128

int getSize_Uchar(unsigned char* src){
    unsigned char* pointer = src;
    int counter = 0;
    while(*pointer != '\0'){
        counter++;
        pointer++;
    }
    return counter;
}

void printer(unsigned char * src){
    unsigned char* start = src;
        
    while(*start != '\0'){
        printf("%x ",*start);
        start++;

    }
    printf("\n");
}

unsigned char* data_package_gen(unsigned char* data, int length){
  unsigned char* data_package = malloc(DATASIZE + 4);
  memcpy(data_package,(unsigned char[]) {0x01},1);
  memcpy(data_package+1,(unsigned char[]) {DATASIZE%256},1);
  memcpy(data_package+2,(unsigned char[]) {DATASIZE/256},1);
  if(length < 256){
    memcpy(data_package+3,(unsigned char[]) {length},1);
    memcpy(data_package+4,data,length);
  } 
  else{
    memcpy(data_package+3,(unsigned char[]) {0x00},1);
    memcpy(data_package+4,data,DATASIZE);
  } 
  return data_package;
}

double log2(double x){
  return log(x) / log(2);
}

int n_bytes(int data){
  return (int) ceil((log2(data)/8.0));
}

char* intToHex(int n, int size){
  char* hex = malloc(size);
  char* hex_out = malloc(size+((size/2)*2));

  sprintf(hex, "%x", n);

  int counter = 0;
  for (int i = 0; i <= size+((size/2)*2); i+=(size)){
    hex_out[i] = '\\';
    hex_out[i+1] = 'x';
    hex_out[i+2] = hex[counter++];
    hex_out[i+3] = hex[counter++];
    // printf("%c%c%c%c\n",hex_out[i],hex_out[i+1],hex_out[i+2],hex_out[i+3]);
  }
  

  return hex_out;
}


unsigned char* control_data_package(unsigned char* file_name ,int file_length, int control){
  int counter = 0, file_length_size = n_bytes(file_length);
  char* file_length_HEX = malloc(file_length);
  file_length_HEX = intToHex(file_length,file_length_size);

  unsigned char* control_data_package = malloc(getSize_Uchar(file_name) + file_length_size + 4);
  if(control == 1){ //START
    memcpy(control_data_package,(unsigned char[]) {0x02},1); // C
    counter++;
  }
  else{ //END
    memcpy(control_data_package,(unsigned char[]) {0x03},1); // C
  }

  memcpy(control_data_package+counter++,(unsigned char[]) {0x00},1); // T1
  memcpy(control_data_package+counter++,(unsigned char[]) {file_length_size},1); // L1

  char * test = {"\x2a\x8d"};
  memcpy(control_data_package+counter,test,2); // V1

  memcpy(control_data_package+(counter += file_length_size),(unsigned char[]) {0x01},1); // T2
  memcpy(control_data_package+counter++,(unsigned char[]) {getSize_Uchar(file_name)},1); // L2
  memcpy(control_data_package+counter,file_name,getSize_Uchar(file_name)); // V2
  return control_data_package;
}

// unsigned char* control_data_package(unsigned char* file_name ,int file_length, int control){
//   int size = getSize_Uchar(file_name);
//   int counter = 0;
//   unsigned char* control_data_package = malloc(size + sizeof(file_length) + 5);
//   if(control == 1){ //START
//     // control_data_package[0] = 0x02;
//     memcpy(control_data_package,(unsigned char[]) {0x02},1); // C
//     counter++;
//   }
//   else{ //END
//     // control_data_package[0] = 0x03;

//     memcpy(control_data_package,(unsigned char[]) {0x03},1); // C
//     counter++;

//   }
//     // control_data_package[0] = 0x00;
//     // control_data_package[1] = sizeof(file_length);
//     // control_data_package[2] = file_length;

//     memcpy(control_data_package+counter,(unsigned char[]) {0x00},1); // T1
//     counter++;

//     memcpy(control_data_package+counter,(unsigned char[]) {0x02},1); // L1
//     counter++;


// /* 
//     char* hexLength = malloc(0);
//     DecToHexStr(file_length,&hexLength);
//     for (int i = 0; i < sizeof(file_length); i++){
//         printf("%x ",hexLength[i]);
//     }
//     printf("\n");
    
//     char array[8] = { 0x32, 0x61 ,0x64, 0x38 };

//     char arr[SIZE] = {0};
//     int i = 0;
//     unsigned long long res = 0;
//     for (i = 0; i < 8; i++)
//         sprintf((arr + (i * 2)), "%2x", (array[i] & 0xff));

//     printf("arr is %s\n", arr);

//     res = strtoll(arr, NULL, 16);
//     printf("res is %llu\n", res);
//  */
//     printf("%x\n",file_length);
//     unsigned char* file_lengthHEX = (unsigned char*) file_length;
    
//     printer(file_lengthHEX);
//     memcpy(control_data_package+counter,(unsigned char[]) {file_length},2); // V1
//     counter += 2;
//     memcpy(control_data_package+counter,(unsigned char[]) {0x01},1); // T2S
//     counter++;
//     memcpy(control_data_package+8,(unsigned char[]) {size},1); // L2
//     counter++;
//     memcpy(control_data_package+9,file_name,size); // V2
//     counter += size;

//   return control_data_package;
// }
int main(){ 

      
     
    unsigned char* fileName = (unsigned char*) "pinguim.gif";
    int file_length = 10968;
    int length = getSize_Uchar(fileName) + n_bytes(file_length) + 4;

    unsigned char* test = control_data_package(fileName,file_length,1);
    // printer(test);
    for (int i = 0; i < length; i++) {
        printf("%x ",test[i]);

    }
        printf("\n"); 

      
    int test32 = 10968 ;
      int* ptest32 = &test32;

      char* p1st8bits = (char*) ptest32;
      printf("%c\n",p1st8bits);
    // sup = intToHex(file_length,length);

    // for (int i = 0; i < length+4; i++){
    //     printf("%c",sup[i]);
    //   /* code */
    // }
    
    // char hex[] = "2ad8";                          // here is the hex string
    // int num = (int)strtol(hex, NULL, 16);       // number base 16

    // printf("%d\n", num);                        // print it as decimal
    // printf("%x\n", num);                        // print it back as hex

    return 0;
}