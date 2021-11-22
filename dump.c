#include <stdio.h>
#include <stdlib.h>
#include <string.h>


size_t getSize ( unsigned char * s ) 
{
    size_t size = 0;

    while ( *s++ ) ++size;

    return size;
}

int main(){ 




    unsigned char* str1 = (unsigned char*) 0x2F;
    unsigned char* str2 = (unsigned char*) 0x2F;

    unsigned char* str3 = malloc(sizeof(str1)+sizeof(str2));

    // unsigned char* allstrings = malloc(sizeof(str1));
    // unsigned char* allstrings2 = malloc(sizeof(str1));


    // strcpy(allstrings,str1);
    // strcpy(allstrings2,str2);

    // int size = sizeof(str1) + sizeof(str2);
    // if(realloc(str1,size) == NULL){
    //     printf("realloc failed\n");
    //     exit(1);
    // }
    int size1 =sizeof(str1);
    int size2 =sizeof(str2);

    printf("%ld\n",getSize(str1));
    
    // printf("%x\n",(unsigned int)str1);

    printf("%d size1 %d size2\n",size1,size2);
    // memcpy(str3,str1,size1);
    // memcpy(str3+size1,str2,size2);


    // printf("%s\n",str3);
    // char* C = "2";
    // char* T1 = "0";
    // char* L1 = "5";
    // char* V1 = "10968";
    // char* T2 = "1";
    // char* L2 = "12";
    // char* V2 = "pinguim.gif";
    
    // char* INIT = malloc(sizeof(char*)*100);

    // strcat(INIT,C);
    // strcat(INIT,T1);
    // strcat(INIT,L1);
    // strcat(INIT,V1);
    // strcat(INIT,T2);
    // strcat(INIT,L2);
    // strcat(INIT,V2);

    // printf("%s\n",INIT);

    // printf("%ld\n",sizeof(unsigned char *));
    
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