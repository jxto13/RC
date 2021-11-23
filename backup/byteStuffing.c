#include <stdio.h>
#include <string.h>

#define SIZE 10

int cont = 0;

unsigned char* byte_stuff(unsigned char* input_data){
    unsigned char output_data[SIZE];
    unsigned char *data_ptr = output_data;
    int j = 0;
    for (int i = 0; i < sizeof(input_data); i++)
    {
        if(input_data[i] == 0x7E){
            output_data[j] = 0x7D;
            output_data[j+1] = 0x5E;
            j += 2;
            cont++;
        }
        else if(input_data[i] == 0x7D){
            output_data[j] = 0x7D;
            output_data[j+1] = 0x5D;
            j += 2;
            cont++;
        }
        else{
            output_data[j] = input_data[i];
            j++;
        }
    }

    return data_ptr;
}

unsigned char* byte_destuff(unsigned char* output_data){
    unsigned char received_data[SIZE];
    unsigned char* received_ptr = received_data;
    int j = 0;
    for (int i = 0; i < sizeof(output_data); i++)
    {
        if(output_data[i] == 0x7D){
            if(output_data[i+1] == 0x5E){
                received_data[j] = 0x7E;
                j++;
                i = i+1;
            }
            else if(output_data[i+1] == 0x5D){
                received_data[j] = 0x7D;
                j++;
                i = i+1;
            }
        }
        else{
            received_data[j] = output_data[i];
            j++;
        }
    }

    return received_ptr;
}

// int main(){
//     unsigned char input[SIZE] = {0x73,0x7E,0x7D,0x23,0x7E};
//     unsigned char stuffed[SIZE];
//     unsigned char received_data[SIZE];

//     printf("This is the input: ");
//     puts(input);

//     memcpy(stuffed,byte_stuff(input),sizeof(byte_stuff(input))+1);
//     printf("This is the same input, but after byte stuffing: ");
//     puts(stuffed);

//     memcpy(received_data,byte_destuff(stuffed),sizeof(byte_destuff(stuffed))-cont);
//     printf("After destuffing: ");
//     puts(received_data); 
//     return 0;   
// }