
#ifndef APP_H
#define APP_H


typedef struct applicationLayer {
    int fileDescriptor; /*Descritor correspondente à porta série*/
    int status;         /*TRANSMITTER = 1| RECEIVER = 0*/
}applicationLayer;

#endif