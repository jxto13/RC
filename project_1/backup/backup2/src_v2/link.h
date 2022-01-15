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
int llwrite(applicationLayer app, unsigned char * buffer, int length);
int llread(applicationLayer app, unsigned char ** buffer);
void signal_handler();
