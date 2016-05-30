/* Andres Landeta alandeta 631427 */

/* 
** Part of this program is based in the sample code TCP2/client.c given for the subject Computer Systems(COMP30017)
** of the University of Melbourne Semester 1 2016. All the rights belong to their respective creators 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "client.h"

#include <unistd.h>

# define MSG_LEN 50  /* Lenght of the messages recieved from server */
# define INS_LEN 700 /* length of the game instructions recieved from server */
# define GUESS_LEN 4 /* lenght of the pattern to send the server */

/* Client used to play the game with the server */

int main(int argc, char * argv[])
{
	char instructions[INS_LEN];

	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	int s, server_port;

	if(argc==3){
		host = argv[1];
		server_port = atoi(argv[2]);
	}
	else {
		fprintf(stderr, "Usage :client host server_port\n");
		exit(1);
	}

	/* translate host name into peer's IP address ; This is name translation service by the operating system */
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "Unknown host %s  \n",host);
		exit(1);
	}
	/* Building data structures for socket */

	bzero( (char *)&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port =htons(server_port);

	/* Active open */
	/* Preliminary steps: Setup: creation of active open socket*/

	if ( (s = socket(AF_INET, SOCK_STREAM, 0 )) < 0 )
	{
		perror("Error in creating a socket to the server");
		exit(1);
	}

	if(connect(s, (struct sockaddr *)&sin, sizeof(sin))  < 0  )
	{
		perror("Error in connecting to the host");
		close(s);
		exit(1);
	}

	/* Read the game instructions */
	int n = read(s,instructions,INS_LEN);
	
	if (n < 0)
	{
		perror("ERROR reading from socket");
		exit(0);
	}
	printf("%s\n",instructions);

	play_game(s);

	close(s);
	return 1;
}

/* Play the game by sending guesses and receiving messages from the server */
void play_game(int s){

	int len;
	char guess[GUESS_LEN];
	char server_msg[MSG_LEN];

	while(1){
		/* Get user input */
		if(scanf("%s", guess)){
			guess[GUESS_LEN]='\0';
			len=strlen(guess)+1;
			write(s,guess,len);
		}
		/* Read response from server and print it*/
		if(read(s,&server_msg,sizeof(server_msg))){
			printf("%s",server_msg);
			/*Breaks the loop when the game has finished*/
			if(strstr(server_msg, "SUCCES") != NULL){
				break;
			} else if (strstr(server_msg, "FAILURE") != NULL){
				break;
			}			
		} else {
			fprintf(stderr, "Error connecting to the game server\n");
			break;
		}

	}
}