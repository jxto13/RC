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

#define DATASIZE 700 //Testing data size

int n_digits(int x){
    int cont = 0;
    while(x != 0){
        x = x/10;
        cont++;
    }
    return cont;
}

unsigned char* data_package_gen(unsigned char* data, int length){
  unsigned char* data_package = malloc(length + 4);
  data_package[0] = 0x01;
  data_package[1] = length%256;
  data_package[2] = length/256;
  data_package[3] = length - (int) data_package[2] * 256;

  memcpy(data_package+4,data,length);
  
  return data_package;
}

unsigned char* control_data_package(char* file_name ,int file_length, int control){
  unsigned char* control_data_package = malloc(strlen(file_name) + n_digits(file_length) + 5);
  if(control == 1){ //START
    control_data_package[0] = 0x02; // C
  }
  else{ //END
    control_data_package[0] = 0x03; // C
  }
  control_data_package[1] = 0x00; // T1

  unsigned char file_size[n_digits(file_length)];
  sprintf((char*)file_size, "%d", file_length);

  memcpy(control_data_package+2,(unsigned char[]) {n_digits(file_length)},1); // L1
  memcpy(control_data_package+3, file_size,n_digits(file_length)); // V1
  control_data_package[n_digits(file_length)+3] = 0x01; // T2
  memcpy(control_data_package+n_digits(file_length)+4,(unsigned char[]) {strlen(file_name)},1); // L2
  memcpy(control_data_package+n_digits(file_length)+5,file_name,strlen(file_name)); // V2
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
      char* fileName = "text2.gif";
      int length = openFile(&fp, fileName);

      int chunk_size = DATASIZE, bytesRead;

      printf("-------Sending tramas----------\n");

      //send control package start
      if((llwrite(app,control_data_package(fileName,length,1),strlen(fileName) + n_digits(length) + 5)) < 0){
        printf("Max retransmissions reached. Exiting...\n");
        return -1;
      }

      unsigned char* file_data = malloc(chunk_size);
    
      int current = 0, counter = 0;
      for (int i = 0; current < length; i+=chunk_size) {
        counter++;
        bytesRead = fread(file_data, 1, chunk_size, fp);
        current += bytesRead;

        int res = llwrite(app,data_package_gen(file_data,chunk_size),bytesRead+4);

        // bytesRead+4 pk bytes lidos mais 4 bytes da header
        if(res < 0){
          printf("Max retransmissions reached. Exiting...\n");
          return -1;
        }
      }
      //send control package end

      if((llwrite(app,control_data_package(fileName,length,2),strlen(fileName) + n_digits(length) + 5)) < 0){
        printf("Max retransmissions reached. Exiting...\n");
        return -1;
      }

      printf("-------Disconnect tramas----------\n"); 

      llclose_writter(app);

      printf("Sended %d tramas in total\n",counter);

      printf("-------CLosing ports----------\n"); 

      fclose(fp);

    }else{ // reciever
      FILE *fp;
      char* fileName = "pinguim_transmitted.gif";
      
      printf("-------Receiving tramas----------\n");
      
      unsigned char* received = malloc(0);
      llread(app,&received, DATASIZE, fp);


    } 

    llclose(app);
    close(app.fileDescriptor);

    return 0;
}
