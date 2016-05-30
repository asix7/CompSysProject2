/* Andres Landeta alandeta 631427 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "game.h"

#include <unistd.h>

/* This file contains the game logic, therefore the functions needed by the server to run it*/

/* Generates a random code for the game */
char*
generate_code(char* code){

	int i = 0;
	
	while(i < NUM_CHOOSEN){
		/*Select a random int between 0 and NUM_LETTERS - 1*/
		srand(time(NULL) + i * (int)rand());
		int r = (int)((rand() / ((RAND_MAX) * 1.0f)) * NUM_LETTERS);
		/*In case rand() == RAND_MAX*/
		if(r == NUM_LETTERS){
			r = NUM_LETTERS - 1;
		}
		code[i] = ALPHABET[r];
		i++;
	}
	return code;
}

/* Get and check the next guess against the code, and inform the player with the correct messages*/
guess_out_t*
next_guess(char* code, char* input, int num_guesses){

	char compare[NUM_CHOOSEN];

	char message[MSG_LEN];
	int i = 0;
	int b = 0;		
	int valid, len;

	guess_out_t *output = malloc(sizeof(guess_out_t));

	/* Copy the pattern to not overwrite it, and work with a copy*/
	strcpy(compare, code);

	/* Check the guess */
	while(i < NUM_CHOOSEN){		
		/* Interrupt if any given letter is not valid */	
		valid = is_valid(input[i]);
		if(!valid){
			break;
		}
		/* Find matching letters and discard them for next letters checks*/
		if(input[i] == compare[i]){
			input[i] = LETTER_PLACEHOLDER;
			compare[i] = LETTER_PLACEHOLDER;
			b++;			
		} 
		i++;
	}

	/* Handle invalid guess */
	if(!valid) {
		num_guesses++;

		/* Format the message to send */
		if(num_guesses >= MAX_GUESSES){
			snprintf(message, sizeof(message), "FAILURE\nThe code was %s\n", code);
			output->log_msg = "FAILURE";
			num_guesses = MAX_GUESSES;	

		} else {
			snprintf(message, sizeof(message), "INVALID\n%d more opportunities. Try again:\n", 
			  		MAX_GUESSES - num_guesses);	
			output->log_msg = "INVALID";			
		}
		message[MSG_LEN - 1] = '\0';
		len = strlen(message) + 1;	

		/* Update output */
		output->send_msg = message;
		output->send_len = len;		
	
	/* Handle successful guess */
	} else if (b == NUM_CHOOSEN) {
		/* Format the message to send */
		snprintf(message, sizeof(message), "SUCCESS\n");
		message[MSG_LEN - 1] = '\0';
		len = strlen(message) + 1;

		/* Update output */
	    output->log_msg = "SUCCESS";
	    num_guesses = MAX_GUESSES;

		output->send_msg = message;
		output->send_len = len;


	/* Report feedback */    
	} else {
		int m = incorrect_position(input, compare);
		num_guesses++;
		output = feedback(num_guesses, b, m, code);
	}	
	
	/* Update the final num_guesses */
	output->num_guesses = num_guesses;	 
	return output ;
}

/* Check if a letter is valid for current alphabet */
int 
is_valid(char letter){
	int valid = 1;
	int n = 0;
	while(valid){
		if(letter == ALPHABET[n])
			break;
		n++;
		if(n == NUM_LETTERS){
			valid = 0;					
		}				
	}
	return valid;
}

/* Find the number of letters in the guess that are in the code but in incorrect position */
int 
incorrect_position(char* input, char* compare){
	int m = 0;
	int i, j;
	for(i = 0; i < NUM_CHOOSEN; i++){
		for(j = 0; j < NUM_CHOOSEN; j++) {
			/* Skip if any letter has been discarted */
			if(input[i] != LETTER_PLACEHOLDER && compare[j] != LETTER_PLACEHOLDER){
				/* Find maching letters and discard them for next letters checks*/
				if(input[i] == compare[j]){
					compare[j] = LETTER_PLACEHOLDER;	
					input[i] = LETTER_PLACEHOLDER;	
					m++;
				}
			}
		}
	}
	return m;
}

/* 
** Provides feedback, either [b,m] or FAILURE in case the number of guesses have been exhausted
** return the new count for the number of guesses
 */
guess_out_t*
feedback(int num_guesses, int b, int m, char* code){
	
	char message[MSG_LEN];
	int len;	
	guess_out_t *output = malloc(sizeof(guess_out_t));

	if(num_guesses >= MAX_GUESSES){
		snprintf(message, sizeof(message), "FAILURE\nThe code was %s\n", code);
		output->log_msg = "FAILURE";
		output->num_guesses = MAX_GUESSES;	

	} else {		
		snprintf(message, sizeof(message), "[%d, %d]\n%d more opportunities. Try again:\n", b, m, MAX_GUESSES - num_guesses);
		/* Record the feedback */
		char *log_msg;
		log_msg = malloc(sizeof(char) * MSG_LEN);
		sprintf(log_msg,"[%d, %d]", b, m);
		output->log_msg = log_msg;
		output->num_guesses = num_guesses;
	}

	message[MSG_LEN - 1]='\0';
	len = strlen(message) + 1;
	output->send_msg = message;
	output->send_len = len;
	
	return output;
}

/* Get the instructions for the game*/
char* game_instructions(){
	return "\nWelcome to Matermind!\n\nIn this game your objective will be to guess a secret code.\n\n\
The code is a simple 4 letter pattern formed by any combination of the letters:\n\
\tA, B, C, D, E and F\n\nIn order for you to win you need to provide your 4 letter guess as input\n\
\texample: ABCD\n\nIf you don't guess the pattern you will receive feedback in the form [b:m]:\n\
\t-b is the number of letters in the correct position\n\
\t-m is the number of correct letters but in the incorrect position\n\n\
You have 10 opportunities to guess, have fun!!!\nPlease provide your guess:\n";
}




