#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#include "stateM_lib.h"
#include "app.h"
#include "link.h"
#include "byteStuffing.h"

#define DATASIZE 512 //Testing data size

int package_N = 0;

double log2(double x){
  return log(x) / log(2);
}

int n_bytes(int data){
  return (int) ceil((log2(data)/8.0));
}

int getSize_Uchar(unsigned char* src){
    unsigned char* pointer = src;
    int counter = 0;
    while(*pointer != '\0'){
        counter++;
        pointer++;
    }
    return counter;
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

unsigned char* control_data_package(unsigned char* file_name ,int file_length, int control){
  unsigned char* control_data_package = malloc(getSize_Uchar(file_name) + n_bytes(file_length) + 5);
  if(control == 1){ //START
    memcpy(control_data_package,(unsigned char[]) {0x02},1); // C
  }
  else{ //END
    memcpy(control_data_package,(unsigned char[]) {0x03},1); // C
  }
  memcpy(control_data_package+1,(unsigned char[]) {0x00},1); // T1
  memcpy(control_data_package+2,(unsigned char[]) {n_bytes(file_length)},1); // L1
  memcpy(control_data_package+3,(unsigned char[]) {file_length},n_bytes(file_length)); // V1
  memcpy(control_data_package+n_bytes(file_length),(unsigned char[]) {0x01},1); // T2
  memcpy(control_data_package+n_bytes(file_length)+1,(unsigned char[]) {getSize_Uchar(file_name)},1); // L2
  memcpy(control_data_package+n_bytes(file_length)+2,file_name,getSize_Uchar(file_name)); // V2
  return control_data_package;
}

int openFile(FILE** fp, char* fileName){

  int length;
  // unsigned char* file_data;


  *fp = fopen(fileName, "rb");
  if(*fp != NULL){
    fseek(*fp, 0, SEEK_END);         
    length = ftell(*fp);
    rewind(*fp);             

    // file_data = malloc(sizeof(char*)*length);   
    // rewind(fp);             
    // fread(file_data, length, 1, fp);
  } else{
    printf("Failed to open file '%s'",fileName);
  }
  return length;
}

int main(int argc, char** argv) {

    if (argc != 2) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    applicationLayer app;

    app.fileDescriptor = open(argv[1], O_RDWR | O_NOCTTY);
    if(app.fileDescriptor == -1){
      printf("Failed to open serialport\n");
      return -1;
    }

    int input = 0;
    do{
      printf("t = transmitter | r = reciever | 1 = exit program \n");
      char mode = getchar();
      getchar(); // clear the buffer which contains newline char

      switch (mode) {
        case 't': // transmitter mode
          app.status = 1;
          input = 1;
          break;

        case 'r': // reciever mode
          app.status = 0;
          input = 1;
          break;

        case '1': //exit the program
          printf("Exited program successfully!\n");
          return 0;

        default:
          printf("No valid letter was entered\n");
          break;
        }
    }while (input != 1);
   
    if (llopen(app) == -1){
      printf("Unable to open port\n");
      exit(-1);
    }
    
    if (app.status == 1) { // transmitter 
      
      FILE *fp;
      unsigned char* fileName = "pinguim.gif";
      int length = openFile(&fp, fileName);

      // unsigned char* file_data;

      // file_data = malloc(sizeof(char*)*length);   
      //rewind(fp);             
      // fread(file_data, length, 1, fp);
      // int res = 0;
      int counter = 0;
      int bytesToRead = DATASIZE, bytesRead;
      unsigned char* file_data = malloc(bytesToRead);

      for (int i = 0; i < length; i+=bytesToRead) {
        // fread(file_data, 1, 15, fp);
        bytesRead = fread(file_data, 1, bytesToRead, fp);

        int stuff_data_size = 0;
        unsigned char* stuff_data = byte_stuff(file_data,bytesToRead,&stuff_data_size);
        
        llwrite(app,stuff_data,stuff_data_size);
    }
      
      // for (int i = 0; i < 15; i+=5 ) {
      //   unsigned char* stuff_data = byte_stuff(&(file_data[i]));
      //   counter += llwrite(app,&(file_data[i]),5);

      //   // printf("%x\n",file_data[i]);
      // }
      // printf("%d\n",counter);
      // sleep(10);
      fclose(fp);

    }else{ // reciever
      FILE *fp;
      char* fileName = "pinguim_transmitted.gif";
      fp = fopen(fileName, "w");

      int counter = 0;
      unsigned char* recieved = malloc(0);
      // unsigned char* destuff_data;
      // int test = llread(app,&recieved);
      // fwrite(recieved,1,test,fp);

      int destuff_data_size = 0, current = 0;
      while (counter != 11000) {
        current = llread(app,&recieved);
        // printf("%d\n",current);
        unsigned char* destuff_data = byte_destuff(recieved,current,&destuff_data_size);
        counter += destuff_data_size;
        fwrite(destuff_data,1,destuff_data_size,fp);
        // printf("%d\n",counter);
      }
      


      fclose(fp); 

    }

    llclose(app);
    close(app.fileDescriptor);

    return 0;
}
