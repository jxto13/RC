#include <stdio.h>
#include "stateM_lib.h"

int F = 0;
int A = 0;
int C = 0;
int BCC = 0;

void clear_machine(){
    F = 0;
    A = 0;
    C = 0;
    BCC = 0;
}

int stateM(char state){
    printf("F = %d, A = %d, C = %d, BCC = %d\n",F,A,C,BCC);
    switch (state){

    case 0x7E: //Se encontrar uma flag
        if(F == 1 && BCC == 1){
            clear_machine();
            return 1; //Se o último foi uma flag
        } 
        else if(F == 1 && BCC == 0) break;
        else F = 1;
        break;

    case 0x03: //Se encontrar A ou C
        if(F == 1 && A == 0){
            A = 1;
            break;
        } 
        else if(F == 1 && A == 1){
            C = 1;
            break;
        } 

    case 0x00:
        //caso de A = 3 e C = 3, flag "SET", BCC=A^C=0
        if(C == 1) {
            BCC = 1;
            break;
        }
    default:
        clear_machine();
        break;
    }
    return 0;
}
