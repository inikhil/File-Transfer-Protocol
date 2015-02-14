#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>

#define BACKLOG 20
#define MAXDATASIZE 1024

int sockfd , new_fd;      // server will start to listen on sockfd and accepts on new_fd 
char present_dir[128];

/* Function declarations for the functions requested by the client */

void server();
int send_message_to_client(char *message, int length, int fd);
int get_message_from_client(char *variable, int fd, int size);
void recv_file_from_client(char *argument);
void send_file_to_client(char * argument);
void recv_mfile_from_client(char *argument);
void send_mfile_to_client(char * argument);
void recv_mfilew_from_client(char *argument);
void send_mfilew_to_client(char * argument);
void change_dir(char * argument);
void ls();
void pwd();
void cd();

// To get the IP Address
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* sigchld_handler is used to wait for the child process to complete using 
waitpid .If waitpid returns 0 it means there are no children to wait on */

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}



int main(int argc, char *argv[])
{
	// argc is taken as 2 because we need the server port as well
	if (argc!=2){
		printf("Usage: ./a.out port");
	}

	struct sockaddr_in server_addr , client_addr; /* connector and server address information */
	int sin_size;
	char s[INET6_ADDRSTRLEN];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("unable to create a socket");
		exit(1);
	}

	// Specifications of server socket
	int PORT=atoi(argv[1]);
	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;         /* host byte order */
	server_addr.sin_port = htons(PORT);       /* short, network byte order */
	server_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
	bzero(&(server_addr.sin_zero), 8);        /* zero the rest of the struct */

	// inet_ntop converts a numeric address into text of string suitable for presentation

	inet_ntop(server_addr.sin_family,
			get_in_addr((struct sockaddr *)&server_addr),
			s, sizeof s);
	printf("Server is running on port %s\n",argv[1]);

    // bind() assigns a local socket address to a socket identified by descriptor socket 

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("Sorry unable to bind to the given port number provided...please check if it is really available\n");
		exit(1);
	}

    // Tell a socket to listen for incoming connections 
	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("unable to listen");
		exit(1);
	}

	while(1) 
	{  
		sin_size = sizeof(struct sockaddr_in);
	    
	    /* accept() shall extract the first connection on the queue of pending connections
	       create a new socket with the same socket type protocol and the address family 
	       as the specified socket and allocate a new file descriptor for that socket */

		if ((new_fd = accept(sockfd, (struct sockaddr *)&client_addr,&sin_size)) == -1) 
		{
			perror("accept");

			continue;
		}


		printf(" Hi!! server: got connection from %s\n", inet_ntoa(client_addr.sin_addr));

		/* fork() creates a new process which becomes a child process of the caller
		   It is used to handle the concurrent server mode */

		if(!fork()) 
		{
			printf(" ACCEPT AND FORK\n");
			server();
			close(new_fd);
			printf("connection closed for %d\n",new_fd);
			exit(0);
		}
		close(new_fd);  /* parent doesn't need this*/
     
	}
		
	return 0;
}

void server()
{
	/* popen() function executes a command and pipes the executes command to 
	the calling program returning a pointer to the stream which can be used 
	by calling program to read and write to the pipe */

	FILE *output =popen("pwd", "r");
	if(!output)
	{
		printf("permission denied ");
		return ;
	}
	
	/* fscanf() reads data from the output and stores them in present_dir in the string format */ 
	fscanf(output, "%[^\n]", present_dir);
	pclose(output);	

	while(1)
	{
		char command[128]="";
		char argument[128]="";
		int bytes_recvd=0;
		int mes_len;

		// recv() returns the number of bytes actually read into the buffer
		bytes_recvd=recv(new_fd, &mes_len, 4, 0);

		if( (bytes_recvd=get_message_from_client(command, new_fd, mes_len))==128)
		{
			continue;
		}
		printf( "command = %s, client fd = %d\n ", command, new_fd);

// start of ls comand : list contents of server current directory =========================================================================
		if(strcmp(command, "ls")==0)
		{
			char data[1024];
			char lsresult[1024];
			char comm[128] = "ls";
			int len;
			bytes_recvd = recv(new_fd, &len, 4, 0);
			/* If user has entered invalid argument , we have received length as 0.
			So the server sends a message to the client saying that it is an invalid command */

			if(len!=0)
			{
				strcat(comm," ");
				bytes_recvd = recv(new_fd, argument, len,0);
				strcat(comm, argument);
				char in_arg[128]="Invalid Argument Usage : ls";
				send(new_fd,in_arg,strlen(in_arg),0);
			}
			else
			{
				argument[0]='\0';
				strcat(comm, " 2>&1"); 
				/* 1 denotes standard output and 2 denotes standard error , 
				   therefore , (2>&1) says to send standard error to where standard output is beng directed */
				FILE *output = popen(comm, "r");
				/* popen() function executes a command and pipes the executes command to 
				the calling program returning a pointer to the stream which can be used 
				by calling program to read and write to the pipe */

				if(!output) 
				{
					printf("permission denied\n");
					return;
				}
				int count = 0;
				char y;
				/* We scan the FILE in a character array to count
				   length of file */
				while(fscanf(output, "%c", &y)!=EOF)
				{
					count++;
				}
				pclose(output);
				// We send the total size of the file to the client and then re-open the file to send data 
				send(new_fd, &count, 4, 0);
				output = popen(comm, "r");
				if (!output) 
				{
					printf("permission denied\n");
					return;
				}

				int bytes_to_read;
				int i=0;
				int j=0;
				int t = (int)count/(MAXDATASIZE-1);
				// Here t denotes the number of iterations required to send the data 
				if(count!=0)
				for(j=0; j<=t; j++)
				{
					bytes_to_read = count>(MAXDATASIZE-1)?(MAXDATASIZE-1):count;
					for(i=0; i<bytes_to_read; i++)
					{
						fscanf(output, "%c", &lsresult[i]);

					}
					lsresult[i]='\0';
					/* We copy the contents of lsresult to the data so that 
					we can use data-block to send to client */

					sprintf(data,"%s",lsresult);
					len = strlen(data);
					send(new_fd, &len, 4, 0);
					send(new_fd, data, len, 0);

					count = count - bytes_to_read;
				}
				pclose(output);
			}
			
		}
// end =======================================================================================================================================
	
// start of cd command : changes server current directory ============================================================================ 
		else if(strcmp(command, "cd")==0)
		{
			int len;
			bytes_recvd = recv(new_fd, &len, 4, 0);

			// If length of the argument is non-zero then we receive the argument
			if(len!=0)
			{
				bytes_recvd = recv(new_fd, argument, len, 0);
			}
			else
			{
				argument[0]='\0';
			}

			char s[128] = "";
			strcat(s,present_dir);
			chdir(argument);           // changes directory if valid argument is given
			getwd(present_dir);   
			/* getwd(present_dir) determines an absolute pathname of the current working directory 
			of the calling process and copy a string containing that pathname to the present_dir */     
			char data[MAXDATASIZE];

            /* We have three cases to deal with basically. 
               1. The server current directory is changed 
               2. The server current directory is changed to itself
               3. Invalid command   */

			if(strcmp(s,present_dir)!=0)
			 sprintf(data,"server current directory changed to : %s\n",present_dir);
			else if (strcmp(argument,s)==0)
				sprintf(data,"Server already present in this directory\n");
			else 
				sprintf(data,"Invalid argument Usage: cd [Enter a valid path here]\n");

			len = strlen(data);
			send(new_fd,&len,4,0);
			send(new_fd,data,len,0);
		}
// end ================================================================================================================================
	
// start of pwd command : displays path of server current directory =======================================================================
		else if(strcmp(command,	"pwd")==0)
		{
			char data[1024];
			getwd(present_dir);
			sprintf(data, "server present directory is : %s\n", present_dir);
			int len,length;
			char res;
			bytes_recvd=recv(new_fd,&length,4,0);
			
			if(length!=0)
			{
				/* this part checks for invalid command (the one with non-zero 
				argument) and sends the message reporting the same to client */
				char in_arg[128]="Invalid argument Usage: pwd\n";
				send(new_fd,in_arg,strlen(in_arg),0);
			}
			else{
				len = strlen(data);
				/* We first send the length of data and then data itself to the 
				client .Note that here data tells us the present directory */
				send(new_fd, &len, 4, 0);
				send(new_fd, data, len, 0);
			}
		}

// end ====================================================================================================================================

// start of gets command : client will download files from server ============================================================================
		else if(strcmp(command, "gets")==0)
		{
			int len;
			int err = 0;
			char err_msg[128]="";
			bytes_recvd = recv(new_fd, &len, 4, 0);
			// If argument length is not zero then we hAve to take the argument
			if(len!=0)
			{
				bytes_recvd=recv(new_fd, argument, len, 0);
			}
			else
			{
				continue;
			}
			/* file is opened in read mode and if file does not exist server sends the error
			msg to the client so that client can understand*/
			FILE *fp=fopen(argument, "r");
			if(fp==NULL)
			{
				err=1;
				send(new_fd, &err, 4, 0);
				strcpy(err_msg, "file does not exists in the server present directory\n");
				len=strlen(err_msg);
				send(new_fd, &len, 4, 0);
				send(new_fd, err_msg, len, 0);
				continue;
			}
			//else we send err=0 that means no error has occured
			else
			{
				err=0;
				send(new_fd, &err, 4, 0);
			}
			/* if client cannot open a file to write it sens err=1 to the server so that server 
			can close it's file*/
			recv(new_fd, &err, 4, 0);
			if(err==1)
			{
				fclose(fp);
				continue;
			}
			
			int size_of_file=0;
			char x;
			// Now the server calculates the length of the file and sends it to the client//
			while(fscanf(fp, "%c", &x)!=EOF)
			{
				size_of_file++;
			}
			rewind(fp);
			send(new_fd, &size_of_file, 4, 0);
			/* After rewinding to the begining of the file the server checks if the size is
			greater than 0 it starts sending max data that can be transferred to the  client*/ 
			if(size_of_file>0)
			{
				char result[MAXDATASIZE], data[MAXDATASIZE];
				int bytes_to_read;
				int i=0;
				int j=0;
				int t= (int)size_of_file/(MAXDATASIZE);
				for(j=0; j<=t; j++)
				{
					bytes_to_read=size_of_file>(MAXDATASIZE)?(MAXDATASIZE):size_of_file;
					/* bytes_to_read is stored in temporary variable result at each iteration and
					then result is copied into data to transfer to the client*/
					for(i=0; i<bytes_to_read; i++)
					{
						fscanf(fp, "%c", &result[i]);
					}
	
    					len = bytes_to_read;
					send(new_fd, &len, 4, 0);
					send(new_fd, result, len, 0);
					// size of the file is decreased by remaining bytes to read
					size_of_file = size_of_file - bytes_to_read;
				}
			}
			fclose(fp);  
		}
// end ====================================================================================================================================

// start of puts command : client will upload files to server ===================================================================================================
		else if(strcmp(command, "puts")==0)
		{
			int len;
			int err = 0;
			char err_msg[128]="";
			bytes_recvd = recv(new_fd, &len, 4, 0);
			// If argument length is not zero then we hAve to take the argument
			if(len!=0)
			{
				bytes_recvd=recv(new_fd, argument, len, 0);
			}
			else
			{
				continue;
			}

			// receive file from client if it exists otherwise err=1 is received
			int err_no;
			recv(new_fd, &err_no, 4, 0);
			if(err_no==1)
			{
				recv(new_fd, &len, 4, 0);
				recv(new_fd, err_msg, len, 0);
				printf("%s\n", err_msg);
				continue;
			}
			/* open the file to write data , if file cannot be opened to write then
			send msg to the client to close it's file*/
			FILE* fp = fopen(argument, "w");
			if(fp ==NULL)
			{
				printf(" file could not be opened for reading or writing..... check the permission of the frile which you want to access\n");
				err_no = 1;
				send(new_fd, &err_no, 4, 0);
				continue;
			}
			else
			{
				err_no=0;
				send(new_fd, &err_no, 4, 0);
			}			
			int size_of_file;
			// receives the size of file which will be written to it
			recv(new_fd, &size_of_file, 4, 0);
			printf("size of file we have received is : %d\n ", size_of_file);
			if(size_of_file>0)
			{
				int i=0;
				int t = (int)size_of_file/(MAXDATASIZE);
				// for each iteration write the received data to a file
				for(i=0;i<=t;i++)
				{
					char result[1024];
					int bytes_recv = recv(new_fd, &len, 4, 0);
					bytes_recv=recv(new_fd, result, len, 0);
					fwrite(result, bytes_recv, 1, fp);
					//write(1, result, bytes_recv);
				}
			}
			fclose(fp);

		}
// end =======================================================================================================================================

// start of mgets: client will download multiple files from server=============================================================================
		else if(strcmp(command, "mgets")==0)
		{
			/* The while loop in the client basically calls this function as many files 
			have been entered on the client side while the server behaves exactly same as 
			gets operation of the server side*/
			int len,m;
			int err = 0;
			char err_msg[128]="";
			bytes_recvd = recv(new_fd, &len, 4, 0);
			if(len!=0)
			{
				bytes_recvd=recv(new_fd, argument, len, 0);
			}
			else
			{
				continue;
			}
			FILE *fp=fopen(argument, "r");
			if(fp==NULL)
			{
				err=1;
				send(new_fd, &err, 4, 0);
				strcat(err_msg, " All file does not exists in the server present directory\n");
				len=strlen(err_msg);
				send(new_fd, &len, 4, 0);
				send(new_fd, err_msg, len, 0);
				continue;
			}
			else
			{
				err=0;
				send(new_fd, &err, 4, 0);
			}
			recv(new_fd, &err, 4, 0);
			if(err==1)
			{
				fclose(fp);
				continue;
			}
			
			int size_of_file=0;
			char x;
			while(fscanf(fp, "%c", &x)!=EOF)
			{
				size_of_file++;
			}
			rewind(fp);
			send(new_fd, &size_of_file, 4, 0);
			if(size_of_file>0)
			{
				char result[MAXDATASIZE], data[MAXDATASIZE];
				int bytes_to_read;
				int i=0;
				int j=0;
				int t= (int)size_of_file/(MAXDATASIZE);
				for(j=0; j<=t; j++)
				{
					bytes_to_read=size_of_file>(MAXDATASIZE)?(MAXDATASIZE):size_of_file;
					for(i=0; i<bytes_to_read; i++)
					{
						fscanf(fp, "%c", &result[i]);
					}
	
    					len = bytes_to_read;
					send(new_fd, &len, 4, 0);
					send(new_fd, result, len, 0);
	
					size_of_file = size_of_file - bytes_to_read;
				}
			}
			fclose(fp);  
		}

// end ==========================================================================================================================================

//start of mputs: client will upload multiple files to server ===================================================================================

		else if(strcmp(command, "mputs")==0)
		{
			/* The while loop in the client basically calls this function as many files 
			have been entered on the client side while the server behaves exactly same as 
			puts operation of the server side*/
			int len;
			int err = 0;
			char err_msg[128]="";
			bytes_recvd = recv(new_fd, &len, 4, 0);
			if(len!=0)
			{
				bytes_recvd=recv(new_fd, argument, len, 0);
			}
			else
			{
				continue;
			}

			// recieve file from client if it exists
			int err_no;
			//char err_msg[128];
			recv(new_fd, &err_no, 4, 0);
			if(err_no==1)
			{
				recv(new_fd, &len, 4, 0);
				recv(new_fd, err_msg, len, 0);
				printf("%s\n", err_msg);
				continue;
			}
		
			FILE* fp = fopen(argument, "w");
			if(fp ==NULL)
			{
				printf(" file could not be opened for reading or writing..... check the permission of the frile which you want to access\n");
				err_no = 1;
				send(new_fd, &err_no, 4, 0);
				continue;
			}
			else
			{
				err_no=0;
				send(new_fd, &err_no, 4, 0);
			}			
			int size_of_file;
			recv(new_fd, &size_of_file, 4, 0);
			printf("size of file we have received is : %d\n ", size_of_file);
			if(size_of_file>0)
			{
				int i=0;
				int t = (int)size_of_file/(MAXDATASIZE);
				for(i=0;i<=t;i++)
				{
					char result[1024];
					int bytes_recv = recv(new_fd, &len, 4, 0);
					bytes_recv=recv(new_fd, result, len, 0);
					fwrite(result, bytes_recv, 1, fp);
					//write(1, result, bytes_recv);
				}
			}
			fclose(fp);

		}

// end ==========================================================================================================================================


// start of exit command : exit connection from client ===============================================================================
		else if(strcmp(command, "exit")==0)
		{
    			return;
		}
// end ===============================================================================================================================

		else
		{
			//unrecognized command client will handle with it;
		}
	}
}

/* bytes return if it was received and -1 if there is error*/   
int get_message_from_client(char *variable, int fd, int size)
{
	int bytes_recvd;
	bytes_recvd = recv(fd, variable, size, 0);
	return bytes_recvd;
}

void recv_file_from_client(char *argument)
{
	printf(" server is recieving");
}
































































