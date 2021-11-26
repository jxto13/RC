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
#include "stateM_data.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define ALARM_TIMEOUT 3
#define RETRANS_MAX 3

#define VTIME_VALUE_TRANSMITTER 0 // VTIME
#define VMIN_VALUE_TRANSMITTER 0 // VMIN

#define VTIME_VALUE_RECEIVER 0 // VTIME
#define VMIN_VALUE_RECEIVER 1 // VMIN

#define TRANSMITTER 1
#define RECEIVER 0

#define FALSE 0
#define TRUE 1

// #define DATASIZE 5
// #define PACKAGE_SIZE DATASIZE + 6

volatile int STOP=FALSE;

// declaracao de variaveis globais
// comando do emissor para o recetor
unsigned char SET[5] = {0x7E,0x03,0x03,0x00,0x7E};
unsigned char DISC[5] = {0x7E,0x03,0x0B,0x04,0x7E};

// resposta do recetor para o emissor
unsigned char UA_R[5] = {0x7E,0x03,0x07,0x04,0x7E};

// comando do recetor para o emissor
unsigned char RR_0[5] = {0x7E,0x01,0x05,0x06,0x7E};
unsigned char RR_1[5] = {0x7E,0x01,0x85,0x86,0x7E};

unsigned char REJ_0[5] = {0x7E,0x03,0x01,0x02,0x7E};
unsigned char REJ_1[5] = {0x7E,0x03,0x81,0x82,0x7E};

// comando do emissor para o recetor de confirmacao no disconnect
unsigned char UA_R_1[5] = {0x7E,0x01,0x07,0x04,0x7E};


int flag=0, conta=1;
int control = 0, disconnect = 0;
int  tramas_r = 0;
int stopLoop = TRUE;

// declaracao de structs
linkLayer driver_layer;
struct termios oldtio;

// declaracao de funcoes
void signal_handler();
void signal_handler_send();
void signal_handler_disc();

unsigned char BCC2(unsigned char* data, int size);
unsigned char* framing(unsigned char* data, int size,int* framed_data_size);


int llopen(applicationLayer app){

    if (app.fileDescriptor < 0){
        return -1;
    }

    driver_layer.baudRate = BAUDRATE;
    driver_layer.timeout = ALARM_TIMEOUT;
    driver_layer.numTransmissions = RETRANS_MAX;

    struct termios newtio;
    int res; // variavel auxiliar e temporaria
    unsigned char buf[5]; // MAX_SIZE 5 porque so existe trama de UA_R e SET no llopen

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
    if (app.status == TRANSMITTER){
        newtio.c_cc[VTIME]    = VTIME_VALUE_TRANSMITTER;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = VMIN_VALUE_TRANSMITTER;   /* blocking read until 5 chars received */
    } else {
        newtio.c_cc[VTIME] = VTIME_VALUE_RECEIVER;
        newtio.c_cc[VMIN] = VMIN_VALUE_RECEIVER;
    }

    

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

        printf("Responding with a UA_R message\n");

        if((res = write(app.fileDescriptor,UA_R,5)) < 0){ 
            printf("Error occurred at write() function.\n Exiting! \n");
            exit(1);
        }
    }
    //conta-1 pk no ultimo alarme e feito conta++, e entao se for o maximo de trasmissoes, return -1 
    if(conta-1 == driver_layer.numTransmissions){
        printf("Max retransmissions reached. Exiting...\n");
        exit(1);
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

void printer(unsigned char* src, int size){
    for (int i = 0; i < size; i++){
        printf("%02x ",src[i]);
    }
    printf("\n");
}

int llclose_writter(applicationLayer app){
    //resetar a conta caso tenha havido algum timeout
    conta = 0;
    //se chegou aqui e que acabou de enviar o ficheiro e a trama de controlo, agr desconectar 
    (void) signal(SIGALRM, signal_handler_disc);

    //podemos de fazer malloc(5) porque o emissor so ira receber respostas que teem tamanho maximo de 5
    unsigned char * received = malloc(5);
    int size_received;
    printf("%d bytes written in DISC\n",write(app.fileDescriptor,DISC,sizeof(DISC)));

    alarm(driver_layer.timeout);

    while(conta <= driver_layer.numTransmissions){
        if(flag){
            alarm(driver_layer.timeout);
            flag=0;
            //voltar a enviar apos o timeout
            write(app.fileDescriptor,DISC, sizeof(DISC));
        }

        //guardar o trama que recebeu em received
        if((size_received = read(app.fileDescriptor,received,5)) > 0){
            tcflush(app.fileDescriptor, TCIOFLUSH);
        }

        if(size_received > 0){
            //comparar o trama com DISC e mandar um UA e fechar
            if(memcmp(received,DISC,size_received) == 0){
                printf("received DISC message, sending UA\n");    
                write(app.fileDescriptor,UA_R_1,sizeof(UA_R_1));
                return 0;
            }
            
        }
    }

    return 0;
}
int llwrite(applicationLayer app, unsigned char* src, int src_size){

    // printf("%d control \n",control);
    (void) signal(SIGALRM, signal_handler_send);
    
    int stuff_data_size = 0, bytesWritten = 0, size_received;
    // adicionar 6 por causa dos bytes do header
    unsigned char* stuff_data = framing(src,src_size,&stuff_data_size); 
    printer(stuff_data,stuff_data_size);

    //temos de fazer malloc(src_size*2+6) porque o emissor vai receber uma trama de controlo no fim
    unsigned char * received = malloc(src_size*2+6);


    bytesWritten = write(app.fileDescriptor,stuff_data, stuff_data_size);
    printf("Sending trama with %d bytes\n",bytesWritten);
    alarm(driver_layer.timeout);

    while(conta <= driver_layer.numTransmissions){

        if(flag){
            alarm(driver_layer.timeout);
            flag=0;
            //voltar a enviar apos o timeout
            bytesWritten = write(app.fileDescriptor,stuff_data, stuff_data_size);
        }

        //guardar o trama que recebeu em received
        if((size_received = read(app.fileDescriptor,received,30)) > 0){
            tcflush(app.fileDescriptor, TCIOFLUSH);
        }
        
        if(size_received > 0){
            //comparar o trama com o Ready Reciever trama tendo em atencao ao N, e dar update do N
            if(memcmp(received,REJ_0,size_received) == 0){
                printf("received REJ message, sending again\n");    
                bytesWritten = write(app.fileDescriptor,stuff_data, stuff_data_size);
                continue;
            }
            if (memcmp(received,REJ_1,size_received) == 0){
                printf("received REJ message, sending again\n");    
                bytesWritten = write(app.fileDescriptor,stuff_data, stuff_data_size);
                continue;
            }
            printer(received,size_received);
            if(control == 0){
                if(memcmp(received,RR_1,size_received) == 0){
                    printf("received RR_1 with N = %x\n",received[2]);    
                    alarm(0);
                    control = 1;
                    break;
                }
            }

            if(control == 1){
                if(memcmp(received,RR_0,size_received) == 0){
                    printf("received RR_0 with N = %d\n",received[2]);    
                    alarm(0);
                    control = 0;
                    break;
                }
            }
        }
    }   

    //conta-1 pk no ultimo alarme e feito conta++, e entao se for o maximo de trasmissoes, return -1 
    if(conta-1 == driver_layer.numTransmissions){
        return -1;
    } 

    return 0;
}

int llread(applicationLayer app, unsigned char** output, int datasize, FILE *fp){
 
    //mudar isto com datasize
    int tem_size = datasize * 2 + 10;
    unsigned char* temp = malloc(tem_size);

    while (TRUE) {

        int res;
        if((res = read(app.fileDescriptor,temp,tem_size)) > 0){
            tcflush(app.fileDescriptor, TCIOFLUSH);
        }

        if(res > 0){
            printf("Received trama with %d bytes \n",res);
            unsigned char* data_unstuffed = malloc(res);

            int size_data_unstuffed = 0;
            data_unstuffed = byte_destuff(temp,res,&size_data_unstuffed);


            // se recebemos 5 bytes significa que esta a receber uma trama de supervisao e nao numerada
            if(res == 5){
                if(disconnect == 1){
                    if((memcmp(temp,UA_R_1,res) == 0)){
                        printf("received DISC confirmation, exiting\n");   
                        printf("Recived %d tramas I in total\n",tramas_r);
                        return 0;
                    }
                }
                if((memcmp(temp,DISC,res) == 0) && disconnect == 0){
                    printf("received DISC, stopping, sending DISC\n");
                    write(app.fileDescriptor,DISC,sizeof(DISC));   
                    disconnect = 1;
                    continue;
                }
                
            }

            //verificar o valor de C, assumindo que o formato de F,A,C,BCC e valido no trama, verificar se o trama e valido depois
            if(temp[4] == 1){
                //-9 pk n sabemos o tamanho dos dados pk ainda estao byte stuffed
                if(stateM_data(data_unstuffed,size_data_unstuffed-6,size_data_unstuffed) == 0){

                    fwrite(data_unstuffed+8,1,size_data_unstuffed-10,fp);
                    
                    printer(data_unstuffed,size_data_unstuffed);
                    printf("Received trama is correct \n");

                    if(temp[2] == 0){
                        printf("Responding with a RR_1 message with ");
                        printf("%d bytes\n",(int) write(app.fileDescriptor,RR_1,sizeof(RR_1)));

                    }else if (temp[2] == 1){
                        printf("Responding with a RR_0 message with ");
                        printf("%d bytes\n",(int) write(app.fileDescriptor,RR_0,sizeof(RR_0)));
                    }

                }else{
                    printf("Received trama as an error, sending REJJ message \n");
                    if(temp[2] == 0){
                        write(app.fileDescriptor,REJ_1,sizeof(RR_1));
                    }
                    if(temp[2] == 1){
                        write(app.fileDescriptor,REJ_0,sizeof(RR_0));
                    }
                }
            tramas_r++;
            }

           
        }
    }
    return 0;
    
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
void signal_handler_disc() {
    printf("No valid message DISC recieved! Resending message... %d\\%d\n",conta,ALARM_TIMEOUT);
	flag=1;
	conta++;
}

unsigned char BCC2(unsigned char* data, int size){
    unsigned char* ptr = data;
    unsigned char bcc = (*ptr);
    ptr++;

    for(int i=1; i<size;i++){
        bcc = bcc^(*ptr);
        ptr++;
    }
    return bcc;
}

unsigned char* framing(unsigned char* data, int size, int* framed_data_size){  
    unsigned char* framed_data = malloc(size*2+6);
    int current_size = 0;

    framed_data[0] = 0x7E; //F
    framed_data[1] = 0x03; //A
    current_size += 2;
    if(control == 0){
        framed_data[2] = 0x00; //C
        framed_data[3] = 0x00^0x03; //C
        current_size += 2;
    } 
    else if(control == 1){
        framed_data[2] = 0x01; //C
        framed_data[3] = 0x01^0x03; //C
        current_size += 2;

    } 
    else{
      printf("Control has an illegal value, exiting..");
      exit(0);
    }    
    // dar stuff do data 

    int data_stuff_size = 0;
    unsigned char* temp_data = byte_stuff(data,size,&data_stuff_size);

    //copiar data
    memcpy(framed_data+4, temp_data, data_stuff_size); //D1 -> DN
    current_size += data_stuff_size;    

    // dar stuff do bcc
    int byte_stuff_size = 0;
    unsigned char bcc = BCC2(data, size);
    unsigned char* byte_stuff_data = byte_stuff(&bcc,1,&byte_stuff_size);

    //adicionar o bcc a trama 
    if(byte_stuff_size == 1){
        framed_data[4 + data_stuff_size] = bcc; //BCC2
        framed_data[5 + data_stuff_size] = 0x7E; //F
        current_size += 2;

    } else {
        // realocar memoria porque o bcc tem 2 bytes
        if(realloc(framed_data, current_size+1) == NULL){
            printf("realloc failed\n");
            exit(1); 
        }
        memcpy(framed_data + 4 + data_stuff_size,byte_stuff_data,byte_stuff_size); //BCC2
        framed_data[4 + data_stuff_size +byte_stuff_size ] = 0x7E; //F
        current_size += byte_stuff_size+1;
    }

    *framed_data_size = current_size;

    return framed_data;
}