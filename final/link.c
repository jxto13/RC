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

#define VTIME_VALUE_TRANSMITTER 1 // VTIME
#define VMIN_VALUE_TRANSMITTER 0 // VMIN

#define VTIME_VALUE_RECEIVER 5 // VTIME
#define VMIN_VALUE_RECEIVER 0 // VMIN

#define TRANSMITTER 1
#define RECEIVER 0

#define FALSE 0
#define TRUE 1

int PACKAGE_SIZE;

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

unsigned char* file_name_received;
unsigned char* file_size_received;
int file_name_size;

int flag=0, conta=1;
int flag_send=0, conta_send=1;
int flag_disc=0, conta_disc=1;

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

void open_control_data_package(unsigned char* received_control_data_package);

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
                printf("printing\n");
                if((res = write(app.fileDescriptor,SET,5)) < 0){
                    printf("Error occurred at write() function.\n Exiting! \n");
                    return -1;
                }
                conta++;
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
    printf("Sending DISC message\n");
    write(app.fileDescriptor,DISC,sizeof(DISC));

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
                if(write(app.fileDescriptor,UA_R_1,sizeof(UA_R_1)) < 0){ 
                    printf("Error occurred at write() function.\n Exiting! \n");
                    exit(1);
                }
                return 0;
            }
        }
    }

    return 0;
}

int llwrite(applicationLayer app, unsigned char* src, int src_size){
    
    int stuff_data_size = 0, bytesWritten = 0, size_received;

    // adicionar 6 por causa dos bytes do header
    unsigned char* stuff_data = framing(src,src_size,&stuff_data_size); 

    //temos de fazer malloc(src_size*2+6) porque o emissor vai receber uma trama de controlo no fim
    unsigned char * received = malloc(src_size*2+6);

    (void) signal(SIGALRM, signal_handler_send);

    bytesWritten = write(app.fileDescriptor,stuff_data, stuff_data_size);
    printf("Sending trama with %d bytes\n",bytesWritten);
    printer(stuff_data,stuff_data_size);
    alarm(driver_layer.timeout);


    while(conta_send <= driver_layer.numTransmissions){

        if(flag_send){
            alarm(driver_layer.timeout);
            flag_send=0;
            //voltar a enviar apos o timeout
            bytesWritten = write(app.fileDescriptor,stuff_data, stuff_data_size);
            printf("Resending trama with %d bytes\n",bytesWritten);
            conta_send++;
        }

        //guardar o trama que recebeu em received
        if((size_received = read(app.fileDescriptor,received,src_size*2+6)) > 0){
            // printer(received,size_received);
        }

        if(size_received > 0){
            // se recebemos 5 bytes significa que esta a receber uma trama de supervisao e nao numerada
            if(size_received == 5){
                if(control == 0){
                    if(memcmp(received,RR_1,size_received) == 0){
                        printf("received RR_1 with %d bytes\n--------\n",size_received);    
                        alarm(0);
                        control = 1;
                        conta_send = 0;
                        return 0;
                    }
                }
                if(control == 1){
                    if(memcmp(received,RR_0,size_received) == 0){
                        printf("received RR_0 with %d bytes\n--------\n",size_received);    
                        alarm(0);
                        control = 0;
                        conta_send = 0;
                        return 0;
                    }
                }
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
            }
        }
    }   

    //conta-1 pk no ultimo alarme e feito conta++, e entao se for o maximo de trasmissoes, return -1 
    if(conta_send - 1 == driver_layer.numTransmissions){
        return -1;
    } 
    conta_send = 1;
    return 0;
}


int llread(applicationLayer app, unsigned char** output, int datasize, FILE *fp){

    int N_control = 1, res, maxDataSize = datasize * 2 +6;

    while(TRUE) {
        char buf[maxDataSize];

        int total_read = 0, buf_size = maxDataSize;
        int n_read; 

        while ((n_read = read(app.fileDescriptor, buf + total_read, buf_size - total_read)) > 0) {
            // printf("%d n_read\n",n_read);
            total_read += n_read;
        }
        // printer(buf, total_read);

        res = total_read;
        if(res > 0){
            printf("Received trama with %d bytes \n",res);
            unsigned char* data_unstuffed = malloc(res);

            int size_data_unstuffed = 0;
            data_unstuffed = byte_destuff((unsigned char*)buf,res,&size_data_unstuffed);

            // se recebemos 5 bytes significa que esta a receber uma trama de supervisao e nao numerada
            if(res == 5){
                if(disconnect == 1){
                    if((memcmp(buf,UA_R_1,res) == 0)){
                        printf("received DISC confirmation, exiting\n");   
                        printf("Recived %d tramas I in total\n",tramas_r);
                        return 0;
                    }
                }
                if((memcmp(buf,DISC,res) == 0) && disconnect == 0){
                    printf("received DISC, stopping, sending DISC\n");
                    write(app.fileDescriptor,DISC,sizeof(DISC));   
                    disconnect = 1;
                    continue;
                }  
            }

            //-6 pk n sabemos o tamanho dos dados pk ainda estao byte stuffed
                if(stateM_data(data_unstuffed,size_data_unstuffed-6,size_data_unstuffed) == 1){
                    // se receber um pacote de start
                    if(buf[4] == 2){
                        if(buf[2] == 0){
                            printf("recieved start control trama\n");
                            //+4 para remover o header
                            open_control_data_package(data_unstuffed+4);
                            char fwrite_name[200];
                            memcpy(fwrite_name,file_name_received,file_name_size);
                            strcat(fwrite_name,"test");
                            fp = fopen(fwrite_name, "w");
                            if(fp == NULL){
                                printf("File Open failed\n");
                                return -1;
                            }
                            write(app.fileDescriptor,RR_1,sizeof(RR_1));
                            continue;
                        }else if (buf[2] == 1){
                            printf("recieved start control trama\n");
                            fp = fopen("pinguim_transmitted.gif", "w");
                            if(fp == NULL){
                                printf("File Open failed\n");
                                return -1;
                            }
                            write(app.fileDescriptor,RR_0,sizeof(RR_0));
                            continue;
                        }
                    }
                    // se receber um pacote de end
                    if(buf[4] == 3){
                        if(buf[2] == 0){
                            printf("recieved end control trama\n");
                            write(app.fileDescriptor,RR_1,sizeof(RR_1));
                            fclose(fp); 
                            printf("-------Disconnect tramas----------\n"); 
                            continue;
                        }else if (buf[2] == 1){
                            printf("recieved end control trama\n");
                            write(app.fileDescriptor,RR_0,sizeof(RR_0));
                            fclose(fp); 
                            printf("-------Disconnect tramas----------\n"); 
                            continue;
                        }
                    }

                    if(buf[4] == 1){ //trama de informacao
                        printf("temp[2] = %d N_control = %d\n",buf[2],N_control);
                        if(N_control == buf[2]){
                            
                            fwrite(data_unstuffed+8,1,size_data_unstuffed-10,fp);

                            if(buf[2] == 0){
                                printf("Responding with a RR_1 message with ");
                                printf("%d bytes\n",(int) write(app.fileDescriptor,RR_1,sizeof(RR_1)));
                                N_control = 1;
                            }else if (buf[2] == 1){
                                printf("Responding with a RR_0 message with ");
                                printf("%d bytes\n",(int) write(app.fileDescriptor,RR_0,sizeof(RR_0)));
                                N_control = 0;
                            }
                            control = buf[2];
                        }
                        else{
                            printf("frame is repeated\n");
                            if(buf[2] == 0){
                                printf("Responding with a RR_1 message with ");
                                printf("%d bytes\n",(int) write(app.fileDescriptor,RR_1,sizeof(RR_1)));
                                continue;
                            }else if (buf[2] == 1){
                                printf("Responding with a RR_0 message with ");
                                printf("%d bytes\n",(int) write(app.fileDescriptor,RR_0,sizeof(RR_0)));
                                continue;
                            }
                        }
                    }
                } else {
                    //rejj
                }   
            tramas_r++;
            }
    }
    return 0;
}

void signal_handler() {
    if(conta == 0 ){
        flag=1;
    }else{
	    printf("No valid message UA recieved! Resending message... %d\\%d\n",conta,ALARM_TIMEOUT);
        flag=1;
    }
}

void signal_handler_send() {
    if(conta_send == 0 ){
        flag_send=1;
    }else{
        printf("No valid response message recieved! Resending message... %d\\%d\n",conta_send,ALARM_TIMEOUT);
        flag_send=1;
    }
}

void signal_handler_disc() {
	if(conta_disc == 0 ){
        flag_disc=1;
    }else{
        printf("No valid message DISC recieved! Resending message... %d\\%d\n",conta_send,ALARM_TIMEOUT);
        flag_disc=1;
    }
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

void open_control_data_package(unsigned char* received_control_data_package){
    //if(received_control_data_package[0] == 0x03)
    // 7e 03 00 03 02 00 07 33 30 35 38 39 30 35 01 08 74 65 73 74 2e 67 69 66 6e 7e 
    printer(received_control_data_package,26);
    int states[2] = {0,0};
    int size;

    unsigned char* ptr = received_control_data_package; //Começa pelo controlo
    ptr++; //Passa para o Tipo

    if(*ptr == 0x00){ //Se o tipo for T1
        ptr++; //Passa para o tamanho L1
        size = *ptr;
        file_name_size = size;
        printf("SIZE:%d\n",size);
        file_size_received = malloc(size);
        for (int i = 0; i < size; i++)
        {
            ptr++; //Para percorrer a trama e ler os valores
            file_size_received[i] = *ptr; //Guarda os valores na variável global
        }

        ptr++; //Passar para o próximo passo
        states[0] = 1; //Para confirmar que já guardou o tamanho do ficheiro
    }

    if(*ptr == 0x01){ //Se o tipo for T2
        ptr++; //Passa para o tamanho L2
        size = *ptr;
        file_name_received = malloc(size);
        file_name_size = size;
        for (int i = 0; i < size; i++)
        {
            ptr++; //Para percorrer a trama e ler os valores
            file_name_received[i] = *ptr; //Guarda os valores na variável global
            printf("VALOR DO PTR: %x\n", *ptr);
            printf("VALOR DO FILE_NAME_RECEIVED[%d]: %x\n",i, file_name_received[i]);
        }
        states[1] = 1; //Para confirmar que já guardou o nome do ficheiro
    }

    else{
        if(states[1] == 1 || states[0] == 1){
            free(file_name_received);
            free(file_size_received);
            printf("Something went wrong with the control package, please provide another one");
        }
    }
}