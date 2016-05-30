/* Andres Landeta alandeta 631427 */
/* Header File for game*/

#define NUM_CHOOSEN 4 /* Size of the pattern used for the code */
#define MAX_GUESSES 10 /* Maximun number of guesses per game */
#define NUM_LETTERS 6 /* Number of letters in the alphabet */
#define ALPHABET "ABCDEF" /* Letters used in the code */
#define LETTER_PLACEHOLDER '@' /* Permit the discard of used letters */
#define MSG_LEN 50 

/* Guess output format */
typedef struct
{
	int num_guesses;
	char* log_msg;
	char* send_msg;
	int send_len;

}guess_out_t;

char* generate_code(char* choosen);
guess_out_t* next_guess(char* choosen, char* input, int num_guesses);
int is_valid(char letter);
int incorrect_position(char* input, char* compare);
guess_out_t* feedback(int num_guesses, int b, int m, char* choosen);
char* game_instructions();
