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

#define TIMEOUT 3

volatile int STOP=FALSE;

int flag=1, conta=1, UA_flag = 0, stopLoop = 1, read_buffer = 0;


void atende() {                   // atende alarme
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}



int main(int argc, char** argv) {

    int fd, res;
    struct termios oldtio, newtio;
    unsigned char buf[255];
    
    if (argc < 2) {
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
    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */
    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

  // ---------------------------------------------------------------------------

    printf("Emissor mode:\n");

    char message[10] = {0x7E,0x03,0x07,0x04,0x00,0x7E, 0x03, 0x03, 0x00, 0x7E};
    
    printf("Sending SET message\n");
    res = write(fd,message,10);
    printf("bytes written - %d\n", res);

    printf("Waiting response:\n");

    (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

    while(conta < 4){
      if(UA_flag == 1) break;

      if(flag){
          alarm(3);
          flag=0;
          printf("flag %d | conta %d\n",flag,conta);
      }
      if((res = read(fd,buf,255)) > 0){
        if(state_conf_UA(buf,res) == 1){
          UA_flag = 1;
          printf("UA message recieved\n");
          break;
        } 
      }
    } 

    printf("Vou terminar.\n");

    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}