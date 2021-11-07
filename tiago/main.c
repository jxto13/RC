#include <stdio.h>
#include "stateM_lib.h"

int main(){
    int i = 0;
    // char buf[10] = {0x7E,0x03,0x04,0x7E,0x7E,0x03, 0x03, 0x00, 0x7E, 0x7E};
    char buf[10] = {0x7E,0x03,0x07,0x04,0x00,0x7E, 0x03, 0x03, 0x00, 0x7E};

    while(i < 10){
        // if(stateM(buf[i]) == 1){
        //     printf("%d %d ",i, stateM(buf[i]));
        //     printf("Machine worked..\n");
        // }
        if(stateM(buf[i]) == 0){
            printf("%d %x ",i, buf[i]);
            printf("Machine loading..\n");
        }
        else{
            printf("%d %x ",i, buf[i]);
            printf("Machine worked!\n");
        }
        i++;
    }  

}