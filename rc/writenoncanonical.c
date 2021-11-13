/*Non-Canonical Input Processing*/

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



#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int flag=1, conta=1, UA_flag = 0, stopLoop = 1, read_buffer = 0;

void atende() {                   // atende alarme
	printf("alarme # %d\n", conta);
	flag=1;
  // if(UA_flag == 1) stopLoop = 0;
  stopLoop = 0;
	conta++;
  read_buffer = 0;
}

int state_conf(unsigned char buf[]){
  int size = sizeof(buf) ;
  // printf("sizeof(buf) %ld\n",size); //perguntar pk que da um size de 8

  for (int i = 0; i < size; i++){
    if (stateM_UA(buf[i]) == 1){
      printf("UA message recived\n");
      return 1;
    } 
  }
  return 0;
}


int main(int argc, char** argv)
{
    int fd, res;
    struct termios oldtio,newtio;
    unsigned char buf[255];
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	    //  (strcmp("/dev/ttyS4", argv[1])!=0) && 
  	     (strcmp("/dev/ttyS10", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }



    printf("Emissor mode:\n");

    // char message[10] = {0x7E,0x03,0x04,0x7E,0x7E,0x03,0x03,0x00,0x7E,0x7E};
    char message[10] = {0x7E,0x03,0x07,0x04,0x00,0x7E, 0x03, 0x03, 0x00, 0x7E};
    
    
    printf("Sending SET message\n");
    res = write(fd,message,10);
    printf("bytes written - %d\n", res);


    printf("Waiting response:\n");

    (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao


    memset(buf, 0, 255);


    // while(conta < 4){
    //   // if(UA_flag == 1) break;
    //   if(flag){
    //       alarm(3);                 // activa alarme de 3s
    //       flag=0;
    //       printf("stopLoop %d | UA_flag %d\n",stopLoop,UA_flag);
    //       while (stopLoop != 0 || UA_flag != 1){
    //         res = read(fd,buf,1);
    //         buf[res] = 0;
    //         if (stateM_UA(buf[0]) == 1){
    //           UA_flag = 1;
    //           // alarm(0);
    //           printf("UA message recived\n");
    //           break;
    //         } 
    //       }
    //   }
    // }

  while(conta < 4){
      // printf("here\n");
      if(read_buffer == 0){
        // print("readed %d \n", read(fd,buf,255));
        if(state_conf(buf) == 1) break;
        read_buffer = 1;
      }
      // if(UA_flag == 1) break;
      
      // printf("inside wile flag %d | conta %d\n",flag,conta);
      // buf[res] = 0;
      // if (stateM_UA(buf[0]) == 1){
      //   UA_flag = 1;
      //         // alarm(0);
      //   printf("UA message recived\n");
      //   break;
      // } 
      if(flag){
          alarm(3);                 // activa alarme de 3s
          flag=0;
          // printf("stopLoop %d | UA_flag %d\n",stopLoop,UA_flag);
          printf("flag %d | conta %d\n",flag,conta);

            
      }
    }
    printf("Vou terminar.\n");

    // memset(buf, 0, 255);

    // while(STOP==FALSE){
    //   res = read(fd,buf,1);
    //   buf[res] = 0;
  
    //   if (stateM_UA(buf[0]) == 1){
    //       STOP=TRUE;
    //       printf("UA message recived\n");
    //   } 
    // }
    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}