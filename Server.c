/***********************************************************************************
 *
 * Author: Jeremy Lowery
 * Professor: Dr. Lisa Frye
 * Creation Date: November 17, 2015
 * Due Date:  Dec 6
 * Assignment: tcp server application
 * Filename: Server.c
 * Purpose: Download server that takes specific commands and
 * allows for basic downloading and directory checking on the server machine
 * compile: make Server
 * Run: ./Server [port]
 *
 **********************************************************************************/ 

#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#define SIZE sizeof(struct sockaddr_in)
#define MAXSIZE 1024
void handle_child_input();
void handle_child_out(char *msg);
void handle_dir_out();
void handle_download_out(char *msg);
void handle_cd_out(char *msg);
void catcher(int sig);
void child_handler(int sig);
int sockfd;
int newsockfd;

int main(int argc, char* argv[])
{
  char c;

  int pid;
  char port[10];
  static struct sigaction act;
  act.sa_handler = catcher;
  sigfillset(&(act.sa_mask));
  sigaction(SIGPIPE, &act, NULL);
  struct sigaction sh;
  (void) signal(SIGINT, catcher);
  sh.sa_handler = &child_handler;
  sh.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  memset (port, 0, 10);
  if (sigaction(SIGCHLD, &sh, 0) == -1)
  {
    perror(0);
    exit(1);
  }
  if (argc != 2 )
  {
    strcpy(port, "553322");
  
  }
  else
    strcpy(port, argv[1]);
    printf("%s\n", port);
  struct sockaddr_in server = {AF_INET,htons(atoi(port)), INADDR_ANY};
  // set up the transport end points
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("socket call failed");
      exit(-1);
    }   // end if 

  // bind and address to the end point
  if (bind(sockfd, (struct sockaddr *)&server, SIZE) == -1)
    {
      perror("bind call failed");
      exit(-1);
    }   // end if bind

  // start listening for incoming connections
  if (listen(sockfd, 5) == -1)
    {
      perror("listen call failed");
      exit(-1);
    }   // end if


  for (;;)
    {
      // accept connection
      if ((newsockfd = accept(sockfd, NULL, NULL)) == -1)
	{
	  perror("accept call failed");
	  exit(-1);
	}   // end if

      // spawn a child to deal with the connection
      if ((pid = fork()) == -1)
	{
	  perror("fork failed");
	  exit(-1);
	}   // end if

      if (pid == 0)
	{
	  handle_child_input();
	}   // end child process

      // parent doesn't need newsockfd
      close(newsockfd);
    }   // end for
}   // end main


/***********************************************************************************
 * Function Name: catcher
 * Description: catches signals and shuts the server down while closing the open socket
 * Parameters: sig - signal that was given
 * Return Value: none
 * **************************************************************************************/
void catcher(int sig)
{
  printf("Shutting Server Down\n");
  close(sockfd);
  exit(0);
}   // end function catcher
/***********************************************************************************
 * Function Name: child_handler
 * Description: Handles waiting for child processes to end.
 * Parameters: sig - the signal that there is a stopped child process
 * Return Value: none
 * **************************************************************************************/
void child_handler(int sig)
{
  while (waitpid(( pid_t) (-1), 0, WNOHANG) > 0) {}
}

/***********************************************************************************
 * Function Name: handle_child_input
 * Description: This is the main logic component of the program. Handles the incomming
 * data from the socket and sends the data where it needs to go. Commands it handles:
 * DOWNLOAD, CD, DIR, HELP, anything else will get a return value of Not a valid input,
 * try again.
 * Parameters: none
 * Return Value: none
 * **************************************************************************************/
void handle_child_input()
{
  char buf[MAXSIZE];
  char incbuff[MAXSIZE];
  size_t rc;
  int n;
  close (sockfd);
  strcpy(buf, "You are connected. Hello User. Use HELP to see a list of commands\n");
  if (write(newsockfd, buf, strlen(buf)+1)==-1)
     perror("Greeting failed:");
  while(1)
  {
     memset (incbuff, 0, MAXSIZE);
     
     if (rc = recv(newsockfd, incbuff, MAXSIZE, 0)<1)
     {
      if (rc == -1)
      {
        perror("Server: recieving"); 
        continue;
      }
      printf("client disconnect\n");
      close(newsockfd);
      exit(0);
     }
     else
     {
       write(1, incbuff, strlen(incbuff));       
         if (strncmp(incbuff, "CD", 2)== 0)
         {
           handle_cd_out(incbuff);
          // killstream();
         }
         else if (strncmp(incbuff, "DIR", 3)== 0)
         {
           printf("I made it to the else if");
           handle_dir_out();
         //  killstream();
         }
         else if (strncmp(incbuff, "DOWNLOAD", 8)== 0)
         {
           handle_download_out((char *)&incbuff);
          // killstream();
         }
         else if (strncmp(incbuff, "DISCONNECT", 10)== 0)
         {
           printf("DISCONNECT was put in\n");
           strcpy(buf, "GoodBye.");
           handle_child_out((char *)&buf);
           close(newsockfd);
           exit(0);
         }
         else if(strncmp(incbuff, "HELP", 4)== 0)
         {
           printf("HELP was put in\n");
           strcpy(buf, "The list of commands are: CD,DIR,DOWNLOAD,DISCONNECT");
           handle_child_out((char *)&buf);
         //  killstream();         
         }
         else
         {
           strcpy(buf, "Not a valid input, try again.");
           handle_child_out((char *)&buf);
          // killstream();
         }
     }
  }
}
/***********************************************************************************
 * Function Name: handle_child_out
 * Description: Sends the response from the server to the client. First sends the length
 * of the response then the response itself
 * Parameters: msg - the response to be sent
 * Return Value: none
 * **************************************************************************************/
void handle_child_out(char *msg)
{
   char msgh[MAXSIZE];
   memset(msgh, 0, MAXSIZE);
   strcpy(msgh, msg);
   uint16_t len = strlen(msgh);
   uint16_t netlen = htons(len);
   printf("len = %d\n", len);
   if ( write (newsockfd,&netlen , sizeof(netlen))==-1)
    {
      perror("Server: sending");
    }
   if ( write (newsockfd, (char *)msgh, strlen(msgh))==-1)
    {
      perror("Server: sending");
    }
    printf("Sent something\n");
    //printf("%s\n", msgh);
}
/***********************************************************************************
 * Function Name: handle_dir_out
 * Description: Builds the buffer that contains the list of files in the current working
 * directory.
 * Parameters: none
 * Return Value: none
 * **************************************************************************************/
void handle_dir_out()
{
  DIR *d;
  struct dirent *dir;
  char buf[MAXSIZE*10];
  int count= 0;
  int errnum;
  //printf("In handle_dir_out\n");
  d= opendir(".");
  if (d)
  {
    strcpy(buf, "Directory: \n");
    while ((dir = readdir(d)) !=NULL)
    {
      count+=1;
     // printf("%s\n", dir->d_name);
   //   if (strlen(buf)>=1000)
   //   {
   //    handle_child_out(buf);
   //    memset(buf, 0, MAXSIZE);
   //   }
      if (count != 1)
      if (count%2==0)
        strcat(buf, "\n");
      else
        strcat(buf, "      ");
      strcat(buf, dir->d_name);
    //  printf("buff size = %d\n", strlen(buf));
    }
    strcat(buf, "\nEnd of Directory List.");
    closedir(d);
    handle_child_out(buf);
    return;
  }
  else 
  {
    errnum= errno;
    handle_child_out(strerror(errnum));
    closedir(d);
    return;
  }
}
/***********************************************************************************
 * Function Name: handle_download_out
 * Description: Opens, reads, sends a file to the client, or an error message.
 * Parameters: msg - the name of the file to open.
 * Return Value: none
 * **************************************************************************************/
void handle_download_out(char *msg)
{
  char msgh[MAXSIZE];
  char msgr[MAXSIZE];
  char buff[MAXSIZE];
  int  errnum, red, wrt;
  char query[MAXSIZE];
  FILE *opn;
  char cwd[MAXSIZE];

  memset(msgh, 0, MAXSIZE);
  strcpy(msgr, msg);
  if (msgh[strlen(msgh)-1] == '\n')
    msgh[strlen(msgh)-1] == '\0';
  strcpy(msgh, msg);
  msgh[strlen(msgh)-1]= '\0';
  memmove(msgh, msgh+9, strlen(msgh));
  printf("%d\n",strlen( msgh));
  printf("%s\n", msgh);


  printf("Opening %s\n",msgh);
  if ((opn= fopen(msgh, "rb"))==NULL)
  {
    perror("open for files");
    errnum= errno;
    handle_child_out(strerror(errnum));
    return;
  }
  else
  {
    strcpy(query, "If you REALLY want to Download this file, type READY!");
    handle_child_out(msgr);
    handle_child_out(query);
    memset(query, 0, sizeof(query));
    if (recv(newsockfd, query, sizeof(query), 0)!= -1)
    {
      printf("%s\n", query);
      if (strncmp(query, "READY", 5)!= 0)
      { printf("bailing out\n");
       return;
      }
    }
    else
    perror("recv for READY:");

  }
  fseek(opn, 0, SEEK_END);
  long filesize = ftell(opn);
  rewind(opn);
  while(!feof(opn))
  {
   printf("Inside the file read loop");

   printf("%d\n", filesize);
   red = fread(buff,1, sizeof(buff), opn);
   if (red<0)
   {
     printf("Couldn't read");
     fclose(opn);
     continue;
   } 
   printf("buff = %s\n", buff);
   if (red ==0)
   {
     printf("Zero on the read for file loop");
     break;
   }
   printf("strlen = %d\n", strlen(buff));
   
   uint16_t netlen = htons(filesize);
   if ( write (newsockfd,&netlen , sizeof(netlen))==-1)
    {
      perror("Server: sending");
    }
  int offset= 0;
  while(offset<red)
  {
   printf("Inside the Write loop");
   wrt = write(newsockfd, &buff[offset], red- offset);
    if (wrt<0)
    {
      printf("wrt was 0");
      perror("write:");
      errnum = errno;
      handle_child_out(strerror(errnum));
      fclose(opn);
      return;
    }
    offset += red;
  }
 }
 fclose(opn);
}

/***********************************************************************************
 * Function Name: handle_cd_out
 * Description: Changes the current working directory on the Server.
 * Parameters: msg - path to the new directory
 * Return Value: none
 * **************************************************************************************/
void handle_cd_out(char *msg)
{
  char msgh[MAXSIZE];
  int err;
  char buff[MAXSIZE];
  char cwd[MAXSIZE];
  if (msgh[strlen(msgh)-1] == '\n')
    msgh[strlen(msgh)-1] = '\0';
  strcpy(msgh, msg);
  msgh[strlen(msgh)-1] = 0;
  memmove(msgh, msgh+3, strlen(msgh));
  printf("%d\n",strlen( msgh));
  printf("%s\n", msgh);
  if (err= chdir((char *)msgh)!= 0)
  {
    perror("chdir:");
    err = errno;
    handle_child_out((char *) strerror(err));
    return;
  }
  else
  {
   // strcpy(buff, "You have sucsessfully moved directories.");
    if (getcwd(cwd, sizeof(cwd)) !=NULL)
    {
      handle_child_out(cwd);
    }
    else
    {
      perror("gercwd :");
   // handle_dir_out();
      return;
    }
  }
}
