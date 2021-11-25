#include <stdio.h>
#include <string.h>
#include <stdlib.h>


unsigned char* byte_stuff(unsigned char* input_data, int input_data_size, int* stuff_data_size){
    unsigned char *data_ptr = malloc(input_data_size);
    int j = 0, reallocsize = input_data_size;

    for (int i = 0; i < input_data_size; i++) {
        if(input_data[i] == 0x7E){
            reallocsize++;
            if(realloc(data_ptr,reallocsize) == NULL){
                printf("realloc failed\n");
                exit(1); 
            }
            data_ptr[j] = 0x7D;
            data_ptr[j+1] = 0x5E;
            j += 2;
        }
        else if(input_data[i] == 0x7D){
            reallocsize++;
            if(realloc(data_ptr,reallocsize) == NULL){
                printf("realloc failed\n");
                exit(1); 
            }
            data_ptr[j] = 0x7D;
            data_ptr[j+1] = 0x5D;
            j += 2;
        }
        else{
            data_ptr[j] = input_data[i];
            j++;
        }
    }
    *stuff_data_size = j;
    return data_ptr;
}

unsigned char* byte_destuff(unsigned char* input_data, int input_data_size, int* destuff_data_size){
    unsigned char *data_ptr = malloc(input_data_size);
    int j = 0, reallocsize = input_data_size;

    for (int i = 0; i < input_data_size; i++) {
        if(input_data[i] == 0x7D){
            if(input_data[i+1] == 0x5E){
                data_ptr[j] = 0x7E;
                j++;
                i = i+1;
                reallocsize--;
                if(realloc(data_ptr,reallocsize) == NULL){
                    printf("realloc failed\n");
                    exit(1); 
                }
            }
            else if(input_data[i+1] == 0x5D){
                data_ptr[j] = 0x7D;
                j++;
                i = i+1;
                reallocsize--;
                if(realloc(data_ptr,reallocsize) == NULL){
                    printf("realloc failed\n");
                    exit(1); 
                }
            }
        }
        else{
            data_ptr[j] = input_data[i];
            j++;
        }
    }
    *destuff_data_size = j;
    return data_ptr;
}