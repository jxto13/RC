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


#define DATASIZE 512 //Testing data size

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
  unsigned char* data_package = malloc(length + 4);
  data_package[0] = 0x01;
  data_package[1] = length%256;
  data_package[2] = length/256;
  data_package[2] = length - (length * (length/256));

  // memcpy(data_package+3,(unsigned char[]) {length},1);
  memcpy(data_package+4,data,length);
  
  return data_package;
}

unsigned char* control_data_package(unsigned char* file_name ,int file_length, int control){
  unsigned char* control_data_package = malloc(getSize_Uchar(file_name) + n_bytes(file_length) + 5);
  if(control == 1){ //START
    control_data_package[0] = 0x02; // C
  }
  else{ //END
    control_data_package[0] = 0x03; // C
  }
  control_data_package[1] = 0x00; // T1
  memcpy(control_data_package+2,(unsigned char[]) {n_bytes(file_length)},1); // L1
  memcpy(control_data_package+3,(unsigned char[]) {file_length},n_bytes(file_length)); // V1
  control_data_package[n_bytes(file_length)] = 0x01; // T2
  memcpy(control_data_package+n_bytes(file_length)+1,(unsigned char[]) {getSize_Uchar(file_name)},1); // L2
  memcpy(control_data_package+n_bytes(file_length)+2,file_name,getSize_Uchar(file_name)); // V2
  return control_data_package;
}

int openFile(FILE** fp, char* fileName){

  int length;

  *fp = fopen(fileName, "rb");
  if(*fp != NULL){
    fseek(*fp, 0, SEEK_END);         
    length = ftell(*fp);
    rewind(*fp); 
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
      char* fileName = "pinguim.gif";
      int length = openFile(&fp, fileName);
      // (void) length; //so pra tirar o warning

      int chunk_size = 420, bytesRead;

      printf("-------Sending tramas----------\n");

      // fazer free()
      unsigned char* file_data = malloc(chunk_size);
    
      int current = 0, counter = 0;
      for (int i = 0; current < length; i+=chunk_size) {
        counter++;
        bytesRead = fread(file_data, 1, chunk_size, fp);
        current += bytesRead;

        int res = send_frame(app,data_package_gen(file_data,chunk_size),bytesRead+4);

        // bytesRead+4 pk bytes lidos mais 4 bytes da header
        if(res < 0){
          printf("Max retransmissions reached. Exiting...\n");
          return -1;
        }

    }
      unsigned char DISC[5] = {0x7E,0x03,0x0B,0x04,0x7E};

      write(app.fileDescriptor,DISC,sizeof(DISC));
      counter++;
      printf("Sended %d tramas in total\n",counter);
      fclose(fp);

    }else{ // reciever
      FILE *fp;
      char* fileName = "pinguim_transmitted.gif";
      fp = fopen(fileName, "w");
      
      printf("-------Receiving tramas----------\n");
      
      unsigned char* received = malloc(0);
      recieve_frame(app,&received, DATASIZE, fp);


      // int counter = 0;


      // unsigned char* destuff_data;
      // int test = llread(app,&received);
      // llread(app,&received);
      // fwrite(received,1,test,fp);
      // printer(received,test);
      // int destuff_data_size = 0, current = 0;
      // while (counter != 65) {
      //   current = llread(app,&received);
      //   // printf("%d\n",current);
      //   // unsigned char* destuff_data = byte_destuff(received,current,&destuff_data_size);
      //   counter += current;

      //   // printf("%d\n",current);

      //   printer(received,current);
      //   // fwrite(destuff_data,1,destuff_data_size,fp);
      //   // printf("%d\n",counter);
      // }
   


      fclose(fp); 

    }

    llclose(app);
    close(app.fileDescriptor);

    return 0;
}