/******************************************************************
Welcome to the Operating System examination

You are editing the '/home/esame/prog.c' file. You cannot remove 
this file, just edit it so as to produce your own program according to
the specification listed below.

In the '/home/esame/'directory you can find a Makefile that you can 
use to compile this program to generate an executable named 'prog' 
in the same directory. Typing 'make posix' you will compile for 
Posix, while typing 'make winapi' you will compile for WinAPI just 
depending on the specific technology you selected to implement the
given specification. Most of the required header files (for either 
Posix or WinAPI compilation) are already included in the head of the
prog.c file you are editing. 

At the end of the examination, the last saved snapshot of this file
will be automatically stored by the system and will be then considered
for the evaluation of your exam. Modifications made to prog.c which are
not saved by you via the editor will not appear in the stored version
of the prog.c file. 
In other words, unsaved changes will not be tracked, so please save 
this file when you think you have finished software development.
You can also modify the Makefile if requesed, since this file will also
be automatically stored together with your program and will be part
of the final data to be evaluated for your exam.

PLEASE BE CAREFUL THAT THE LAST SAVED VERSION OF THE prog.c FILE (and of
the Makfile) WILL BE AUTOMATICALLY STORED WHEN YOU CLOSE YOUR EXAMINATION 
VIA THE CLOSURE CODE YOU RECEIVED, OR WHEN THE TIME YOU HAVE BEEN GRANTED
TO DEVELOP YOUR PROGRAM EXPIRES. 


SPECIFICATION TO BE IMPLEMENTED:
Implementare una programma che riceva in input, tramite argv[], 
N differenti stringhe S1 ... SN, con N maggiore o uguale a 1.
Per ognuna delle stringhe dovra' essere attivato un nuovo processo  
che gestira' tale stringa  (indichiamo quindi con P1 ... PN i 
processi che dovranno essere attivati).
Il processo originale dovra' leggere stringhe dallo standard input, e dovra'
comunicare ogni stringa letta a P1. P1 dovra' verificare se la stringa ricevuta
e' uguale alla stringa S1 da lui gestita, e dovra' incrementare un contatore
in caso positivo. Altrimenti, in caso negativo, dovra' comunicare la stringa
ricevuta al processo P2 che fara' lo stesso controllo, e cosi' via fino a PN.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando uno qualsiasi dei processi Pi venga colpito
esso dovra' riportare su standard output il valore del contatore che indica
quante volte la stringa Si e' stata trovata uguale alla stringa che 
il processo originale aveva letto da standard input. Il processo originale 
non dovra' invece eseguire alcuna attivita' all'arrivo della segnalazione.

In caso non vi sia immissione di dati sullo standard input, e non vi siano 
segnalazioni, l'applicazione dovra' utilizzare non piu' del 5% della capacita' 
di lavoro della CPU.

*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_LEN 1024

int counter = 0;
char *my_string = NULL;

void sigint_handler(int sig) {
    printf("Process %d matched \"%s\" %d times\n", getpid(), my_string, counter);
    fflush(stdout);
    exit(0);
}


void sigint_handler_father(int sig) {
    /* pid_t wpid; */
    int status = 0;
    while ((wpid = wait(&status)) > 0);
    exit(0);
}


void child_process(int read_fd, int write_fd, char *my_str) {
    char buffer[MAX_LEN];
    my_string = my_str;

    signal(SIGINT, sigint_handler);

    while (1) {
        int n = read(read_fd, buffer, MAX_LEN);
        if (n <= 0) {
            break;
        }
        buffer[n] = '\0';
        buffer[strcspn(buffer, "\n")] = '\0'; // Rimuovi newline
        printf("Process (%s) read %s\n", my_string, buffer);

        if (strcmp(buffer, my_string) == 0) {
            counter++;
        } else if (write_fd != -1) {
            write(write_fd, buffer, strlen(buffer));
        }
    }
    printf("Process (%s) exiting\n", my_string);

    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s string1 [string2 ... stringN]\n", argv[0]);
        exit(1);
    }

    int N = argc - 1;
    int pipes[N][2];

    for (int i = 0; i < N; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    signal(SIGINT, sigint_handler_father);

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child
            for (int j = 0; j < N; j++) {
                if (j == i)
                    close(pipes[j][1]); // chiudi write end della propria pipe
                else
                    if (j == i+1)
                        close(pipes[j][0]); // close only read of next pipe
                else {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            int read_fd = pipes[i][0];
            int write_fd = (i + 1 < N) ? pipes[i + 1][1] : -1;
            child_process(read_fd, write_fd, argv[i + 1]);
        }
    }

    // Parent process
    for (int i = 1; i < N; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    close(pipes[0][0]); // chiude lettura prima pipe

    char input[MAX_LEN];
    while (1) {
        if (fgets(input, MAX_LEN, stdin) != NULL) {
            write(pipes[0][1], input, strlen(input));
        } 
    }

    return 0;
}

