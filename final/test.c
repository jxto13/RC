#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void printer(unsigned char* to_print){
    printf("SIZE: %d\n", 21);
    for (int i = 0; i < 21; i++)
    {
        printf("%x ", to_print[i]);
    }

    printf("\n"); 
}

int main(){
    int size = 10968;
    char* file_name = "pinguim.gif"; 

    unsigned char* test = control_data_package(file_name,size,0);

    printer(test);

    return 0;
}