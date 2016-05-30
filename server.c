/* Andres Landeta alandeta 631427 */

/* 
** Part of this program is based in the sample code TCP2/server.c given for the subject Computer Systems(COMP30017)
** of the University of Melbourne Semester 1 2016. All the rights belong to their respective creators 
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "server.h"
#include "game.h"

/* Used to compile in Ubuntu(Nectar) */
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#define FILE_NAME "log.txt"
#define MAX_CONNECTIONS 100000 /* Max number of connections during server running time */
#define TIME_LEN 25 /* Time format max len */
#define SEC_TO_MIC 1000000000

extern int errno;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static FILE *logfile;
static volatile int correct_guesses = 0;
static volatile int number_of_connections = 0;

/* 
** Server of codemaker, takes the game information and communicates with 
** the clients (TCP) so they can play the game.
*/

int main (int argc, char *argv[])
{
	struct sockaddr_in server, client;
	int s, server_port;
	char *code;
	
	logfile = fopen(FILE_NAME, "w+");
	signal(SIGINT, end_server);
	signal(SIGTERM, end_server);

	if(argc >= 2){
		server_port = atoi(argv[1]);		
	}
	else {
		fprintf(stderr, "Usage :server portnumber\n");
		exit(1);
	}
	/* Get the code if its provided*/
	if(argc==3){		
		int codelen = strlen(argv[2]);	

		if(codelen < NUM_CHOOSEN){
			fprintf(stderr, "No enough characters for code\n");
			exit(1);
		} else {
			code = malloc(sizeof(char)*NUM_CHOOSEN);
			strncpy(code, argv[2], NUM_CHOOSEN);
		}
	} else {
		code = NULL;
	}

	printf("Server port %i\n",server_port);
	/* Building data structures for sockets */
	/* Identify two end points; one for the server and the other for the client when it connects to the server socket */
	memset (&server,0, sizeof (server));
	
	/* Server socket initializations */
	/* AF_INET: specifies the connection to Internet. In our example we use 
	TCP port 5431 as the known server port; Any client need to connect to this port;
	INADDR_ANY specifies the server is willing to accept connections on any of the local host's IP addresses. */ 

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons (server_port); 

	/* Preliminary server steps: Setup: creation of passive open socket*/

	if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
      perror ("Error creating socket");
      exit (1);
	}
	printf("Socket descriptor:  %d\n", s);

	/* Bind the socket to local address */

	if (bind (s, (struct sockaddr *) &server, sizeof (server)) < 0)
	{
      perror ("Error in binding to the specified port");
      exit (1);
	}
	printf("sock:  family = %d\n", server.sin_family);
	printf("       saddr  = %d\n", server.sin_addr.s_addr);
	printf("       port   = %d\n", ntohs(server.sin_port));

	/* Sets the maximum number of pending connections to be allowed, in our case this number is 10 */

	if ( listen (s, 5) < 0)
	{
        perror("listen() failed with error");        
        exit(1);
	}
	else
	{
		printf("Listening on port %d...\n", ntohs(server.sin_port));
	}

	process_connections(s, code);

	return 1;
}

/* Accepts and process new connections up to MAX_CONNECTIONS*/
void
process_connections(int s, char* code)
{	
	int new_s, len; 
	struct sockaddr_in client;
	memset (&client,0, sizeof (client));

	/* Create threads*/
	int thread;
	pthread_t* thread_id = malloc(MAX_CONNECTIONS*sizeof(pthread_t));

	/* Initialize thread data */
	data_t *new_conection = malloc(sizeof(data_t));

	/* The main loop of the program*/
	/* Wait for connection then receive and print text */
	while (1)
	{
		len = sizeof(client);
		while(number_of_connections < MAX_CONNECTIONS){
			if ((new_s = accept (s, (struct sockaddr *) &client, &len)) < 0)
			{
				printf("errno = %d, new_s = %d\n", errno, new_s);
				perror ("Accept failed");
				exit (1);
			}
			/* Create the connection with a new client */
			else
			{				
				number_of_connections++;
				/* Translate client ip4*/
				char ip4[INET_ADDRSTRLEN];
				inet_ntop(AF_INET,&(client.sin_addr), ip4, INET_ADDRSTRLEN);
				
				/* Fill structure passed to thread */
				new_conection->client_socket = new_s;
				new_conection->ip4 = ip4;
				
				if(code != NULL){					
					new_conection->code = malloc(sizeof(char)*sizeof(NUM_CHOOSEN));
					strcpy(new_conection->code, code);		
				} 

				/*Create new thread with the new client*/
				thread = pthread_create(thread_id+number_of_connections, NULL, new_game, (void*) new_conection);

				
			}
		}
	}
}


/* New game handler, send and recieve messages with a client */
void 
*new_game(void *new_conection)
{
    int len;
    char guess[NUM_CHOOSEN + 1];     
    guess_out_t *output; 
    char *code = malloc(sizeof(char)*sizeof(NUM_CHOOSEN));;

    char *time_str = malloc(sizeof(char) * TIME_LEN);

	/* Get data from strcuture */
	data_t *client_data = (data_t*)new_conection;
	int new_s = client_data->client_socket;
	char *ip4 = client_data->ip4;

	pthread_mutex_lock(&mutex);
	fprintf(logfile, "%s (%s) (sock_id = %d) client connected\n", time_format(time_str), ip4, new_s);
    pthread_mutex_unlock(&mutex);	

	/* Generate a code for each client if it was not provided */
	if(client_data->code != NULL){		
		strcpy(code, client_data->code);	
	} else {					
		code = generate_code(code);		
	}	

	pthread_mutex_lock(&mutex);
	fprintf(logfile, "%s (0.0.0.0) Server secret for (%s)(sock_id = %d) = %s\n", time_format(time_str), ip4, new_s, code);
	pthread_mutex_unlock(&mutex);

	/*Get and send instructions for the game */
	char* instructions = game_instructions();
	write(new_s, instructions, strlen(instructions) + 1);

	/*Read guesses from the user until the number of guesses has been exhausted */
	int num_guesses = 0;
	while (len=recv(new_s,&guess,sizeof(guess),0)){
		if(num_guesses < MAX_GUESSES){

			/* Write guess into log file */
			fprintf(logfile, "%s From: (%s)(socket_id = %d) guess = %s\n", time_format(time_str), ip4, new_s, guess);

			/*Get the output of the guess*/
			output = next_guess(code, guess, num_guesses);
			
			/*Send Message to player*/
			write(new_s, output->send_msg, output->send_len);	
				
			/* Write outcome into log file */	
			pthread_mutex_lock(&mutex);
			if(strstr(output->log_msg, "SUCCESS")){
				correct_guesses++;
				fprintf(logfile, "%s To: (%s)(socket_id = %d) %s\n", time_format(time_str), ip4, new_s, 
					                                   output->log_msg);
			} else if(strstr(output->log_msg, "FAILURE")){
				fprintf(logfile, "%s To: (%s)(socket_id = %d) %s\n", time_format(time_str), ip4, new_s, 
					                                   output->log_msg);
			} else if(strstr(output->log_msg, "INVALID")){
				fprintf(logfile, "%s To: (%s)(socket_id = %d) %s\n", time_format(time_str), ip4, new_s, 
					                                   output->log_msg);
			} else {
				fprintf(logfile, "%s To: (%s)(socket_id = %d) server's hint: %s\n", time_format(time_str), ip4, new_s, 
					                                                  output->log_msg);
			} 
			pthread_mutex_unlock(&mutex);

			num_guesses = output->num_guesses;					
		} else {
			break;
		}
	}
	close (new_s);
} 

/* Creates the correct time format to used in the log file */
char* 
time_format(char* time_str){
	time_t server_time;
	struct tm *local_time;	

	server_time = time(NULL);
	local_time = localtime( &server_time);
	strftime(time_str,TIME_LEN,"[%d %m %Y %H:%M:%S]", local_time);

	return time_str;
}

/* Handles SIGINT and SIGTERM by finalizing the server */
void
end_server(){
	printf("\nFinalize Server\n");

	fprintf(logfile,"Total num of connections:       %d\n", number_of_connections);
	fprintf(logfile,"Total num of codes guessed:     %d\n", correct_guesses);

	/* Obtain and write Resource Usage of the Program */
	struct rusage resource_usage;
	getrusage(RUSAGE_SELF, &resource_usage);
	fprintf(logfile,"Max Resident Set Size:          %ld\n", resource_usage.ru_maxrss);
	fprintf(logfile,"CPU Time (microseconds):        user = %ld system = %ld\n",
	            resource_usage.ru_utime.tv_sec*SEC_TO_MIC + resource_usage.ru_utime.tv_usec,
	            resource_usage.ru_stime.tv_sec*SEC_TO_MIC + resource_usage.ru_stime.tv_usec);
	fprintf(logfile,"Involuntary Context Switches:   %ld\n", resource_usage.ru_nivcsw);
	fprintf(logfile,"Volutary Context Switches:      %ld\n", resource_usage.ru_nvcsw);
	fprintf(logfile,"Total Context Cwitches:         %ld\n", resource_usage.ru_nvcsw + 
		                                                    resource_usage.ru_nivcsw);
	if(number_of_connections != 0)
		fprintf(logfile,"Cont Switches per connection:   %f\n", 1.0*(resource_usage.ru_nvcsw + 
		                                                    resource_usage.ru_nivcsw)/number_of_connections);
	fclose(logfile);
	exit(0);
}

