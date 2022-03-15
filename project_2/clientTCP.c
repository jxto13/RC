/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>
// #include "ftpclient.h"

#define SERVER_PORT 21

// #define SERVER_ADDR "193.137.29.15"

#define FTP_BAD -2 /* Catch all, socket errors etc. */

char* filename;

int get_line(int s, char *buf, unsigned buf_size);

int get_reply(int s, char *replyMessage);

int send_cmd(int s, char *msgbuf);

int receive_file(int data_s);

int main(int argc, char **argv)
{

  if (argc > 2)
    printf("**** To many arguments\n");

  char temp1[4][BUFSIZ];

  sscanf(argv[1], "ftp://%[^:]:%[^@]@%[^/]/%[^\n]", temp1[0], temp1[1], temp1[2], temp1[3]);

  // user = temp1[0]
  // pass = temp1[1]
  // hostname = temp1[2]
  // path = temp1[3]

  filename = basename(temp1[3]);

  struct hostent *h;

 
  if ((h = gethostbyname(temp1[2])) == NULL) {
          herror("gethostbyname()");
          exit(-1);
      }

  printf("Host name  : %s\n", h->h_name);
  printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));

  int sockfd;
  struct sockaddr_in server_addr;
  size_t bytes;

  /*server address handling*/
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *) h->h_addr))); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(SERVER_PORT);            /*server TCP port must be network byte ordered */

  /*open a TCP socket*/
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket()");
    exit(-1);
  }
  /*connect to the server*/
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("connect()");
    exit(-1);
  }

  // mandar um byte para receber a mensgem de bem vindo do servidor
  send_cmd(sockfd, "\n");

  char replyMessage[BUFSIZ];
  // resposta
  get_reply(sockfd, replyMessage);

  // escrever o user e a palavra pass
  char userCMD[BUFSIZ] = "";
  strcat(userCMD, "USER ");
  strcat(userCMD, temp1[0]);
  strcat(userCMD, "\n");

  // escrever o user e a palavra pass
  send_cmd(sockfd, userCMD);
  get_reply(sockfd, replyMessage);

  // escrever o user e a palavra pass
  char passCMD[BUFSIZ] = "";
  strcat(passCMD, "PASS ");
  strcat(passCMD, temp1[1]);
  strcat(passCMD, "\n");

  send_cmd(sockfd, passCMD);
  get_reply(sockfd, replyMessage);

  // entrar em modo passivo
  send_cmd(sockfd, "PASV\n");
  get_reply(sockfd, replyMessage);

  // Getting ip address and port
  char dataTransferIP[BUFSIZ] = "";
  uint16_t dataPort;
  char temp2[6][BUFSIZ];

  sscanf(replyMessage, "227 Entering Passive Mode (%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])", temp2[0], temp2[1], temp2[2], temp2[3],
         temp2[4], temp2[5]);

  for (int i = 0; i < 6; i++){
    if (i < 4){
      strcat(dataTransferIP, temp2[i]);
      if (i == 3){
        continue;
      }else{
        strcat(dataTransferIP, ".");
      }
    }else{
      if (i == 4){
        dataPort = atoi(temp2[i]) * 256;
      }else{
        dataPort += atoi(temp2[i]);
      }
    }
  }

  printf("Data Transfer IP: %s\n", dataTransferIP);
  printf("Data Transfer Port: %d\n", dataPort);

  //criar um novo socket
  int sockfd_download;
  struct sockaddr_in server_addr_download;

  /*server address handling*/
  bzero((char *)&server_addr_download, sizeof(server_addr_download));
  server_addr_download.sin_family = AF_INET;
  server_addr_download.sin_addr.s_addr = inet_addr(dataTransferIP); /*32 bit Internet address network byte ordered*/
  server_addr_download.sin_port = htons(dataPort);                  /*server TCP port must be network byte ordered */

  /*abrir o TCP socket*/
  if ((sockfd_download = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket()");
    exit(-1);
  }

  /*connect to the server*/
  if (connect(sockfd_download, (struct sockaddr *)&server_addr_download, sizeof(server_addr_download)) < 0){
    perror("connect()");
    exit(-1);
  }else{
    printf("connection successful\n");
  }

  // escrever o user e a palavra pass
  char pathCMD[BUFSIZ] = "";
  strcat(pathCMD, "retr ");
  strcat(pathCMD, temp1[3]);
  strcat(pathCMD, "\n");

  send_cmd(sockfd, pathCMD);

  get_reply(sockfd, replyMessage);

  // download file
  receive_file(sockfd_download);


  get_reply(sockfd_download, replyMessage);

  get_reply(sockfd, replyMessage);

  if (close(sockfd_download) < 0)
  {
    perror("close()");
    exit(-1);
  }

  if (close(sockfd) < 0)
  {
    perror("close()");
    exit(-1);
  }
  return 0;
}

// /* Read one line from the server, being careful not to overrun the
//    buffer. If we do reach the end of the buffer, discard the rest of
//    the line. */
int get_line(int s, char *buf, unsigned buf_size)
{
  int eol = 0;
  int cnt = 0;
  int ret;
  char c;

  while (!eol)
  {
    ret = read(s, &c, 1);

    if (ret != 1)
    {
      printf("read %s\n", strerror(errno));
      return FTP_BAD;
    }

    if (c == '\n')
    {
      eol = 1;
      continue;
    }

    if (cnt < buf_size)
    {
      buf[cnt++] = c;
    }
  }
  if (cnt < buf_size)
  {
    buf[cnt++] = '\0';
  }
  else
  {
    buf[buf_size - 1] = '\0';
  }
  return 0;
}

// /* Read the reply from the server and return the MSB from the return
//    code. This gives us a basic idea if the command failed/worked. The
//    reply can be spread over multiple lines. When this happens the line
//    will start with a - to indicate there is more*/
int get_reply(int s, char *replyMessage)
{
  char buf[BUFSIZ];
  int more = 0;
  int ret;
  int first_line = 1;

  do
  {

    if ((ret = get_line(s, buf, sizeof(buf))) < 0)
    {
      return (ret);
    }

    printf("FTP: %s\n", buf);
    strcpy(replyMessage, buf);

    if (first_line)
    {
      first_line = 0;
      more = (buf[3] == '-');
    }
    else
    {
      if (isdigit(buf[0]) && isdigit(buf[1]) && isdigit(buf[2]) && buf[3] == ' ')
      {
        more = 0;
      }
      else
      {
        more = 1;
      }
    }
  } while (more);

  char code[3];
  sprintf(code, "%c%c%c", buf[0], buf[1], buf[2]);

  // return (buf[0] - '0');
  return atoi(code);
}

/* Send a command to the server */
int send_cmd(int s, char *msgbuf)
{
  int len;
  int slen = strlen(msgbuf);

  if ((len = write(s, msgbuf, slen)) != slen)
  {
    printf("write error %s\n", strerror(errno));
    return FTP_BAD;
  }

  printf("----------- \n message - %s bytes write %d \n----------- \n", msgbuf, len);

  return 1;
}

int receive_file(int data_s){
  FILE *file;
  int bytes_received = 0;
  char temp[BUFSIZ];
  int n;

  
  file = fopen(filename, "wb");

  if (file == NULL){
    perror("Error in open file");
  }

  // read data in from server and write these into new file
  n = read(data_s, temp, 1);
  while (n > 0){
    bytes_received += n;
    fwrite(temp, 1, n, file);
    n = read(data_s, temp, 1);
  }
  if (n == 0){
    fclose(file);
  } else if (n < 0) {
    fprintf(stderr, "Error in read()");
  }
  return 0;
}
