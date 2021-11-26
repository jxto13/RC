#include <unistd.h>
#include <signal.h>
#include <stdio.h>

int flag=1, conta=1;
int UA = 0, sig = 0;

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}


int main(int argc, char **argv)
{

(void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao
int i = 0;
while(conta < 4){
   if(flag){
      alarm(3);                 // activa alarme de 3s
      flag=0; 
      while(1){
          if(UA == 1) break;
      }
   }
}
printf("Vou terminar.\n");

}