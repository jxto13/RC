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
#include "byteStuffing.h"
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

// #define DATASIZE 5
// #define PACKAGE_SIZE DATASIZE + 6

volatile int STOP=FALSE;

// declaracao de variaveis globais
unsigned char SET[5] = {0x7E,0x03,0x03,0x00,0x7E};
unsigned char UA[5] = {0x7E,0x03,0x07,0x04,0x7E};

unsigned char RR_0[5] = {0x7E,0x03,0x05,0x06,0x7E};
unsigned char RR_1[5] = {0x7E,0x03,0x85,0x86,0x7E};

unsigned char REJ_0[5] = {0x7E,0x03,0x01,0x02,0x7E};
unsigned char REJ_1[5] = {0x7E,0x03,0x81,0x82,0x7E};

// unsigned char DISC[5] = {0x7E,0x03,0x0B,0x04,0x7E};

int flag=0, conta=1;
int control = 0, disconnect = 0;

// declaracao de structs
linkLayer driver_layer;
struct termios oldtio;

// declaracao de funcoes
void signal_handler();
void signal_handler_send();
unsigned char BCC2(unsigned char* data, int size);
unsigned char* framing(unsigned char* data, int size,int* framed_data_size);


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
    unsigned char buf[5]; // MAX_SIZE 5 porque so existe trama de UA e SET no llopen

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

            if((res = read(app.fileDescriptor,buf,5)) > 0){
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
            if((res = read(app.fileDescriptor,buf,5)) > 0){
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

    //reset das variaveis globais apos o llopen
    flag=0, conta=1;

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

int llread(applicationLayer app, unsigned char ** buffer){
    int size_to_read = 1;
    unsigned char temp[size_to_read];
    unsigned char * ans = malloc(size_to_read);

    int res, counter = 0;
    int size_of_ans = 0;
  
    while (STOP==FALSE) {
        if((res = read(app.fileDescriptor,temp,size_to_read)) > 0){

            memcpy(ans+size_of_ans,temp,res);
            
            size_of_ans+=res;
            counter += res;
            
            if(realloc(ans,size_of_ans+res) == NULL){
                printf("realloc failed\n");
                exit(1); 
            }

        } else if (res < 0){
            printf("Error occurred at read() function.\n Exiting! \n");
            return -1;
        } else{
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

void printer(unsigned char* src, int size){
    for (int i = 0; i < size; i++){
        printf("%02x ",src[i]);
    }
    printf("\n");
}

int send_frame(applicationLayer app, unsigned char* src, int src_size){
    (void) signal(SIGALRM, signal_handler_send);
    
    int stuff_data_size = 0, bytesWritten = 0;

    // adicionar 6 por causa dos bytes do header
    unsigned char* stuff_data = framing(src,src_size,&stuff_data_size); 

    
    bytesWritten = llwrite(app,stuff_data, stuff_data_size);
    printf("Printing trama with %d\n",bytesWritten);
    alarm(driver_layer.timeout);

    while(conta <= driver_layer.numTransmissions){

        if(flag){
            alarm(driver_layer.timeout);
            flag=0;
            //voltar a enviar apos o timeout
            bytesWritten = llwrite(app,stuff_data, stuff_data_size);
        }

        //guardar o trama que recebeu em recevied
        unsigned char * recevied = malloc(0);
        int size_recevied;
        //comparar o trama com o Ready Reciever trama tendo em atencao ao N, e dar update do N
        size_recevied = llread(app,&recevied);
        
        if(control == 0){
            if(memcmp(recevied,RR_0,size_recevied) == 1){
                alarm(0);
                control = 1;
                break;
            }
        }else{
            if(memcmp(recevied,RR_1,size_recevied) == 1){
                alarm(0);
                control = 0;
                break;
            }
        }
        
    }
    if(conta-1 == driver_layer.numTransmissions){
        return -1;
    } 


    // int framed_data_size = 0;
    // unsigned char* framed_data = framing(test_package,5+6,&framed_data_size);

    // printer(framed_data,framed_data_size);
    // printf("%d before printer\n",stuff_data_size);
    // printer(stuff_data,stuff_data_size);
    // printf("%d bytes wrote\n",llwrite(app,stuff_data,stuff_data_size));

    // llwrite(app,stuff_data,src_size+6);
    // unsigned char* response = malloc(0);
    // while(STOP==TRUE){
    //     if(llread(app,&response) )
    //     STOP=TRUE;
    // }
    return 0;
}
int recieve_frame(applicationLayer app, unsigned char** output){

    while (disconnect != 1) {
        unsigned char* temp = malloc(0);
        int res = llread(app,&temp);
        printer(temp,res);
        if(temp[2] == 0x00){
            llwrite(app,RR_0,sizeof(RR_0));
            printf("Responding with a RR_0 message\n");
        }else if (temp[2] == 0x01){
            llwrite(app,RR_1,sizeof(RR_1));
            printf("Responding with a RR_1 message\n");

        }
        disconnect = 1;
    }
    
    
}
void signal_handler() {
    printf("No valid message recieved! Resending message... %d\\%d\n",conta,ALARM_TIMEOUT);
	flag=1;
	conta++;
}

void signal_handler_send() {
    printf("No valid message recieved! Resending message... %d\\%d\n",conta,ALARM_TIMEOUT);
	flag=1;
	conta++;
}

unsigned char BCC2(unsigned char* data, int size){
    unsigned char* ptr = data;
    unsigned char bcc = (*ptr);
    ptr++;
    for(int i=0; i<size;i++){
      bcc = bcc^(*ptr);
      ptr++;
    }
    return bcc;
}

unsigned char* framing(unsigned char* data, int size, int* framed_data_size){  
    unsigned char* framed_data = malloc(size+6);

    framed_data[0] = 0x7E; //F
    framed_data[1] = 0x03; //A

    if(control == 0){
        framed_data[2] = 0x00; //C
        framed_data[3] = 0x00^0x03; //C
    } 
    else if(control == 1){
        framed_data[2] = 0x01; //C
        framed_data[3] = 0x01^0x03; //C
    } 
    else{
      printf("Control has an illegal value, exiting..");
      exit(0);
    }    
    // dar stuff do data 
    unsigned char* temp_data = malloc(0);
    int data_stuff_size = 0;
    temp_data = byte_stuff(data,size,&data_stuff_size);

    if(realloc(framed_data, size+(data_stuff_size-size)) == NULL){
        printf("realloc failed\n");
        exit(1); 
    }
    memcpy(framed_data+4, temp_data, data_stuff_size); //D1 -> DN

    // dar stuff do bcc
    unsigned char* byte_stuff_data = malloc(0);
    int byte_stuff_size = 0;
    unsigned char bcc = BCC2(data, size);
    byte_stuff_data = byte_stuff(&bcc,1,&byte_stuff_size);

    if(byte_stuff_size == 1){
        if(realloc(framed_data, size+(data_stuff_size-size)+1) == NULL){
            printf("realloc failed\n");
            exit(1); 
        }
        framed_data[4 + data_stuff_size] = bcc; //BCC2
        framed_data[5 + data_stuff_size] = 0x7E; //F
    } else {
        memcpy(framed_data + 4,byte_stuff_data,byte_stuff_size);
        framed_data[data_stuff_size + byte_stuff_size] = 0x7E; //F

    }
    
    free(temp_data);
    free(byte_stuff_data);

    *framed_data_size = size+(data_stuff_size-size)+6;

    return framed_data;
}