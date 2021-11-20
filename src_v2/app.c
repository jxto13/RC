#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "stateM_lib.h"
#include "app.h"
#include "link.h"


int main(int argc, char** argv) {

    if (argc < 2) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    applicationLayer app;

    app.fileDescriptor = open(argv[1], O_RDWR | O_NOCTTY);

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
      int length;
      unsigned char* file_data;

      char* fileName = "pinguim.gif";
      fp = fopen(fileName, "rb");
      if(fp != NULL){
        fseek(fp, 0, SEEK_END);         
        length = ftell(fp);
        file_data = malloc(sizeof(char*)*length);   
        rewind(fp);             
        fread(file_data, length, 1, fp);
      } else{
        printf("Failed to open file '%s'",fileName);
      }
      

      int res = 0;
      int counter = 0;
      for (int i = 0; i < 10; i++) {
        if((res = write(app.fileDescriptor,&file_data,1)) < 0){
            printf("Error occurred at write() function.\n Exiting! \n");
            return -1;
        }
        counter += res;

        // printf("%x\n",file_data[i]);
      }
      printf("%d\n",counter);
      // sleep(10);
      fclose(fp);


    
        // llwrite(); // chamar llwrite()
    }else{ // reciever
    FILE *fp;
      char* fileName = "pinguim_transmitted.gif";
      fp = fopen(fileName, "w");

      int res = 0;

      int counter = 0;
      unsigned char buffer[255] ;
      while((res = read(app.fileDescriptor,buffer,1)) != 0){
          // printf("%d\n",res);
          fwrite(buffer,1,sizeof(buffer),fp);
          counter += res;
          memset(buffer,0,255);
      }
        // llread(); // chamar llread()
      printf("%d\n",counter);

      fclose(fp);

    }

    llclose(app);
    close(app.fileDescriptor);

    return 0;
}
