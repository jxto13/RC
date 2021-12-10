#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* file_name_received;
char* file_size_received;

void open_control_data_package(unsigned char* received_control_data_package){
    //if(received_control_data_package[0] == 0x03)
    // 7e 03 00 03 02 00 07 33 30 35 38 39 30 35 01 08 74 65 73 74 2e 67 69 66 6e 7e 

    int states[2] = {0,0};
    int size;

    unsigned char* ptr = received_control_data_package; //Começa pelo controlo
    ptr++; //Passa para o Tipo

    if(*ptr == 0x00){ //Se o tipo for T1
        ptr++; //Passa para o tamanho L1
        size = *ptr;
        file_size_received = malloc(*ptr);
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
        file_name_received = malloc(*ptr);
        for (int i = 0; i < size; i++)
        {
            ptr++; //Para percorrer a trama e ler os valores
            file_name_received[i] = *ptr; //Guarda os valores na variável global
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

void printer(unsigned char* src, int size){
    for (int i = 0; i < size; i++){
        printf("%02x ",src[i]);
    }
    printf("\n");
}

int main(){
    unsigned char test[30] = {0x7e, 0x03, 0x00, 0x03, 0x02, 0x00, 0x0a, 0x32, 0x31, 0x32, 0x34, 0x32, 0x30, 0x32, 0x39, 0x39, 0x38, 0x01, 0x09, 0x74, 0x65, 0x78, 0x74, 0x32, 0x2e, 0x67, 0x69, 0x66, 0x64, 0x7e};
    open_control_data_package(test+4);
    printer(test, 30);
    printf("%d\n", atoi(file_size_received));
}