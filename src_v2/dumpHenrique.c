#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATASIZE 50

int converter(int k){
  return (unsigned char) (k%256);
}

int main(){
    unsigned char* data_package = malloc(DATASIZE + 4);
    memcpy(data_package,(unsigned char[]) {converter(100)},1);
    printf("%c\n",data_package[0]);
    return 0;
}