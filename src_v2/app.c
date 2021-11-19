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
    if (app.fileDescriptor < 0){
        printf("Unable to open port\n");
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
   

    llopen(app);
    
    if (app.status == 1) {
        // llwrite(); // chamar llwrite
    }else{
        // llread(); // chamar llread()

    }

    llclose(app);


    return 0;
}