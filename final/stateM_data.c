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

void printer2(unsigned char* src, int size){
    for (int i = 0; i < size; i++){
        printf("%02x ",src[i]);
    }
    printf("\n");
}

int stateM_data(unsigned char* frame, int data_size, int frame_size){
    unsigned char* ptr = frame;
    int i = 0;
    unsigned char state;

    while(i < frame_size){
        i++;

        if(stateM_data_BCC1 == 1 && stateM_data_BCC2 == 0){
            unsigned char* data_ptr = frame;
            // se chegou aqui o bcc1 e valido, sabemos que isto e um trama I, logo avancamos 8 bytes para os dados
            data_ptr += 4;
            data_destuffed = malloc(data_size); // +1 para o BCC2

            for (int j = 0; j < data_size; j++){  
                data_destuffed[j] = *data_ptr;
                data_ptr++;
            }
            BCC2_destuffed = *data_ptr;
            
            if(BCC2_destuffed == BCC2_calculation(data_destuffed,data_size)){ //compara BCC2
                stateM_data_BCC2 = 1;
                ptr = data_ptr+1;
            }

            else{
                printf("Frame has an error! Please request a new frame\n");
                return -1;
            }
        }

        state = *ptr;

        switch (state)
        {
            case 0x7E:
                if(stateM_data_BCC2 == 1){
                    clear_machine_stateM_data();
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

/* BCC1 : SE C FOR 1 = 2
          SE C FOR 0 = 3 */