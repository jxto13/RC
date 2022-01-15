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

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

int reader(int fd, unsigned char ** buffer){
    int size_to_read = 1;
    unsigned char temp[size_to_read];
    unsigned char * ans = malloc(size_to_read);

    int res, counter = 0;
    int size_of_ans = 0;
    printf("%d fd\n",fd);
    while (STOP==FALSE) {
        // printf("%d\n",res);
        if((res = read(fd,temp,size_to_read)) > 0){

            memcpy(ans+size_of_ans,temp,res);
            
            size_of_ans+=res;
            counter += res;
            
            if(realloc(ans,size_of_ans+res) == NULL){
                printf("realloc failed\n");
                exit(1); 
            }
            // for (int i = 0; i < res; i++){
            //     printf("%d - %x\n",i,temp[i]);
            // }
        } else if (res < 0){
            printf("Error occurred at read() function.\n Exiting! \n");
            return -1;
        }
        if(res == 0){
            STOP=TRUE;
        }
    }
    if(realloc(*buffer,size_of_ans+size_to_read) == NULL){
        printf("realloc failed\n");
        return -1; 
    }
    *buffer = ans;
    return counter;
}


/*
 * Read as many bytes as possible, up to buf_size bytes, from file descriptor fd
 * into buffer buf.  Return the number of bytes read, or an error code on
 * failure.
 */
int read_full(int fd, char buf[], int buf_size) {
    int total_read = 0;
    int n_read;
    while (1) {
      if ((n_read = read(fd, buf + total_read, buf_size - total_read)) > 0) {
        printf("%d n_read\n",n_read);
        total_read += n_read;
      }
      if( n_read == 0) printf("%d n_read\n",n_read);
    }
    
    // while ((n_read = read(fd, buf + total_read, buf_size - total_read)) > 0) {
    //   printf("%d n_read\n",n_read);
    //     total_read += n_read;
    // }

    return ((n_read < 0) ? n_read : total_read);        
}

int main(int argc, char **argv)
{
  int fd, res;
  struct termios oldtio, newtio;
  char buf[255];

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS11", argv[1]) != 0)))
  {
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
  // int flags = fcntl(fd, F_GETFL, 0); 
  // fcntl(fd, F_SETFL, flags | O_NONBLOCK);

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

  newtio.c_cc[VTIME] = 5; /* inter-character timer unused */
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

  printf("New termios structure set\n");
  while (STOP == FALSE) { 
    res = read(fd, buf, 255);                          /* loop for input */
    if(res > 0){
    printf("%d\n",res);
      buf[res] = 0;             /* so we can printf... */
      printf(":%s:%d\n", buf, res);
    }
    if(res == 0){
        STOP = TRUE;
    }
    // // res = read(fd, buf, 1); /* returns after 5 chars have been input */
    // if (buf[0] == 'z')
  }
  // fd_set current_sockets, ready_sockets;

  // FD_ZERO(&current_sockets);
  // FD_SET(fd, &current_sockets);

  // while (TRUE){
  //   ready_sockets = current_sockets;
  //   if(select(FD_SETSIZE, &ready_sockets, NULL,NULL,NULL) <0){
  //     perror("select error");
  //     exit(EXIT_FAILURE);
  //   }
  //   for (int i = 0; i < FD_SETSIZE; i++){
  //     if(FD_ISSET(i, &ready_sockets)){
  //       if(i == fd){
  //         while (TRUE)
  //         {
  //           /* code */
  //         }
          
  //       }
  //     }
  //   }
    
  // }
  


  // read_full(fd, &buf,255);

  // res = reader(fd,&src);
  // res = read(fd,src,1);
  // printf("%d bytes read\n"),res;
  
  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */
  // while (STOP == FALSE) { 
  //   printf("%d\n",res);
  //   res = read(fd, buf, 255);                          /* loop for input */
  //   if(res > 0){
  //     buf[res] = 0;             /* so we can printf... */
  //     printf(":%s:%d\n", buf, res);
  //   }else {
  //       STOP = TRUE;
  //   }
  //   // // res = read(fd, buf, 1); /* returns after 5 chars have been input */
  //   // if (buf[0] == 'z')
  // }

  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
