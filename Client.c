/***********************************************************************************
 * Author :   Jeremy Lowery
 * Professor: Dr. Lisa Frye
 * Creation Date: November 27, 2015
 * Due Date: Dec 6, 2015
 * Course: CSC328
 * Filename: Client.c
 * Compile with : make Client
 * Run: ./Client <destination> [Port]
 * Purpose: Client for Server.c. This is a TCP Client that interfaces with 
 * Server.c and downloads a file from said server with DOWNLOAD. Also you can
 * change directories with CD, and print the contents of the current directory
 * with DIR. DISCONNECT will close the client and HELP will display all
 * of the options.
 *
 **********************************************************************************/ 

#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#define SIZE sizeof(struct sockaddr_in)
#define IPADDR   "127.0.0.1"
#define MAXSIZE 512
 struct return_struc {
     
   int len;
   char  msg[MAXSIZE*10];
};
struct return_struc  recv_sol( int s);
void download_helper(int s, char * str);
int main(int argc,char* argv [])
{
  int sockfd;
  char host[100];
  char port[100];
  int reclen,errono;
  char input[100];
  memset(port, 0, sizeof(port));
  if (argc<2)
  {
    printf ("Usage: Client <Hostname| IP> [Port]\n");
    exit (0);
  }
  if (argc == 3)
  {
   // printf("%s\n", argv[2]);
    strcpy(port, argv[2]);
   // printf("port = %s\n", port);
     if (!isdigit(*port))
     {
       printf("The Port needs to be a number! You put : %s\n", port);
       exit(0);
     }
  }
  else 
    strcpy(port, "553322");

  char c[MAXSIZE];
  struct addrinfo hints, *res;
  void *ptr;
  char addrstr[MAXSIZE];
  char conbuf[MAXSIZE];
  int   total_recv;
  

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
   errono = getaddrinfo(strcpy(host,argv[1]),NULL, &hints, &res);
   printf("%s\n", host);
  if (errono !=0)
  {
    perror("getaddrinfo");
    printf("failed");
    exit(1);
  }
  while (res)
    {
      ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
   //   printf ("res is %s \n",&((struct sockaddr_in *)res->ai_addr)->sin_addr);
      inet_ntop (res->ai_family, ptr, addrstr, 100);
      res = res->ai_next;
    }
    printf ("%s\n",addrstr);
  struct sockaddr_in server = {AF_INET,htons(atoi(port)), addrstr};
  // convert and store the server's IP address
  server.sin_addr.s_addr = inet_addr(addrstr);
  // set up the transport end point
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("socket call failed");
      exit(-1);
    }   // end if
  // connect the socket to the server's address
  if (connect(sockfd, (struct sockaddr *)&server, SIZE) == -1)
    {
      perror("connect call failed");
      exit(-1);
    }   // end if
    if (recv(sockfd, c, sizeof(c), 0)==-1)
      perror("Recieving greeting failed:");
    write(1, c, strlen(c));
    while (1)
    {
     // printf("Top of the while loop\n");
      
      memset(c, 0, MAXSIZE);
      write(1,"Input: ",7 );
      fgets(c, MAXSIZE, stdin);
     // printf("Starting to write: %s\n", c);
      int ch=0;
      ch= write(sockfd, &c, strlen(c)+1);
      if ((ch==-1))
      {
        printf("test");
        perror("Client: sending");
	break;
      }//end sending
      if (ch<1)
        printf("ch was less than one\n");   
       //  printf("Before struc call\n");
         struct return_struc result=recv_sol(sockfd);
         printf("%s\n", result.msg);
        if (strncmp(result.msg, "GoodBye.", 8)==0)
        {
          printf ("%s\n", result.msg);
          printf("Shutting Down Client\n");
          close(sockfd);
          exit(0);
        }
        else if (strncmp(result.msg, "DOWNLOAD", 8)==0)
        {
           download_helper(sockfd, result.msg);
          
        }
       result.len = 0;
       memset(result.msg, 0, sizeof(result.msg));
    }//end while loop
      printf("Outside the while loop\n");
    close(sockfd);
    exit(0);
}  // end main
/************************************************************************
 *   Function name: recv_sol
 *   Description: Builds a buffer from the incomming socket
 *   Parameters: s - socket id
 *   Return Value: struct rs - contains the length of the message and the message
 *   Note: I just commented out all of the debugging stuf instead of deleting it
 *   because I am still going to work on this after I turn it in.
 *  ************************************************************************/
struct return_struc recv_sol( int s)
{
 int rc, length;
 char inc[MAXSIZE];
 int sockfd = s;
 int total=0;
 int chk= 0;
 char temp[MAXSIZE*10];
 int rcleft=0;
 uint16_t len=0, netlen=0;
 struct return_struc rs;
      // printf("test\n");
       memset(temp, 0, MAXSIZE*10);
       strcpy(temp, "");
       rc = recv(sockfd, &netlen,sizeof(netlen) , 0);
       if (rc ==-1)
       {
         perror("length revc:");
         return;
       }
      // printf("netlen = %d\n",netlen);
       len = ntohs(netlen);
       rs.len = len;
      // printf("rc = %d, len = %d \n", rc, len);
      while (total < len)
      {
        memset(inc, 0, MAXSIZE);
        rcleft = len- total;
      //  printf("rcleft = %d\n", rcleft);
        if (rcleft> MAXSIZE)
          rcleft = MAXSIZE;
      // printf("inc before transfer = %s\n", inc);
        rc =recv(sockfd,(char*)&inc, rcleft, 0);///////////////////////////STOP LOSING THIS
        
        if ((rc <0))
        {
           printf("ERROR ERROR FAILED TO READ\n");
           perror("Client: recieving");
           break; 
        }
          total += rc;
          inc[rc]= '\0';
          strcat(temp, inc);
         // inc[len]='\0';
       //  printf("strlen(inc) = %d, rc = %d (rcleftn = %d)! len = %d, total = %d inc = %s\n\n",strlen( inc), rc, rcleft, len, total, inc);
          if (rc==0)
          {
             printf("server disconnected\n");
             close (sockfd);
             exit (0);
             
          }

        
      }//End I/O loop
            temp[strlen(temp)] = '\n';
            //write(1, temp,strlen(temp));
            strcpy(rs.msg, temp);
  return rs;
}
/************************************************************************
 *   Function name: download_helper
 *   Description: Gathers information from the socket, then writes it to a file
 *   Parameters: s, str- s is the socket id, str is the filename
 *   Return Value: none
 *  ************************************************************************/
void download_helper(int s, char *str )
{
  int length ;
  char conbuf[MAXSIZE];
  char c[MAXSIZE] ;
  char inc[MAXSIZE];
  int sockfd = s;
  size_t rc;
  int rcv, wrt;
  FILE *fd;
  struct stat filestat;
  int res;
  char temp[MAXSIZE];
    memset(conbuf, 0, MAXSIZE);
    memset(c, 0, MAXSIZE);
    memset(inc, 0, MAXSIZE);
    //strcpy(c, str);
    strcpy(conbuf,str);
    length = strlen(conbuf);
    memmove(conbuf, conbuf+9, length );
    if (conbuf[strlen (conbuf) - 1] == '\n')
      conbuf[strlen (conbuf) - 1] = '\0';
    conbuf[strlen(conbuf)-1]= '\0';
   // printf("conbuf %s\n", conbuf);
    res = stat(conbuf, &filestat);
   // printf("res = %d", res);
    struct return_struc rs = recv_sol(sockfd);// reads the socket for the servers response on if the file exists
   // printf("%s\n", rs.msg);
    if (strstr(rs.msg, "If you REALLY") ==NULL)
    {
     
     printf("struc msg  %s\n", rs.msg);
     return;
    }
      if (res == -1)
        write(1, "Ready: ", 7);
      if (res ==0)
        write(1, "File exists, READY to overwrite or STOP to cancel:",50);
      fgets(c, MAXSIZE, stdin);
     
   // printf("Writing for DOWNLOAD\n");
    if (( write(sockfd, c, strlen(c))==-1))//Response for READY status
    {
      perror("Client: sending READY\n");
    }
      if (strncmp(c, "READY", 5)==0)
      {
      fd =fopen(conbuf, "wb+");
      if (fd==NULL)
      {
        perror("file open:");
        return;
      }
      printf("opened %s\n", conbuf); 
      int rcleft =0;
      uint16_t len, netlen;
      rc = recv(sockfd, &netlen, sizeof(netlen), 0);// call to get the length of the file
      if (rc ==-1)
      {
        perror("length revc:");
        return;
      }
      len = ntohs(netlen);
     // printf("rc = %d, len = %d\n", rc, len);
      while(rcleft < len)// WHILE FOR READYING SOCKET FOR DOWNLOAD
      {
        memset(inc, 0, MAXSIZE);   
      //  printf("rcleft(%d), rcv(%d), strlen(inc)(%d)\n",rcleft, rcv, strlen(inc));
      //  printf("Blocking for DOWNLOAD\n");
        rcv=recv(sockfd, inc,MAXSIZE, 0);////call to the socket for information
       
       // printf("inc = %s\n", &inc);
       // printf("%s\n",inc);
        if (rcleft<0)
        { 
          printf("Didn't read from socket\n");
          perror("recv for DOWNLOAD");
          fclose(fd);
         return;
        }  
        if (rcv==0|| len == 0)
        {
          printf("rcv == 0 len == 0\n");
          fclose(fd);
          return;
        }
        if (strlen(inc) == 0 && rcv == 0)
        {
          printf("Exiting becasue strlen==0 in 1st loop\n");
          //fclose(fd);
          //return;
        }
        //inc[rcv]=='\0';
        rcleft += rcv;
       // printf("rcleft(%d), rcv(%d), strlen(inc)(%d)\n",rcleft, rcv, strlen(inc));
        //printf("%s\n", inc);
        int offset = 0;
        while(offset< rcv)  // WHILE for writing FILE    
        {
          if (strncmp(inc, "DONE TRANSACTION!", 17)==0)
          {
            printf ("strcmp found the DONE! IN 2nd loop\n");
            fclose(fd);
            return;
          }
          if (len== 0)
          {
            printf("strlen == 0\n");
            break;
          }
          wrt= fwrite(&inc[offset],1,rcv-offset, fd );//////// WRITE ////////////////////////////////
        //  printf("wrt = %d\n", wrt);
          if (wrt<1)
          {
            printf("Not writing\n");
            fclose(fd);
            break;
          }
          offset+=rcv;
        //  printf ("offset(%d), wrt(%d), rcv(%d)\n", offset, wrt, rcv);
        }         
      }
     // printf("Do I ever get here?\n");
      fclose(fd);
      return;
     }         
     else if (strncmp(c, "STOP", 4)==0)
     {
       return;
     }  
     
       return;
} 
