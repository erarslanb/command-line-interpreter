
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


/*Bilkent University Spring 2019
 *CS 342 Operating Systems Course - Project 1
 *Simple Command Line Interpreter
 *
 * --Batuhan Erarslan
 */


FILE *fp;
char userInput[100];
char fileInput[100];

char *string;
char *array[512];
char *array2[512];
char *commands[512];


void displayPrompt() {
    printf("Input a single command, or two commands separated by '|':\n ");
}

//Divide compound commands into separate commands
void separateCommands(char *input) {
    int i = 0;
    string = strtok(input, "|");  //divide from pipe symbol
    while (string != NULL) {
        commands[i] = strdup(string);
        i++;
        string = strtok(NULL, "|");
    }
    commands[i] = NULL;
}

// Divide the input into strings
void separateStrings(char *input, char **array) {
    int i = 0;
    string = strtok(input, "\n ");  //divide by newline or space
    while (string != NULL) {
        array[i] = strdup(string);
        i++;
        string = strtok(NULL, "\n ");
    }
    array[i] = NULL;
}

void execute() {
    int child = fork(); // Create child process

    if (child == 0) {           // execute child process
        if (execvp(array[0], array) == -1) {
            perror("Could not execute command");
            exit(errno);
        }
    }else {         //wait for completion
        wait(NULL);
        }
}


//Execute compound commands
int executeTwo(char **command1, char **command2, int N) {

    pid_t child1, child2;
    int p1[2], p2[2];

    char *toread = (char *) calloc(N, sizeof(char));  //allocate buffer

    if (pipe(p1) < 0) {     //Open the first pipe- exit with error message if not successful
        printf("\nCould not create pipe");
        exit(1);
    }

    child1 = fork();   //create child process

    if (child1 < 0) {     //exit if fork is not successful
        printf("\nCould not create child");
        exit(0);
    }

    if (child1 == 0) {                //execute child process
        dup2(p1[1], STDOUT_FILENO);

        if (execvp(command1[0], command1) < 0) {
            printf("\nCould not execute command");
            exit(0);
        }
    } else if (child1 > 0) {    //parent process

        if (pipe(p2) < 0) {     //open the second pipe- exit with error message if not successful
            printf("\nCould not create pipe");
            exit(1);
        }


        int r = 0;
        int total = 0;

        do {
            r = read(p1[0], toread, N);    //read from first pipe into buffer 'toread'
            lseek(p1[0], N, SEEK_CUR);

            total = total + r;

            write(p2[1], toread, r);        //write into the read end of the second pipe
            lseek(p2[1], r, SEEK_CUR);

        } while (r == N);

        close(p1[0]);
        close(p2[1]);

        child2 = fork();         //create second child process


        if (child2 < 0) {       //exit with error message if fork is not successful
            printf("\nCould not create ch,ild");
            exit(0);
        }

        if (child2 == 0) {
            dup2(p2[0], STDIN_FILENO);

            if (execvp(command2[0], command2) < 0) {
                printf("\nCould not execute command");
                exit(0);
            }

        } else {
            wait(NULL);
            wait(NULL);
        }

        return total;
    }
}


int main(int argc, char *argv[]) {
    int N;

    while (1) {
        if (argc >= 2) {         //check if there are arguments
            N = atoi(argv[1]);

            if (N < 1 || N > 4096) {  //check the range for N
                printf("\nN too large");
                return (-1);
            } else {
                if (argc >= 3) {       //check for filename
                    fp = fopen(argv[2], "r");
                    if (fp == NULL) {
                        printf("\nCannot open the file");
                        return (-1);
                    }

                    while (fgets(fileInput, sizeof(fileInput), fp) != NULL) { //open file
                        char *flag = strchr(fileInput, '|'); //check for pipes

                        if (flag != NULL) {            //execute compound command
                            separateCommands(fileInput);
                            separateStrings(commands[0], array);
                            separateStrings(commands[1], array2);


                            //executeTwo returns the number of bytes read
                            int byteCount = executeTwo(array, array2, N);

                            int count = (byteCount / N);    //calculate the no. of
                                                            //r-w operations
                            if ((byteCount % N) > 0)
                                count++;

                            printf("character-count: %d\n", byteCount);
                            printf("read-call-count: %d\n", count);

                            break;
                        } else {                  //execute single command
                            puts(fileInput);
                            separateStrings(fileInput, array);
                            execute();
                            break;
                        }
                    }
                    break;
                } else {                         //execute from user input- the code is
                                                 //almost identical to the part above

                    displayPrompt();            //prompt for input
                    fgets(userInput, 100, stdin);

                    char *flag = strchr(userInput, '|');

                    if (flag != NULL) {
                        separateCommands(userInput);
                        separateStrings(commands[0], array);
                        separateStrings(commands[1], array2);


                        int byteCount = executeTwo(array, array2, N);

                        int count = (byteCount / 1000);

                        if ((byteCount % 1000) > 0)
                            count++;

                        printf("character-count: %d\n", byteCount);
                        printf("read-call-count: %d\n", count);

                        break;
                    } else {
                        puts(userInput);
                        separateStrings(userInput, array);
                        execute();
                        break;
                    }

                }
            }
        }
    }
}



