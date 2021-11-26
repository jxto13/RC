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

void print_frame(unsigned char* frame, int size){
    for (int i = 0; i < size; i++)
    {
        printf("%x ", frame[i] );
    }
    printf("\n");
}

int stateM_data(unsigned char* frame, int data_size, int frame_size){
    unsigned char* ptr = frame;
    int i = 0;
    unsigned char state;

    while(i < frame_size){
        i++;

        if(BCC1 == 1 && BCC2 == 0){
            printf("%d\n",i);
            unsigned char* data_ptr = frame;
            data_ptr += 8;
            data_destuffed = malloc(data_size); // +1 para o BCC2
            for (int j = 0; j < data_size; j++)
            {  
                printf("%x ",*data_ptr);
                data_destuffed[j] = *data_ptr;
                data_ptr++;
            }
            printf("\n");
            BCC2_destuffed = *data_ptr; 
            if(BCC2_destuffed == BCC2_calculation(data_destuffed,data_size)){ //compara BCC2
                BCC2 = 1;
                ptr = data_ptr+1;
            }

            else{
                printf("Frame has an error! Please request a new frame\n");
                exit(0);
            }
        }
            // print_frame(data_destuffed,10);
        }

        state = *ptr;

        switch (state)
        {
            case 0x7E:
                if(BCC2 == 1){
                    clear_machine();
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
    
    return 0;
}

/* BCC1 : SE C FOR 1 = 2
          SE C FOR 0 = 3 */

int main(){
    // unsigned char frame[SIZE] = {0x7E, 0x03, 0x01, 0x02, 0x27, 0x29, 0x31, 0x34, 0x37, 0x1B, 0x7E};
    // unsigned char frame2[20] = {0x7E, 0x03, 0x00, 0x03, 0x01, 0x00, 0x02, 0x0A, 0x47, 0x49, 0x46, 0x38, 0x39, 0x61
    // , 0x01, 0x01, 0x2f, 0x01, 0x06, 0x7E};
    unsigned char frame3[20] = {0x7E, 0x03, 0x01, 0x02, 0x01, 0x00, 0x02, 0x0A, 0xF5, 0x00, 0x00, 0xF8, 0xF8, 0xF8
    , 0xF0, 0xF0, 0xF0, 0xE8, 0x15, 0x7E};
    // 47 49 46 38 39 61 01 01 2f 01
// F5 00 00 F8 F8 F8 F0 F0 F0 E8
// 7e 03 01 02 01 00 02 0a 4e af db ef c8 b8 7d 5e 8f ef fb 40 7e 
    // unsigned char data[10] = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x01, 0x2f, 0x01};
    // unsigned char data2[10] = {0xF5, 0x00, 0x00, 0xF8, 0xF8, 0xF8, 0xF0, 0xF0, 0xF0, 0xE8};

    // printf("BCC2 dos dados calculado na main :%x\n", BCC2_calculation(data,10));    

    print_frame(frame3, 20);
    printf("FRAME I STATE: %d\n",stateM_data(frame3,10,20));

    return 0;
}