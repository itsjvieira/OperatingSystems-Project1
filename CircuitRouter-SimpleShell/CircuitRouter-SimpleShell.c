/* Joao Vieira, no. 90739
 * Joao Parreiro, no. 89483
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../lib/commandlinereader.h"
#include "../lib/list.h"
#include "../lib/types.h"

#define VECTOR_SIZE 3
#define BUFFER_SIZE 256


/* verify status, print pid values and
 * free status and pid values
 */
void print_free_values(list_t* list_pid, list_t* list_status) {

	if (!(list_isEmpty(list_pid) && list_isEmpty(list_status))) {

		list_iter_t iter_pid, iter_status;
		list_iter_reset(&iter_pid, list_pid);
		list_iter_reset(&iter_status, list_status);

		pid_t* value_pid = list_iter_next(&iter_pid, list_pid);
		int* value_status = list_iter_next(&iter_status, list_status);

		while (list_iter_hasNext(&iter_pid, list_pid) && list_iter_hasNext(&iter_status, list_status)) {
			if (WIFEXITED(*value_status)) {
				if (!WEXITSTATUS(*value_status)) {
					printf("CHILD EXITED (PID=%d; return OK)\n", *value_pid);
					free(value_pid);
					free(value_status);
				}
				else {
					printf("CHILD EXITED (PID=%d; return NOK)\n", *value_pid);
					free(value_pid);
					free(value_status);
				}

				value_pid = list_iter_next(&iter_pid, list_pid);
				value_status = list_iter_next(&iter_status, list_status);
			}
		}

		if (WIFEXITED(*value_status)) {
			if (!WEXITSTATUS(*value_status)) {
				printf("CHILD EXITED (PID=%d; return OK)\n", *value_pid);
				free(value_pid);
				free(value_status);
			}
			else {
				printf("CHILD EXITED (PID=%d; return NOK)\n", *value_pid);
				free(value_pid);
				free(value_status);
			}
		}
	}

	printf("END.\n");
}


/* check if maxchildren is a positive number
 */
void check_maxchildren(const char* maxchildren) {
	int size = strlen(maxchildren), i;

	for (i = 0; i < size; i++) {
		if ((maxchildren[i] < '0') || (maxchildren[i] > '9')) {
			fprintf(stderr, "invalid arguments\n");
			exit(EXIT_FAILURE);
		}
	}
	return;
}


int main(int argc, char const *argv[]) {

	long maxchildren, count = 0;
	int n_args, status, status_child;

	pid_t pid;
	
	/* no comparation functions needed */
	list_t* list_pid = list_alloc(NULL);
	list_t* list_status = list_alloc(NULL);
	
	char* arg_vector[VECTOR_SIZE];
	char* buffer = (char*)malloc(sizeof(char)*(BUFFER_SIZE+1));


	/* verification maxchildren */
	if (argc > 2) {
		fprintf(stderr, "invalid arguments\n");
		exit(EXIT_FAILURE);
	}
	else if (argc == 2) {
		check_maxchildren(argv[1]);
		maxchildren = atoi(argv[1]);
		if (maxchildren == 0) {
			fprintf(stderr, "maxchildren can not be zero\n");
			exit(EXIT_FAILURE);
		}
	}
	else
		maxchildren = -1;


	for (;;) {
		if (count != maxchildren) {
			n_args = readLineArguments(arg_vector, VECTOR_SIZE, buffer, BUFFER_SIZE);

			/* verification number of arguments */
			if (n_args == 0) {
				fprintf(stderr, "no arguments\n");
				continue;
			}
			else if (n_args > 2) {
				fprintf(stderr, "too many arguments\n");
				continue;
			}


			/* verification command */
			if (!strcmp(arg_vector[0], "exit")) {
				if (n_args > 1) {
					fprintf(stderr, "too many arguments\n");
					continue;
				}
				break;
			}
			else if (!strcmp(arg_vector[0], "run"))
				pid = fork();
			else {
				fprintf(stderr, "invalid command\n");
				continue;
			}


			/* verification of pid */
			if (pid == -1) {
				perror("unable to fork");
			}
			else if (pid == 0) {
				status_child = execl("./CircuitRouter-SeqSolver", "./CircuitRouter-SeqSolver", arg_vector[1], NULL);
				perror("child error");
				exit(status_child);
			}
			else {
				count++;
			}
		}

		else { /* wait process and insert in list */
			pid_t* wait_pid = (pid_t*)malloc(sizeof(pid_t));
			int* wait_status = (int*)malloc(sizeof(int));
			
			*wait_pid = wait(&status);
			*wait_status = status;

			list_insert(list_pid, wait_pid);
			list_insert(list_status, wait_status);
			count--;
		}
	}


	long i;
	/* wait process and insert in list */
	for (i = count; i > 0; i--) {
		pid_t* wait_pid = (pid_t*)malloc(sizeof(pid_t));
		int* wait_status = (int*)malloc(sizeof(int));

		*wait_pid = wait(&status);
		*wait_status = status;

		list_insert(list_pid, wait_pid);
		list_insert(list_status, wait_status);
	}


	print_free_values(list_pid, list_status);


	free(buffer);
	list_free(list_pid);
	list_free(list_status);
}