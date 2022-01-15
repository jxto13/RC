#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{

  if (argc > 2)
    printf("**** To many arguments\n");
  
  char temp1[4][20];
//   sscanf(argv[1],"ftp://%s:1234@%s/",temp1,temp2);

  sscanf(argv[1],"ftp://%[^:]:%[^@]@%[^/]%[^\n]",temp1[0],temp1[1],temp1[2],temp1[3]);

  printf("1 = %s \n2 = %s \n3 = %s \nt4 = %s \n",temp1[0],temp1[1],temp1[2],temp1[3]);

  char temp2[6][20];

  sscanf("227 Entering Passive Mode (193,137,29,15,203,23).",
  "227 Entering Passive Mode (%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])",temp2[0],temp2[1],temp2[2],temp2[3],
  temp2[4],temp2[5]);
  char message[BUFSIZ] = "";
  uint16_t dataPort;

  printf("1 = %s \n2 = %s \n3 = %s \nt4 = %s \nt5 = %s \nt6 = %s \n",
  temp2[0],temp2[1],temp2[2],temp2[3],temp2[4],temp2[5]);

  for (int i = 0; i < 6; i++) {
    //   printf("%s\n",temp2[i]);
    if(i < 4){
        strcat(message,temp2[i]);
        if(i == 3){
            continue;
        }else{
            strcat(message,".");
        }
    }else{
        if(i == 4){
            dataPort = atoi(temp2[i]) * 256;
        }else{
            dataPort += atoi(temp2[i]);
        }
    }
  }

  printf("message %s\n",message);
  printf("port %d\n",dataPort);

}