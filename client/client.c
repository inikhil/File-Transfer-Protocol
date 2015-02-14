
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

#define MAXDATASIZE 1024

int sockfd;  
char present_dir[128];

/* Function declarations for uploading and downloading files , 
changing the directory of the client and server & listing the 
files of current directory of client and server */

void client();
void send_file_to_server(int);
void recv_file_from_server(char []);
void lls();
void lcd();
void lpwd();
void ls();
void cd();
void getfile(char []);
void putfile(char []);
void mgetfile(char []);
void mputfile(char []);
void mgetfilew(char []);
void mputfilew(char []);
void exit();
void clear();

// Our program starts from here 

int main(int argc, char *argv[])
{
	
	// client requires proper IP address and port of the server 

	if(argc!=3){
		printf("Usage: ./a.out server-address server-port");

	}
	struct sockaddr_in their_addr; /* connector's address information */

    // Building a socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket not found");
		exit(1);
	}

	// Socket structure's specifications for the client-socket
	memset((char *)&their_addr, 0, sizeof(their_addr));
	int PORT=atoi(argv[2]);
	their_addr.sin_family = AF_INET;      /* host byte order */
	their_addr.sin_port = htons(PORT);    /* short, network byte order */
	their_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

    /* inet_aton will find the running server's address and if server isn't found, 
      then it will print "Give proper IP address of the server" */

	/* inet_aton() returns non-zero if the address is a valid one, 
	and it returns zero if the address is invalid. */

	if (!inet_aton(argv[1], &their_addr.sin_addr)) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		printf("Give a proper server address\n");
		exit(1);
	}

     /* Once we've built a socket descriptor with the socket() call, we can connect() that socket to
        a remote server using the connect() system call */

	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("can not connect");
		exit(1);
	}

	else
	{
		client();
		close(sockfd);
	}
       
	return 0;
}

/* Printing every ftp command and function and which will be executed by our code
   and asking user to enter his/her choice */

void get_string( char *ad) //Gets a string i.e. get command of your choice
{ 
	printf("======================================================================================================================\n");
	printf("lls    : to lists content of client current directory \n");
	printf("lcd    : to change client current directory, usage: lcd dir_name \n");
	printf("lpwd   : to display path of client current directory \n");
	printf("ls     : to lists content of server current directory \n");
	printf("cd     : to change server current directory, usage: cd dir_name\n");
	printf("pwd    : to display path of server current directory \n");
	printf("gets   : to download file from server, file name specified, usage: gets file_name\n");
	printf("puts   : to upload file to server, file name specified, usage: puts file_name\n");
	printf("mgets  : to download multiple files from server, file name specified, usage: mgets file_name1,file_name2\n");
	printf("mputs  : to upload multiple files to server, file name specified, usage: mputs file_name1,file_name2\n");
	printf("exit   : to exit the connection \n");
	printf("clear  : to clear command window \n");
	printf("======================================================================================================================\n");

	printf("ftp>enter your choice : ");			
	char c;
	int j;
	for(j=0;;j++)
	{
	  c=getchar();
	  *(ad+j)=c;
	  if (c=='\n')
		break;
 	}
	*(ad+j)='\0';
};


void client()
{
	/* popen() function executes a command and pipes the executes command to 
	the calling program returning a pointer to the stream which can be used 
	by calling program to read and write to the pipe */

	FILE *output =popen("pwd", "r");
	if(!output)
	{
		printf(" permission denied ");
		return;
	}
	/* fscanf() reads data from the output and stores them in present_dir in the string format */ 
	fscanf(output, "%[^\n]", present_dir);
	pclose(output);	
	
	// 
	while(1)
	{
		// command and argument are used to differentiate user inputs
		char message[128]="";
		char command[128]="";
		char argument[128]="";

		
		get_string(message);   // get command string 

		// command1 stores the ftp command  
		char *command1=strtok(message, " ");

		// argument1 stores the file name and path 
		char *argument1=strtok(NULL, ";");
	
		if(command1==NULL)
		{
			continue;
		}
		strcpy(command, command1);

		/* If argument1 is not null then we copy the argument1 into 
		argument so that we can differentiate between ftp arguments */
		if(argument1!=NULL)
		{
			strcpy(argument, argument1);
		}

		printf("command = %s \n ", command);   // Printing the ftp command entered by user

		if(argument1!=NULL)
		{
			printf(" argument = %s \n ", argument);
		}

        // Things to be done at client side ==========================================

        /*strcmp compares the command string with the input string. 
        It returs 0 if the two strings are equal */

		if(!strcmp(command, "lls")) // get the content of client current directory
		{
			char x[128]="";
			strcat(x, "ls ");

			/*The following lines check whether a ftp command ( say ls) is valid or not 
			(for instance not followed by invalid options such as “ls –z” */

			if(argument!=NULL)
			{
				strcat(x, argument);
			}
			
			/* system(x) passes x specified by command to the host environment to be executed by the command processor
			   and return after the command has been completed */

			system(x);
		}	

		else if(!strcmp(command, "lcd")) // change client current directroy
		{
			if(strlen(argument)==0)
			{
				argument[0]='\0';
			}

			char pr[128] = "" ;   
			strcat(pr,present_dir);
 
			chdir(argument);

			/* getwd(present_dir) determines an absolute pathname of the current working directory 
			of the calling process and copy a string containing that pathname to the present_dir */

			getwd(present_dir);

            if (strcmp(argument,pr)==0)
            	printf(" client already present in this directory\n");
			else if(!strcmp(pr,present_dir))
			{
				printf("Invalid argument Usage: lcd [Enter a valid path here]\n") ;
			}
			else
		    {
			    printf(" client current directory changed to : %s\n ", present_dir);
			} 
		}

		else if(!strcmp(command, "lpwd")) // get client current directory path 
		{
			if(argument1==NULL)
		    {		
				getwd(present_dir);
				printf("client current directory present at : %s\n ", present_dir);
			} 

			else printf("Invalid argument Usage: lpwd \n");
		}
		
// end ================================================================================================

// start of ls command request to server ==============================================================
		else if(!strcmp(command, "ls")) // get the content of server current directory
		{
			// First we calculate the length of the command and then we send it along with command to the server
			int len =strlen(command);
			send(sockfd, &len, 4, 0);
			send(sockfd, command, len, 0);

			if(strlen(argument)==0)
			{
				// When argument length is zero i.e., valid argument 
				len=0;
				send(sockfd, &len, 4, 0);
				int count;
				recv(sockfd, &count, 4, 0);
			    // receive count of the total characters required for server current directory 

				int bytes_to_read;
				int i=0;
				int t=(int)count/(MAXDATASIZE-1);

				// 
				if(count>0)
				{
					for(i-0;i<=t;i++)
					{
						char result[1024];
						int bytes_recv = recv(sockfd, &len, 4, 0);
						bytes_recv = recv(sockfd, result, len, 0);
						write(1, result, bytes_recv);
						// result is the buffer, bytes_recv is the length
					}
				}
			}

			else
			{
				// When argument length is non - zero i.e., invalid argument
				char result[1024] ;
				len = strlen(argument);

				// send() returns the number of bytes actually sent out
				send(sockfd,&len,4,0);
				send(sockfd, argument, len, 0);

				//recv() returns the number of bytes actually read into the result
				int bytes_recv= recv(sockfd,result,80,0);
				write(1,result,bytes_recv);
				printf("\n");
			}
		}
// end ===========================================================================================================================================

// start of cd command : request to server ======================================================================================================
		else if(!strcmp(command, "cd")) // change server current directory
		{
			int len =strlen(command);
			send(sockfd, &len, 4, 0);
			send(sockfd, command, len, 0);
			
			if(strlen(argument)==0)
			{
				len = 0;
				send(sockfd, &len, 4, 0);
			}
			else
			{
				/* If length of argument is non-zero then we send the argument 
				to the server so that server can change its directory */
				len=strlen(argument);
				send(sockfd, &len, 4, 0);
				send(sockfd, argument, len, 0);
			}

			char buf[MAXDATASIZE];
			// Client receives the current directory of the server 
			int bytes_recv=recv(sockfd, &len, 4, 0);
			bytes_recv=recv(sockfd, buf, len, 0);
			write(1, buf, bytes_recv);
		}
	
// end ===========================================================================================================================================

// start of pwd command : server current directory path =========================================================================================
		
else if(!strcmp(command, "pwd")) //get server current directory path
		{
			char buf[1204];
			int bytes_recv;
			int len,length;
			len = strlen(command);
			send(sockfd, &len, 4, 0);
			send(sockfd, command, len, 0);
			char result[100];
			length=strlen(argument);
			send(sockfd,&length,4,0);

			if(strlen(argument)!=0)
			{
				// Invalid command as argument is not null
				bytes_recv= recv(sockfd,result,100,0);
				write(1, result, bytes_recv);
				//printf("\n");
			}
			
			/* We first receive the length of data and then data itself from the 
			server. Note that here data tells us the present server directory */
			else{
				bytes_recv = recv(sockfd, &len, 4, 0);
				bytes_recv = recv(sockfd, buf, len, 0);
				write(1, buf, bytes_recv);
			}
		}

// end ==========================================================================================================================================

// start of gets command : download files from server ============================================================================================
		else if(!strcmp(command, "gets"))
		{
			int len = strlen(command);
			// We send the command length and command itself to the server
			send(sockfd, &len, 4, 0);
			send(sockfd, command, len, 0);

			// ERROR - If the length of argument is 0 , then no file is entered 
			if(strlen(argument)==0)
			{
				len=0;
				send(sockfd, &len, 4, 0);
				printf("please enter the file along with command\n Usage: gets [filename]\n");
				continue;
			}
			// File name sent to the server
			else
			{
				len =strlen(argument);
				send(sockfd, &len, 4, 0);
				send(sockfd, argument, len, 0);
			}

			// receive file from server if it exists
			int err_no;
			char err_msg[128]="";
			recv(sockfd, &err_no, 4, 0);

			// If file doesn't exist in the server , then it receives err_no as 1 from the server
			if(err_no==1)
			{
				recv(sockfd, &len, 4, 0);
				recv(sockfd, err_msg, len, 0);
				printf("%s\n", err_msg);
				continue;
			}
			// If no error is found , it opens the file in write mode 
			FILE* fp = fopen(argument, "w");
			if(fp ==NULL)
			{
				printf(" file could not be opened for reading or writing..... check the permission of the frile which you want to access\n");
				err_no = 1;
				// If the file can't be opened , it sends message to the server that it can't write 
				send(sockfd, &err_no, 4, 0);
				continue;
			}
			else
			{
				err_no=0;
				send(sockfd, &err_no, 4, 0);
			}
			
			/* The rest of the portion of code of this loop is used 
			to receive file from the server where first ,the server 
			sends the total length of file and then the client 
			calculates the no of iterations required to receive the 
			file and then it receives data accordingly. */

			int size_of_file;
			recv(sockfd, &size_of_file, 4, 0);
			printf("size of file we have received is : %d\n ", size_of_file);
			if(size_of_file>0)
			{
				int i=0;
				int t = (int)size_of_file/(MAXDATASIZE);

				for(i=0;i<=t;i++)
				{
					char result[1024];
					int bytes_recv = recv(sockfd, &len, 4, 0);
					bytes_recv=recv(sockfd, result, len, 0);
					fwrite(result, bytes_recv, 1, fp);
					//write(1, result, bytes_recv);
				}
			}
			fclose(fp);
		}

		
// end ==========================================================================================================================================
	
// starts of puts command: upload file to server ================================================================================================

		else if(!strcmp(command, "puts"))
		{
			int err = 0;
			char err_msg[128]="";
			int len = strlen(command);
			send(sockfd, &len, 4, 0);
			send(sockfd, command, len, 0);
			// ERROR - If the length of argument is 0 , then no file is entered 
			if(strlen(argument)==0)
			{
				len=0;
				send(sockfd, &len, 4, 0);
				printf("please enter the file along with command\n Usage: gets [filename]\n");
				continue;
			}
			//send the filename to the server which the client wants to put
			else
			{
				len =strlen(argument);
				send(sockfd, &len, 4, 0);
				send(sockfd, argument, len, 0);
			}
			// file is opened in read mode to send to the server
			FILE *fp=fopen(argument, "r");
			// If file doesn't exist it can't be opened so send this information to the server
			if(fp==NULL)
			{
				err=1;
				send(sockfd, &err, 4, 0);
				printf("file does not exists in the client present directory\n");
				strcpy(err_msg, "file does not exists in the client present directory\n");
				len=strlen(err_msg);
				send(sockfd, &len, 4, 0);
				send(sockfd, err_msg, len, 0);
				continue;
			}
			// err=0 implies no error has occured
			else
			{
				err=0;
				send(sockfd, &err, 4, 0);
			}
			// if server cannot open a file to write the data received then error msg is received by the client
			recv(sockfd, &err, 4, 0);
			if(err==1)
			{
				fclose(fp);
				continue;
			}

			int size_of_file=0;
			char x;
			// Now the client calculates the length of the file and sends it to the server//
			while(fscanf(fp, "%c", &x)!=EOF)
			{
				size_of_file++;
			}
			rewind(fp);
			send(sockfd, &size_of_file, 4, 0);
			/* After rewinding to the begining of the file the client checks if the size is
			greater than 0 it starts sending max data that can be transferred to the  server*/ 
			if(size_of_file>0)
			{
				char result[MAXDATASIZE], data[MAXDATASIZE];
				int bytes_to_read;
				int i=0;
				int j=0;
				int t= (int)size_of_file/(MAXDATASIZE);
				for(j=0; j<=t; j++)
				{
					/* bytes_to_read is stored in temporary variable result at each iteration and
					then result is copied into data to transfer to the server*/
					bytes_to_read=size_of_file>(MAXDATASIZE)?(MAXDATASIZE):size_of_file;
					for(i=0; i<bytes_to_read; i++)
					{
						fscanf(fp, "%c", &result[i]);
					}
	
    					len = bytes_to_read;
					send(sockfd, &len, 4, 0);
					send(sockfd, result, len, 0);
					// size of the file is decreased by remaining bytes to read
					size_of_file = size_of_file - bytes_to_read;
				}
			}
			fclose(fp);  
		
			
		}

// end ==========================================================================================================================================

// start of mgets : download multiple files from server =========================================================================================

		else if(!strcmp(command, "mgets"))
		{
			
			char delims[]=",";
			char *result =NULL;
			result=strtok(argument, delims);
			/* strtok is used to separate each file by using delimiter "," 
			   It initially takes the first input file and does the 
			   gets FTP command which we have done previously. */
			 if(result==NULL){
			 	printf("please enter the file along with command\n Usage: mgets [filename1],[filename2] or mgets [filename1]\n");
			 }
			
			while( result != NULL ) 
			{
				strcpy(argument,result);
				

				int len = strlen(command);
				// We send the command length and command itself to the server
				send(sockfd, &len, 4, 0);
				send(sockfd, command, len, 0);

				if(strlen(argument)==0)
				{
					len=0;
					send(sockfd, &len, 4, 0);
					printf("please enter the file along with command\n Usage: gets [filename]\n");
					continue;
				}
				else
				{
					len =strlen(argument);
					send(sockfd, &len, 4, 0);
					send(sockfd, argument, len, 0);
				}
				// recieve file from server if it exists
				int err_no;
				char err_msg[128]="";
				recv(sockfd, &err_no, 4, 0);
				if(err_no==1)
				{
					recv(sockfd, &len, 4, 0);
					recv(sockfd, err_msg, len, 0);
					printf("%s\n",err_msg);
					//printf("dvdnvdn\n");
					break;
				}
				
				FILE* fp = fopen(argument, "w");
				if(fp ==NULL)
				{
					printf(" file could not be opened for reading or writing..... check the permission of the frile which you want to access\n");
					err_no = 1;
					// If the file can't be opened , it sends message to the server that it can't write
					send(sockfd, &err_no, 4, 0);
					continue;
				}
				else
				{
					err_no=0;
					send(sockfd, &err_no, 4, 0);
				}
	
				/* The rest of the portion of code of this loop is used 
				to receive file from the server where first ,the server 
				sends the total length of file and then the client 
				calculates the no of iterations required to receive the 
				file and then it receives data accordingly. */

				int size_of_file;
				recv(sockfd, &size_of_file, 4, 0);
				printf("size of file we have received is : %d\n ", size_of_file);
				if(size_of_file>0)
				{
					int i=0;
					int t = (int)size_of_file/(MAXDATASIZE);
					for(i=0;i<=t;i++)
					{
						char result[1024];
						int bytes_recv = recv(sockfd, &len, 4, 0);
						bytes_recv=recv(sockfd, result, len, 0);
						fwrite(result, bytes_recv, 1, fp);
						//write(1, result, bytes_recv);
					}
				}
				fclose(fp);
				//if(m==1){result=NULL;}
		 		result = strtok( NULL, delims );
		 	}
		}

// end ==========================================================================================================================================

//	start of mputs: upload multiple files to server ==========================================================================================

		else if(!strcmp(command, "mputs"))
		{
			char delims[]=",";
			char *result =NULL;
			result=strtok(argument, delims);
			/* strtok is used to separate each file by using delimiter "," 
			   It initially takes the first input file and does the 
			   puts FTP command which we have done previously. */

			if(result==NULL){
			 	printf("please enter the file along with command\n Usage: mputs [filename1],[filename2] or mputs [filename1]\n");
			 }

			while( result != NULL ) 
			{
				strcpy(argument,result);
				int err = 0;
				char err_msg[128];
				int len = strlen(command);
				send(sockfd, &len, 4, 0);
				send(sockfd, command, len, 0);
				if(strlen(argument)==0)
				{
					len=0;
					send(sockfd, &len, 4, 0);
					printf("please enter the file along with command\n Usage: gets [filename]\n");
					continue;
				}
				else
				{
					len =strlen(argument);
					send(sockfd, &len, 4, 0);
					send(sockfd, argument, len, 0);
				}
	
				FILE *fp=fopen(argument, "r");
	
				if(fp==NULL)
				{
					err=1;
					send(sockfd, &err, 4, 0);
					strcpy(err_msg, "All file does not exists in the client present directory\n");
					len=strlen(err_msg);
					send(sockfd, &len, 4, 0);
					send(sockfd, err_msg, len, 0);
					break;
				}
				else
				{
					err=0;
					send(sockfd, &err, 4, 0);
				}
				recv(sockfd, &err, 4, 0);
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
				send(sockfd, &size_of_file, 4, 0);
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
						send(sockfd, &len, 4, 0);
						send(sockfd, result, len, 0);
		
						size_of_file = size_of_file - bytes_to_read;
					}
				}
				fclose(fp);  
			
				result = strtok( NULL, delims );
		 	}
		}
// end ==========================================================================================================================================


// To exit the connection =======================================================================================	
		else if(!strcmp(command, "exit")) 
		{
			int bytes_recv;
			int len;
			len=strlen(command);
			send(sockfd, &len, 4, 0);
			send(sockfd,command,len,0);
			break;
		}
// end =========================================================================================================
		
 // clear the command window ============================================
		else if(!strcmp(command, "clear"))
		{
			system("clear");
		}
//end ===================================================================
		else
		{
			printf(" command is not found please try again \n");
		}
	
	}

	printf(" Thank you the procedure is done \n ");
}

int get_message_from_server(char *variable, int fd, int size) // return the bytes which were received and if there is error return -1
{
	int bytes_recvd;
	bytes_recvd=recv(fd, variable, size, 0);
	return bytes_recvd;
}












































 


















