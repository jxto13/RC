#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* file_name_received;
unsigned char* file_size_received;

int file_length = 10968;

int n_digits(int x){
    int cont = 0;
    while(x != 0){
        x = x/10;
        cont++;
    }

    return cont;
}

int getSize_Uchar(unsigned char* src){
    unsigned char* pointer = src;
    int counter = 0;
    while(*pointer != '\0'){
        counter++;
        pointer++;
    }
    return counter;
}

unsigned char* control_data_package(char* file_name ,int file_length, int control){
  unsigned char* control_data_package = malloc(strlen(file_name) + n_digits(file_length) + 5);
  if(control == 1){ //START
    control_data_package[0] = 0x02; // C
  }
  else{ //END
    control_data_package[0] = 0x03; // C
  }
  control_data_package[1] = 0x00; // T1

  unsigned char file_size[n_digits(file_length)];
  sprintf((char*)file_size, "%d", file_length);

  memcpy(control_data_package+2,(unsigned char[]) {n_digits(file_length)},1); // L1
  memcpy(control_data_package+3, file_size,n_digits(file_length)); // V1
  control_data_package[n_digits(file_length)+3] = 0x01; // T2
  memcpy(control_data_package+n_digits(file_length)+4,(unsigned char[]) {strlen(file_name)},1); // L2
  memcpy(control_data_package+n_digits(file_length)+5,file_name,strlen(file_name)); // V2
  return control_data_package;
}

void printer(unsigned char* to_print, int size){
    for (int i = 0; i < size; i++)
    {
        printf("%x ", to_print[i]);
    }

    printf("\n"); 
}

void open_control_data_package(unsigned char* received_control_data_package){
    //if(received_control_data_package[0] == 0x03)

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
        states[1] = 1; //Para confirmar que já guardou o tamanho do ficheiro
    }

    else{
        if(states[1] == 1 || states[0] == 1){
            free(file_name_received);
            free(file_size_received);
            printf("Something went wrong with the control package, please provide another one");
        }
    }
}

int main(){
    int size = 10968;
    char* file_name = "pinguim.gif"; 

    unsigned char* c_data_package = control_data_package(file_name,size,0);

    printer(c_data_package, 21);
    open_control_data_package(c_data_package);
    printer(file_size_received, 5);
    printer(file_name_received, 11);

    return 0;
}