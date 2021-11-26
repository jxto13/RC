#define MAX_SIZE 255

#include "app.h"

typedef struct linkLayer{
    char port[20];                 /*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate;                  /*Velocidade de transmissão*/
    unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
    unsigned int timeout;          /*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
    char frame[MAX_SIZE];          /*Trama*/
}linkLayer;

int llopen(applicationLayer app);
int llclose(applicationLayer app);
int llclose_writter(applicationLayer app);
int llwrite(applicationLayer app, unsigned char* src, int src_size);
int llread(applicationLayer app, unsigned char** output, int datasize,FILE *fp);
unsigned char BCC2(unsigned char* data, int size);
unsigned char* framing(unsigned char* data, int size,int* framed_data_size);
void signal_handler();
void signal_handler_send();
void signal_handler_disc();
void printer(unsigned char* src, int size);
