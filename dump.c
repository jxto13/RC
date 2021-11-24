#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "byteStuffing.h"

/* int comperator(unsigned char* stuff_data, unsigned char* destuff_data,int size){
    unsigned char* start = stuff_data;
    unsigned char* start2 = destuff_data;
    unsigned char* printerstart = stuff_data;
    unsigned char* printerstart2 = destuff_data;
        // printf("destuff_data_size %d\n",destuff_data_size);
    while (*start != NULL) {
        if(*start != *start2){
            while(*printerstart != NULL){
                printf("%x ",*printerstart);
                printerstart++;
            }
            printf("\n");
            while(*printerstart2 != NULL){
                printf("%x ",*printerstart2);
                printerstart2++;
            }
            printf("\n");
            break;
        }

        start++;
        start2++;

    }
    return 0;
} */
int getSize_Uchar(unsigned char* src){
    unsigned char* pointer = src;
    int counter = 0;
    while(*pointer != '\0'){
        counter++;
        pointer++;
    }
    return counter;
}
// void printer(unsigned char * src){
//     unsigned char* start = src;
        
//         while(*start != NULL){
//                 printf("%x ",*start);
//                 if(start == 0x7d) printf("--------\n");
//                 start++;

//             }
//             printf("\n");
// }
int main(){ 
/* 


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
     */
    // FILE *fp, *dest;
    // int length;

    unsigned char* input = (unsigned char*)"pinguim.gif";   
    printf("%d\n",getSize_Uchar(input));

    // fp = fopen("pinguim.gif", "rb");   
    // dest = fopen("pinguim_transmitted.gif", "w");    

    // fseek(fp, 0, SEEK_END);         
    // length = ftell(fp);           
    // rewind(fp);             

    // int bytesToRead = 15;
    // unsigned char* file_data = malloc(bytesToRead);

    // int bytesRead;

    // /* unsigned char test[9] = {0xA8, 0x9A, 0xE9, 0xB7, 0x7E, 0x9F, 0x77, 0x9F, 0x5D};
    //     int stuff_data_size = 0;

    //     unsigned char* stuff_data = byte_stuff(test,9,&stuff_data_size);
    //     printf("Stuff data\n");
    //     printer(stuff_data);

    //     int destuff_data_size = 0;
    //     unsigned char* destuff_data = byte_destuff(stuff_data,stuff_data_size,&destuff_data_size);
        
    //     printf("Destuff data\n");
    //     printer(destuff_data); */
    //  for (int i = 0; i < length; i+=bytesToRead) {
    //     // fread(file_data, 1, 15, fp);
    //     bytesRead = fread(file_data, 1, bytesToRead, fp);
    //     // if(i > 10900) printf("%d bytesRead\n",bytesRead);
    //     int stuff_data_size = 0;
    //     unsigned char* stuff_data = byte_stuff(file_data,bytesToRead,&stuff_data_size);

    //     // printf("bytesRead %d\n",bytesRead);
    //     // printf("stuff_data_size %d\n",stuff_data_size);

    //     // printf("Stuff data\n");
    //     // printer(stuff_data);
       
    //     for (int i = 0; i < stuff_data_size; i++){
    //         // if(file_data[i] == 0x7E) printf("%d - %x %x\n",i,*stuff_data, *stuff_data+1);

    //         // printf("%x ",stuff_data[i]);
    //     }
        
    //     // printf("\n");

    //     int destuff_data_size = 0;
    //     unsigned char* destuff_data = byte_destuff(stuff_data,stuff_data_size,&destuff_data_size);
        
    //     // printf("Destuff data\n");
    //     // printer(destuff_data);
    //     for (int i = 0; i < destuff_data_size; i++){
    //         // if(file_data[i] == 0x7E) printf("%d - %x %x\n",i,*stuff_data, *stuff_data+1);
    //         // if(destuff_data[i] == 0x7D) printf("---------");

    //         // printf("%x ",destuff_data[i]);
    //     }
    //     // printf("\n");
    //     if(destuff_data_size != stuff_data_size){
    //         printf("%d -- ",i);
    //         printf("%d destuff_data_size | %d stuff_data_size\n",destuff_data_size,stuff_data_size);
    //     }
    //     // comperator(stuff_data,destuff_data,stuff_data_size);
    //     fwrite(destuff_data,1,destuff_data_size,dest);

    //     // for (int i = 0; i < sizeof(stuff_data); i++){
    //     //     printf("%x ",*start);
    //     //     *start++;
    //     // }
    //     // printf("\n");
        
        
    // }
     
    // // while ((bytesRead = fread(file_data, 1, 8, fp)) > 0) {
    // //     unsigned char* stuff_data = byte_stuff(&(file_data));

    // //     // if(file_data[i] == 0x7E) printf("%d - %x %x\n",i,*stuff_data, *stuff_data+1);

    // //     unsigned char* indexTask;
	// // 	// LIST_TASK *newCar = malloc(sizeof(LIST_TASK));
	// // 	for (indexTask = stuff_data; indexTask != '\0'; indexTask++){
    // //         printf("%x ",*file_data);

    // //     }
        
    // //     printf("\n");

  
    // //     // for (int i = 0; i < 8; i++){
    // //     //     printf("%x ",file_data[i]);
    // //     // }
    // //     // printf("\n");
    // // }
  
    // // fread(file_data, length, 1, fp);
 
    // // // fgets(file_data,length,fp);
    // // for (int i = 0; i < length; i++) {
    // //     // fwrite(&(file_data[i]),1,1,dest);
    // //     unsigned char* stuff_data = byte_stuff(&(file_data[i]));
    // //     if(file_data[i] == 0x7E) printf("%d - %x %x\n",i,*stuff_data, *stuff_data+1);
    // // }
    // fclose(fp);
    // fclose(dest);

    return 0;
}