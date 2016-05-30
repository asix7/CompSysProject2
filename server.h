/* Andres Landeta alandeta 631427 */
/* Header file for Server*/

/* Data of the new connection */
typedef struct
{
	int client_socket;
	char* code;
	char* ip4;
}data_t;


void process_connections(int s, char* code);
void *new_game(void *new_client_sock);
char* time_format(char* time_str);
void end_server();

