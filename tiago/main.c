#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int flag = 1, conta = 0;

void atende() {                   // atende alarme
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}

int main(){

    (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

    while (conta < 4){
        // if(flag){
          alarm(3);
          flag=0;
    //   }
    }
    
    
}

// int flag_alarm=0;

// void timeout_handler(int signal){    
//   printf("inside\n");    
//       flag_alarm=1;
// } 

// int main(){

// int contador_timeouts = 0;
// signal(SIGALRM, timeout_handler);  
// while(contador_timeouts<3){
//     flag_alarm=0;
//     printf("Reenviando pedido.\n");
//     // write(fd, frame, sizeof(frame));
//     alarm(1);    
//     // read(fd,frame_conflig[5]);
//     if (!flag_alarm)                  //o alarm nao disparou.
//           break;
//     else
//         contador_timeouts++;
// }

// // }
// #define TIMEOUT 3
// int flag_alarm=0, contador_timeouts = 3,tamanho_a_ler = 255;
// unsigned char frame_conflig[255];

// void timeout_handler(int signal){             
//    flag_alarm=1;
// } 
// int main(){

// signal(SIGALRM, timeout_handler);  


// int aux = 0;
// while(contador_timeouts<3){    
//     flag_alarm=0;    
//     printf("Reenviando pedido.\n");    
//     write(fd, frame, sizeof(frame));    
//     alarme(TIMEOUT);        
//     while (ret < tamanho_a_ler && !flag_alarm)     //Enquanto não leres tudo e o alarm não disparou    
//    {         
//         aux=read(fd,frame_conflig[5]);            //tentas ler alguma coisa.         
//         if(aux>0)                                 // o read pode-te retornar -1, não podes somar esse valor directamente ao ret.            
//              ret+=aux;    
//    }   
//     alarm(0);                                //desactiva o alarm, caso este ainda não tenha disparado.    
//     if (ret==tamanho_a_ler)                  //o alarm nao disparou.         
//              break;    
//     else        
//              contador_timeouts++;
// }
// }

// int ACTIVO = FALSE;
// int timeoutCounter =0;
// int MAX_REPETICOES =3;
// int timeoutFLAG = TRUE;




// void timeout_handler() {

//     if(ACTIVO){
//         printf("\nENTROU NO ALARME #%d\n", timeoutCounter);

//         if(MAX_REPETICOES == timeoutCounter) {
//             printf("\nLIMITE DE RETRANSMISSOES ALCANCADO\n");
//             exit(-1);
//         }


//         timeoutFLAG = TRUE;
//         timeoutCounter++;
//         alarm(TIMEOUT);
//     }
// } 


// void llread(int fd, unsigned char frame_resposta[]){   // Esta funcao recebe um array como argumento e coloca, nesse array, um a um, os elementos lidos na funcao read{

//     int tamanho_ler = 5;
//     int aux =0, ret =0;
//     char ele;
//     printf("Entrou no llread\n");

// //*****************************************




//     ACTIVO = TRUE;    
//     alarm(TIMEOUT);    //SINALIZAR //new


//     while (ret < tamanho_ler) {  //Enquanto não ler tudo e o alarm não disparou //eq->while(STOP == FALSE)      
//         aux=read(fd, &ele, 1);          //tenta ler alguma coisa.  
//         if(aux>0){             // o read pode retornar -1, não podes somar esse valor directamente ao ret.            
//             frame_resposta[ret] = ele;
//             ret+=aux;
//         }  
//     timeoutFLAG = FALSE;
//     ACTIVO = TRUE;        
//     alarm(TIMEOUT);
//    }   


//     if(Analisa_frames(frame_resposta) != 5){
//         ACTIVO = FALSE;    
//         timeoutCounter = 0;
//     }




// //*****************************************


//     return;
// }

