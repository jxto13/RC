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
#include "link.h"
#include "app.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define ALARM_TIMEOUT 3
#define RETRANS_MAX 3

#define VTIME_VALUE 1 // VTIME
#define VMIN_VALUE 0 // VMIN

#define TRANSMITTER 1
#define RECEIVER 0

#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

// declaracao de variaveis globais
unsigned char SET[5] = {0x7E,0x03,0x03,0x00,0x7E};
unsigned char UA[5] = {0x7E,0x03,0x07,0x04,0x7E};
// unsigned char DISC[5] = {0x7E,0x03,0x0B,0x04,0x7E};

int flag=0, conta=1;

// declaracao de structs
linkLayer driver_layer;
struct termios oldtio;

// declaracao de funcoes
void signal_handler();

///meter llwrite dentro do llopen

int llopen(applicationLayer app){

    if (app.fileDescriptor < 0){
        return -1;
    }

    driver_layer.baudRate = BAUDRATE;
    driver_layer.timeout = ALARM_TIMEOUT;
    driver_layer.numTransmissions = RETRANS_MAX;

    struct termios newtio;
    int res; // variavel auxiliar e temporaria
    unsigned char buf[MAX_SIZE]; // MAX_SIZE 255

    if ( tcgetattr(app.fileDescriptor,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = VTIME_VALUE;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = VMIN_VALUE;   /* blocking read until 5 chars received */

    tcflush(app.fileDescriptor, TCIOFLUSH);

    if ( tcsetattr(app.fileDescriptor,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    if (app.status == TRANSMITTER){
        (void) signal(SIGALRM, signal_handler);  
        if((res = write(app.fileDescriptor,SET,5)) < 0){
            printf("Error occurred at write() function.\n Exiting! \n");
            return -1;
        }
        alarm(driver_layer.timeout);

        while(conta <= driver_layer.numTransmissions){

            if(flag){
                alarm(driver_layer.timeout);
                flag=0;

                if((res = write(app.fileDescriptor,SET,5)) < 0){
                    printf("Error occurred at write() function.\n Exiting! \n");
                    return -1;
                }
            }

            if((res = read(app.fileDescriptor,buf,255)) > 0){
                if(state_conf_UA(buf,res) == 1){
                    alarm(0);
                    break;
                } 
            } 
            if(res < 0){ 
                printf("Error occurred at read() function.\n Exiting! \n");
                exit(1);
            }
        }
    }else{
        while (STOP==FALSE) {
            if((res = read(app.fileDescriptor,buf,255)) > 0){
                if(res < 0){ 
                    printf("Error occurred at read() function.\n Exiting! \n");
                    exit(1);
                }
                if(state_conf_SET(buf,res) == 1){
                    break;
                } 
            }
        }

        printf("Responding with a UA message\n");

        if((res = write(app.fileDescriptor,UA,5)) < 0){ 
            printf("Error occurred at write() function.\n Exiting! \n");
            exit(1);
        }
    }


    return app.fileDescriptor;
}

int llclose(applicationLayer app) {
    sleep(1);
    if (tcsetattr(app.fileDescriptor, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}

int llwrite(applicationLayer app, unsigned char* buffer, int length){
    int res;
    if((res = write(app.fileDescriptor,buffer,length)) < 0){ 
            printf("Error occurred at write() function.\n Exiting! \n");
            return -1;
    }
    return res;
}
int llread(applicationLayer app,unsigned char * buffer){

    int res, counter = 0;
    while (STOP==FALSE) {
        if((res = read(app.fileDescriptor,buffer,1)) < 0){
            printf("Error occurred at read() function.\n Exiting! \n");
            return -1;
        }
        counter += res;
        if (res==0) STOP=TRUE;
    }

    return counter;


}
void signal_handler() {
    printf("No valid message recieved! Resending message... %d\\%d\n",conta,ALARM_TIMEOUT);
	flag=1;
	conta++;
}
