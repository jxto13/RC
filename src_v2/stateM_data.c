#include <stdio.h>
#include <stdlib.h>

#define SIZE 6 + 5

int F = 0;
int A = 0;
int C = 0;
int BCC1 = 0;
int BCC2 = 0;
int c_value = -1;
unsigned char* data_destuffed;
unsigned char BCC2_destuffed;

void clear_machine(){
    F = 0;
    A = 0;
    C = 0;
    BCC1 = 0;
    BCC2 = 0;
    c_value = 0;
}

unsigned char BCC2_calculation(unsigned char* data){
    unsigned char* ptr = &data[0];
    unsigned char bcc = (*ptr);
    for(int i=0; i<5;i++){
      bcc = bcc^(*ptr);
      ptr++;
    }
    return bcc;
}

void print_frame(unsigned char* frame, int size){
    for (int i = 0; i < size; i++)
    {
        printf("%x ", frame[i] );
    }
    printf("\n");
}

int stateM_data(unsigned char* frame, int data_size, int frame_size){
    unsigned char* ptr = &frame[0];
    int i = 0;
    unsigned char state;

    while(i < frame_size){
        i++;

        if(BCC1 == 1){
            unsigned char* data_ptr = ptr;
            data_destuffed = malloc(data_size); // +1 para o BCC2
            for (int j = 0; j < data_size; j++)
            {  
                data_destuffed[j] = *data_ptr;
                data_ptr++;
            }

            BCC2_destuffed = *data_ptr; 
            
            if(BCC2_destuffed == BCC2_calculation(data_destuffed)){ //compara BCC2
                BCC2 = 1;
                ptr = data_ptr+1;
            }

            else{
                printf("Frame has an error! Please request a new frame\n");
                exit(0);
            }
        }

        state = *ptr;

        switch (state)
        {
            case 0x7E:
                if(BCC2 == 1){
                    clear_machine();
                    return 1;
                }
                else if(BCC2 == 0 && BCC1 == 1) clear_machine();
                else F = 1;
                break;

            case 0x03: 
                if(F == 1 && A == 0) A = 1;
                else if(A == 1 && C == 1 && c_value == 0 && BCC1 == 0) BCC1 = 1;
                else clear_machine();
                break;

            case 0x00:
                if(A == 1 && C == 0){
                    C = 1;
                    c_value = 0;
                }
                else clear_machine();
                break;
            
            case 0x01: 
                if(A == 1 && C == 0){
                    C = 1;
                    c_value = 1;
                }
                else clear_machine();
                break;

            case 0x02:
                if(C == 1 && c_value == 1 && BCC1 == 0) BCC1 = 1;
                else clear_machine();
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

int main(){
    unsigned char frame[SIZE] = {0x7E, 0x03, 0x00, 0x03, 0x27, 0x29, 0x31, 0x34, 0x37, 0x1B, 0x7E};
    //unsigned char data[5] = {0x27, 0x29, 0x31, 0x34, 0x37};
    //printf("BCC2 dos dados calculado na main :%x\n", BCC2_calculation(data));    

    //print_frame(frame, SIZE);
    printf("FRAME I STATE: %d\n",stateM_data(frame,5,SIZE));

    return 0;
}