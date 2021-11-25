#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATASIZE 5
#define PACKAGE_SIZE DATASIZE + 6

int control = 0;

unsigned char BCC2(unsigned char* data){
    unsigned char* ptr = &data[0];
    unsigned char bcc = (*ptr);
    for(int i=0; i<5;i++){
      bcc = bcc^(*ptr);
      ptr++;
    }
    return bcc;
}

unsigned char* framing(unsigned char* data){  
    unsigned char* framed_data = malloc(PACKAGE_SIZE);

    memcpy(framed_data, (unsigned char[]) {0x7E}, 1); //F
    memcpy(framed_data+1, (unsigned char[]) {0x03}, 1); //A
    if(control == 0){
        memcpy(framed_data+2, (unsigned char[]) {0x00}, 1); //C
        memcpy(framed_data+3, (unsigned char[]) {(0x00^0x03)}, 1); //C
    } 
    else if(control == 1){
        memcpy(framed_data+2, (unsigned char[]) {0x01}, 1); //C
        memcpy(framed_data+3, (unsigned char[]) {(0x01^0x03)}, 1); //C
    } 
    else{
      printf("Control has an illegal value, exiting..");
      exit(0);
    }    

    memcpy(framed_data+4, data, DATASIZE); //D1 -> DN
    memcpy(framed_data+4+DATASIZE, (unsigned char[]) {BCC2(data)}, 1); //BCC2
    memcpy(framed_data+5+DATASIZE, (unsigned char[]) {0x7E} , 1); //F

    return framed_data;
}


int main(){
    unsigned char test_package[DATASIZE] = {0x23,0x25,0x27,0x29,0x31};
    unsigned char* package_ptr = &test_package[0];
    unsigned char* framed_data = framing(package_ptr);

    for (int i = 0; i < PACKAGE_SIZE; i++)
    {
      printf("%x ", framed_data[i]);
    }

    printf("\n");

    printf("%x\n",0x03^0x01);
} 