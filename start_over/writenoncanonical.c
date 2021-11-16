/** 
 * Non-Canonical Input Processing
 * From https://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html by Gary Frerking and Peter Baumann
**/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int flag=1, conta=1;

void handle_input_from_source(int); 
void atende();


int main(int argc, char **argv)
{
  int fd, res;
  struct termios oldtio, newtio;
  char buf[255];

  int i;

  if (argc < 2)  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, &oldtio) == -1)
  { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }


//----------------------------------------------

  printf("New termios structure set\n");

  char message[10] = {0x7E,0x03,0x07,0x04,0x00,0x7E, 0x03, 0x03, 0x00, 0x7E};
    
  printf("Sending SET message\n");
  res = write(fd,message,10);
  printf("bytes written - %d\n", res);

  printf("Waiting response:\n");

  fd_set readfs;
  struct timeval Timeout;
  int    loop=1;    /* loop while TRUE */

	FD_ZERO(&readfs);
  FD_SET(fd, &readfs);  /* set testing for source 1 */

  (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

  while (1) {
    // set timeout value within input loop 
    Timeout.tv_usec = 0;  // milliseconds 
    Timeout.tv_sec  = 3;  // seconds  

    // res = select(FD_SETSIZE, &readfs, NULL, NULL, NULL);

    // if (res == 0) {
    //   printf("Timeout occured\n");
    //   exit(1);
    // }
    if(read(fd, &buf, 255) == 0) {
			res = select(FD_SETSIZE, &readfs, NULL, NULL, &Timeout);
			if(res == -1) {
				perror("select()"); //some error occured in select
				return -2;
			} else if(res == 0) {
				memset(fd, 0, 255);  //cleans buffer if timeout
				tcflush(fd, TCIOFLUSH);
				return -1;
			} else if(FD_ISSET(fd, &readfs)) {
				//port has data
				read(fd, &buf, 1);
			}
		}
    // if(flag){
    //   alarm(3);
    //   flag=0;
    // }
    // if(res == 0) printf("here\n");
    // if (FD_ISSET(fd, &readfs)){
    //   printf("conta\n");
    //   handle_input_from_source(fd);
    // } 

  }
    // volatile int STOP=FALSE;
  
    // while(STOP==FALSE){
    //   res = read(fd,buf,1);
    //   buf[res] = 0;
    //   printf("%x\n",buf[0]);
    //   if (stateM_UA(buf[0]) == 1){
    //       STOP=TRUE;
    //       printf("UA message recived\n");
    //   } 
    // }

//------------------------------------------------

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}

void handle_input_from_source(int fd)
{
    int res = 0, i;
    char buf[255];
  
    res = read(fd,buf,255);
    buf[res]=0;
    for (int i = 0; i < 255; i++){
      if (stateM_UA(buf[i]) == 1){
        write(STDOUT_FILENO,"UA message recevied\n",21);
      }
    }
    
    for (i = 0; i < res; i++) printf("%x", buf[i]);
    printf("\n");

}



void atende() {                   // atende alarme
  printf("alarme # %d\n",conta);
	flag=1;
	conta++;
}