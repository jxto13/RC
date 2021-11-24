#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DATASIZE 50

double log2(double x){
  return log(x) / log(2);
}

int n_bytes(int data){
  return (int) ceil((log2(data)/8.0));
}


int main(){
    printf("%d\n",n_bytes(65000));
    return 0;
}