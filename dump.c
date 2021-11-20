#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){ 

    char* C = "2";
    char* T1 = "0";
    char* L1 = "5";
    char* V1 = "10968";
    char* T2 = "1";
    char* L2 = "12";
    char* V2 = "pinguim.gif";
    
    char* INIT = malloc(sizeof(char*)*100);

    strcat(INIT,C);
    strcat(INIT,T1);
    strcat(INIT,L1);
    strcat(INIT,V1);
    strcat(INIT,T2);
    strcat(INIT,L2);
    strcat(INIT,V2);

    printf("%s\n",INIT);
    // FILE *fp, *dest;
    // int length;


    // fp = fopen("pinguim.gif", "rb");   
    // dest = fopen("pinguim_transmitted.gif", "a");    

    // fseek(fp, 0, SEEK_END);         
    // length = ftell(fp);           
    // rewind(fp);             

    // unsigned char file_data[length];

    // fread(file_data, length, 1, fp);
 
    // // fgets(file_data,length,fp);
    // for (int i = 0; i < length; i++) {
    //     fwrite(&(file_data[i]),1,1,dest);
        
    // }
    // fclose(fp);
    // fclose(dest);

    return 0;
}