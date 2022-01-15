/* #include <stdio.h>
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
*/

#include <stdio.h>
#include <stdlib.h>

#include "byteStuffing.h"

int stateM_data_F = 0;
int stateM_data_A = 0;
int stateM_data_C = 0;
int stateM_data_BCC1 = 0;
int stateM_data_BCC2 = 0;
int c_value = -1;
unsigned char* data_destuffed;
unsigned char BCC2_destuffed;

void clear_machine_stateM_data(){
    stateM_data_F = 0;
    stateM_data_A = 0;
    stateM_data_C= 0;
    stateM_data_BCC1 = 0;
    stateM_data_BCC2 = 0;
    c_value = 0;
}

unsigned char BCC2_calculation(unsigned char* data,int size){
    unsigned char* ptr = data;
    unsigned char bcc = (*ptr);
    ptr++;

    for(int i=1; i<size;i++){
        bcc = bcc^(*ptr);
        ptr++;
    }
    return bcc;
}

int stateM_data(unsigned char* frame, int data_size, int frame_size){
    unsigned char* ptr = frame;
    int i = 0;
    unsigned char state;

    while(i < frame_size){
        i++;

        if(stateM_data_BCC1 == 1 && stateM_data_BCC2 == 0){
            unsigned char* save_point = ptr;
            ptr += 4;
            data_destuffed = malloc(data_size); // +1 para o BCC2

            for (int j = 0; j < data_size; j++){  
                data_destuffed[j] = *ptr;
                ptr++;
            }
            BCC2_destuffed = *ptr;
            
            if(BCC2_destuffed == BCC2_calculation(data_destuffed,data_size)){ //compara BCC2
                stateM_data_BCC2 = 1;
                ptr = ptr+1;
            }

            else{
                printf("Frame has an error! Please request a new frame\n");
                ptr = save_point;
                clear_machine_stateM_data();
            }
        }

        state = *ptr;
        
        switch (state)
        {
            case 0x7E:
                if(stateM_data_BCC2 == 1){
                    clear_machine_stateM_data();
                    return 1;
                }
                else if(stateM_data_BCC2 == 0 && stateM_data_BCC1 == 1) clear_machine_stateM_data();
                else stateM_data_F = 1;
                break;

            case 0x03: 
                if(stateM_data_F == 1 && stateM_data_A == 0) stateM_data_A = 1;
                else if(stateM_data_A == 1 && stateM_data_C== 1 && c_value == 0 && stateM_data_BCC1 == 0) stateM_data_BCC1 = 1;
                else clear_machine_stateM_data();
                break;

            case 0x00:
                if(stateM_data_A == 1 && stateM_data_C== 0){
                    stateM_data_C= 1;
                    c_value = 0;
                }
                else clear_machine_stateM_data();
                break;
            
            case 0x01: 
                if(stateM_data_A == 1 && stateM_data_C== 0){
                    stateM_data_C= 1;
                    c_value = 1;
                }
                else clear_machine_stateM_data();
                break;

            case 0x02:
                if(stateM_data_C== 1 && c_value == 1 && stateM_data_BCC1 == 0) stateM_data_BCC1 = 1;
                else clear_machine_stateM_data();
                break;

            default:
                break;
        }
    
        ptr++;
    }
    
    return 0;
}

// Data package size = 9
// Frame size = 27

int main(){
    unsigned char test[27] = {0x7E, 0x03, 0x00, 0x03, 0x02, 0x00, 0x01, 0x7E, 0x03, 0x00, 0x03, 0x02, 0x00, 0x01, 0x00, 0x25, 0x27, 0x29, 0x31, 0x35, 0x2F, 0x7E, 0x34, 0x32, 0x23, 0x00, 0x7E};
    //unsigned char test[15] = {0x7E, 0x03, 0x00, 0x03, 0x02, 0x00, 0x01, 0x00, 0x25, 0x27, 0x29, 0x31, 0x35, 0x2F, 0x7E};
    //unsigned char data[5] = {0x25, 0x27, 0x29, 0x31, 0x35};
    printf("%d\n", stateM_data(test, 5, 27));

    return 0;
}