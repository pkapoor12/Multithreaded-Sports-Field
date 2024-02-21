/*
 * stadium.c
 *
 *  Created on: Jan 29, 2024
 *      Author: Pujeet Kapoor
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BASEBALL 1
#define FOOTBALL 2
#define RUGBY 3
#define NUM_BASEBALL 36
#define NUM_FOOTBALL 44
#define NUM_RUGBY 60
#define GAMES 2
#define TOTAL_GAMES GAMES * 3 * 2

int baseball_team[NUM_BASEBALL/2]; //baseball players ready to play
int football_team[NUM_FOOTBALL/2]; //football players ready to play
int rugby_team[NUM_RUGBY/2]; //rugby players ready to play
int rugby_pair_delay[NUM_RUGBY/4]; //the delay each pair of rugby players uses while playing
int baseball_team_num = 0; //number of players in array
int football_team_num = 0; //number of players in array
int rugby_team_num = 0; //number of players in array
int baseball_team_full = 0;
int football_team_full = 0;
int rugby_team_full = 0;
int baseball_on_field = 0;
int football_on_field = 0;
int rugby_on_field = 0;
int last_played;
int baseball_length;
int football_length;

int field_occupied = 0;
int total_games = 0;

pthread_mutex_t baseball_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t football_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rugby_m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t field_m = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t field_c = PTHREAD_COND_INITIALIZER;
pthread_cond_t baseball_empty_c = PTHREAD_COND_INITIALIZER;
pthread_cond_t football_empty_c = PTHREAD_COND_INITIALIZER;
pthread_cond_t rugby_empty_c = PTHREAD_COND_INITIALIZER;
pthread_cond_t baseball_c = PTHREAD_COND_INITIALIZER;
pthread_cond_t football_c = PTHREAD_COND_INITIALIZER;
pthread_cond_t rugby_c = PTHREAD_COND_INITIALIZER;

int read_seed(FILE*);
void* baseball(void*);
void* football(void*);
void* rugby(void*);
int get_team(int, int);
void try_to_enter(int, int);
void play(int, int, int);
void try_to_leave(int, int, int);

int main() {

	//read seed from seed.txt file for srand()
	FILE* seed_file = fopen("seed.txt", "r");
	int seed = read_seed(seed_file);
	fclose(seed_file);
	srand(seed);

	//set the length of baseball and football matches
	baseball_length = rand() % (10 - 6 + 1) + 6;
	football_length = (12 - 8 + 1) + 8;

	//arrays to store tid (for joining at the end) and number of each array to pass as an argument
	pthread_t tid[NUM_BASEBALL + NUM_FOOTBALL + NUM_RUGBY];
	int num[NUM_BASEBALL + NUM_FOOTBALL + NUM_RUGBY];

	//create all baseball threads
	for(int i = 0; i < NUM_BASEBALL; i++) {
		num[i] = i + 1;
		pthread_create(&tid[i], NULL, baseball, &num[i]);
	}

	//create all football threads
	for(int i = NUM_BASEBALL; i < NUM_BASEBALL + NUM_FOOTBALL; i++) {
		num[i] = i + 1;
		pthread_create(&tid[i], NULL, football, &num[i]);
	}

	//create all rugby threads
	for(int i = NUM_BASEBALL + NUM_FOOTBALL; i < NUM_BASEBALL + NUM_FOOTBALL + NUM_RUGBY; i++) {
		num[i] = i + 1;
		pthread_create(&tid[i], NULL, rugby, &num[i]);
	}

	//wait for all threads to finish
	for(int i = 0; i < NUM_BASEBALL + NUM_FOOTBALL + NUM_RUGBY; i++) {
		pthread_join(tid[i], NULL);
	}


	exit(0);

}

int read_seed(FILE* seed_file) {

	char seed_string[10];
	int seed;

	fgets(seed_string, 10, seed_file);
	seed = atoi(seed_string);
	return seed;

}

//baseball threads
void* baseball(void* arg) {

	//delay for entering field and after leaving field
	int delay = rand() % (7 - 3 + 1) + 3;
	sleep(delay);

	int* num = arg;

	//only join team and play for specified number of games (2)
	for(int i = 0; i < GAMES; i++){

		//try to join team
		int joined = get_team(BASEBALL, *num);
		//if last to join team
		if(joined == NUM_BASEBALL/2) {
			try_to_enter(BASEBALL, *num);
		}

		//play
		pthread_mutex_lock(&field_m);

		while(baseball_on_field == 0) {
			pthread_cond_wait(&baseball_c, &field_m);
		}

		play(BASEBALL, *num, joined);

		pthread_mutex_unlock(&field_m);
		sleep(baseball_length);

		//leave field
		try_to_leave(BASEBALL, *num, joined);
		sleep(delay);


	}

	pthread_exit(0);

}

//football threads
void* football(void* arg) {

	//delay for entering field and leaving field
	int delay = rand() % (6 - 2 + 1) + 2;
	sleep(delay);

	int* num = arg;

	//only join team and play for specified number of games (2)
	for(int i = 0; i < GAMES; i++){

		//try to join team
		int joined = get_team(FOOTBALL, *num);
		//if last to join team
		if(joined == NUM_FOOTBALL/2) {
			try_to_enter(FOOTBALL, *num);
		}

		//play
		pthread_mutex_lock(&field_m);

		while(football_on_field == 0) {
			pthread_cond_wait(&football_c, &field_m);
		}

		play(FOOTBALL, *num, joined);

		pthread_mutex_unlock(&field_m);
		sleep(football_length);

		//leave field
		try_to_leave(FOOTBALL, *num, joined);
		sleep(delay);


	}

	pthread_exit(0);

}

//rugby threads
void* rugby(void* arg) {

	//delay for entering and after leaving
	int delay = rand() % (5 - 2 + 1) + 2;
	sleep(delay);

	int* num = arg;
	int pair;

	//only join team and play for specified number of games
	for(int i = 0; i < GAMES; i++){

		//try to join team
		int joined = get_team(RUGBY, *num);

		//set pair number
		if(joined % 2 == 1) {
			pair = (joined + 1) / 2;
		}
		else {
			pair = joined / 2;
		}

		//if last to join
		if(joined == NUM_RUGBY/2) {
			try_to_enter(RUGBY, *num);
		}

		//play
		pthread_mutex_lock(&field_m);

		while(rugby_on_field == 0) {
			pthread_cond_wait(&rugby_c, &field_m);
		}

		play(RUGBY, *num, joined);

		pthread_mutex_unlock(&field_m);
		sleep(rugby_pair_delay[pair - 1]);

		//leave field
		try_to_leave(RUGBY, *num, joined);
		sleep(delay);

	}

	pthread_exit(0);
}


//function to join team, returns what position (order) thread joins at
int get_team(int sport, int num) {

	int joined = 0;

	if(sport == BASEBALL) {

		pthread_mutex_lock(&baseball_m);

		while(joined == 0) {
			//if not full
			if(baseball_team_num < NUM_BASEBALL/2 && baseball_team_full == 0) {
				baseball_team[baseball_team_num] = num;
				baseball_team_num++;
				joined = baseball_team_num;
			}

			//if last to join
			if(baseball_team_num == NUM_BASEBALL/2 && baseball_team_full == 0) {
				printf("\t\t[Baseball: %d]: Team Ready\n", num);
				baseball_team_full = 1;
			}
			//wait (sleep) if full until emptied
			else if(baseball_team_full == 1) {
				while(baseball_team_full == 1) {
					pthread_cond_wait(&baseball_empty_c, &baseball_m);
				}

			}
		}

		pthread_mutex_unlock(&baseball_m);

	}

	else if(sport == FOOTBALL) {

		pthread_mutex_lock(&football_m);

		while(joined == 0) {
			//if not full
			if(football_team_num < NUM_FOOTBALL/2 && football_team_full == 0) {
				football_team[football_team_num] = num;
				football_team_num++;
				joined = football_team_num;
			}

			//if last to join
			if(football_team_num == NUM_FOOTBALL/2 && football_team_full == 0) {
				printf("[Football: %d]: Team Ready\n", num);
				football_team_full = 1;
			}
			//wait (sleep) if full until emptied
			else if(football_team_full == 1) {
				while(football_team_full == 1) {
					pthread_cond_wait(&football_empty_c, &football_m);
				}

			}
		}

		pthread_mutex_unlock(&football_m);

	}

	else if(sport == RUGBY) {

		pthread_mutex_lock(&rugby_m);

		while(joined == 0) {
			//if not full
			if(rugby_team_num < NUM_RUGBY/2 && rugby_team_full == 0) {
				rugby_team[rugby_team_num] = num;
				rugby_team_num++;
				joined = rugby_team_num;
				if(joined % 2 == 0) {
					printf("\t\t\t[Rugby: %d]: Pair Ready\n", num);
					rugby_pair_delay[(joined/2) - 1] = rand() % (7 - 4 + 1) + 4;
				}
			}
			//if last to join
			if(rugby_team_num == NUM_RUGBY/2 && rugby_team_full == 0) {
				rugby_team_full = 1;
			}
			//wait (sleep) if full until emptied
			else if(rugby_team_full == 1) {
				while(rugby_team_full == 1) {
					pthread_cond_wait(&rugby_empty_c, &rugby_m);
				}
			}
		}

		pthread_mutex_unlock(&rugby_m);
	}

	return joined;

}

//last one to join team attempts to enter field with this function
void try_to_enter(int sport, int num) {


	if(sport == BASEBALL) {

		pthread_mutex_lock(&field_m);

		//makes sure only one sport can play at time and a sport cannot go twice in a row
		while((field_occupied != BASEBALL && field_occupied > 0) || last_played == BASEBALL) {
			if(total_games == TOTAL_GAMES - 1) {
				break;
			}
			pthread_cond_wait(&field_c, &field_m);
		}

		field_occupied = BASEBALL;
		printf("\t\t[Baseball: %d] Game STARTED\n", num);
		baseball_on_field++;
		pthread_cond_broadcast(&baseball_c);

		pthread_mutex_unlock(&field_m);

	}
	else if(sport == FOOTBALL) {

		pthread_mutex_lock(&field_m);

		//makes sure only one sport can play at a time and a sport cannot go twice in a row
		while((field_occupied != FOOTBALL && field_occupied > 0) || last_played == FOOTBALL) {
			if(total_games == TOTAL_GAMES - 1) {
				break;
			}
			pthread_cond_wait(&field_c, &field_m);
		}

		field_occupied = FOOTBALL;
		printf("[Football: %d] Game STARTED\n", num);
		football_on_field++;
		pthread_cond_broadcast(&football_c);

		pthread_mutex_unlock(&field_m);

	}

	else if(sport == RUGBY) {

		pthread_mutex_lock(&field_m);

		//makes sure only one sport can play at a time and a sport cannot go twice in a row
		while((field_occupied != RUGBY && field_occupied > 0) || last_played == RUGBY) {
			if(total_games == TOTAL_GAMES - 1) {
				break;
			}
			pthread_cond_wait(&field_c, &field_m);
		}

		field_occupied = RUGBY;
		printf("\t\t\t[Rugby: %d] Game STARTED\n", num);
		rugby_on_field++;
		pthread_cond_broadcast(&rugby_c);

		pthread_mutex_unlock(&field_m);
	}

	return;

}

//print messages that playing has begun
void play(int sport, int num, int position) {

	if(sport == BASEBALL) {
		printf("\t\t[Baseball: %d] Playing at position %d\n", num, position);
	}
	else if(sport == FOOTBALL) {
		printf("[Football: %d] Playing at position %d\n", num, position);
	}
	else if(sport == RUGBY) {
		int pair;
		if(position % 2 == 1) {
			pair = (position + 1) / 2;
		}
		else {
			pair = position/2;
		}
		printf("\t\t\t[Rugby: %d] Playing at position %d (Pair %d)\n", num, position, pair);
	}

	return;
}

//playing threads leave field through this function, last to leave signals an empty team and empty field
void try_to_leave(int sport, int num, int position) {

	if(sport == BASEBALL) {

		pthread_mutex_lock(&baseball_m);

		baseball_team[position - 1] = 0;
		baseball_team_num--;

		if(baseball_team_num == 0) {
			printf("\t\t[Baseball: %d] Game ENDED\n", num);
			baseball_team_full = 0;
			baseball_on_field = 0;
			field_occupied = 0;
			total_games++;
			last_played = BASEBALL;
			pthread_cond_broadcast(&baseball_empty_c);
			pthread_cond_signal(&field_c);
		}

		pthread_mutex_unlock(&baseball_m);

	}
	else if(sport == FOOTBALL) {

		pthread_mutex_lock(&football_m);

		football_team[position - 1] = 0;
		football_team_num--;

		if(football_team_num == 0) {
			printf("[Football: %d] Game ENDED\n", num);
			football_team_full = 0;
			football_on_field = 0;
			field_occupied = 0;
			total_games++;
			last_played = FOOTBALL;
			pthread_cond_broadcast(&football_empty_c);
			pthread_cond_signal(&field_c);
		}

		pthread_mutex_unlock(&football_m);

	}
	else if(sport == RUGBY) {

		pthread_mutex_lock(&rugby_m);

		rugby_team[position - 1] = 0;
		rugby_team_num--;

		if(rugby_team_num == 0) {
			printf("\t\t\t[Rugby: %d] Game ENDED\n", num);
			rugby_team_full = 0;
			rugby_on_field = 0;
			field_occupied = 0;
			total_games++;
			last_played = RUGBY;
			pthread_cond_broadcast(&rugby_empty_c);
			pthread_cond_signal(&field_c);
		}

		pthread_mutex_unlock(&rugby_m);

	}


	return;

}
