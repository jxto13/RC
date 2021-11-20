#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){ 
    FILE *fp;
    int length;


    fp = fopen("pinguim.gif", "rb");    
    fseek(fp, 0, SEEK_END);         
    length = ftell(fp);           
    rewind(fp);             

    unsigned char file_data[length];

    fread(file_data, length, 1, fp);
    // fgets(file_data,length,fp);
    for (int i = 20; i < 30; i++) {
        if(file_data[i] == 0xE0) printf("True ");
        printf("%ld - %x \n",sizeof(file_data[i]), file_data[i]);

    }

    return 0;
}