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
Scrivere un programma che riceva in input tramite argv[] un numero di parametri
superiore a 1 (incluso il nome del programma). Ogni parametro registrato a partire
da argv[1] deve corrispondere ad una stringa di un unico carattere, ed ognuna di 
queste stringhe dovra' essere diversa dalle altre. 
Per ognuna di queste stringhe l'applicazione dovra' attivare un thread che ne 
effettuera' la gestione.
Dopo aver attivato tali thread, l'applicazione dovra' leggere ogni stringa proveniente 
da standard input e renderla disponibile a tutti i thread attivati in precedenza. 
Ciascuno di essi dovra' controllare se qualche carattre presente nella stringa proveniente
da standard input corrisponde al carattere della stringa che lui stesso sta gestendo, 
ed in tal caso dovra' sostituire quel carattere nella stringa proveniente dallo standard input
sovrascrivendolo col carattere '*'. 
Al termine delle attivita' di controllo e sostituzione di caratteri da parte 
di tutti i thread, l'applicazione dovra' scrivere su un file dal nome "output.txt"
la stringa originale proveniente da standard input e quella ottenuta tramite le 
sostituzioni di carattere (anche se non realmente avvenute), su due linee consecutive del file.

L'applicazione dovra' gestire il segnale  SIGINT (o CTRL_C_EVENT nel caso WinAPI) 
in modo tale che quando il processo venga colpito dovra' riportare su standard output 
le stringhe presenti in output.txt che possono aver subito sostituzione di carattere.

In caso non vi sia immissione di dati sullo standard input e non vi siano segnalazioni, 
l'applicazione dovra' utilizzare non piu' del 5% della capacita' di lavoro della CPU.

*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>

#define MAX_LEN 1024
#define MAX_PROC 256

volatile sig_atomic_t sigint_received = 0;

void handle_sigint(int sig) {
    sigint_received = 1;
}

// Controlla se la stringa è stata modificata
int has_changes(const char *orig, const char *mod) {
    return strcmp(orig, mod) != 0;
}

// Stampa le righe modificate da output.txt
void print_modified_lines(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return;
    }
    char orig[MAX_LEN], mod[MAX_LEN];
    while (fgets(orig, MAX_LEN, fp) && fgets(mod, MAX_LEN, fp)) {
        orig[strcspn(orig, "\n")] = 0;
        mod[strcspn(mod, "\n")] = 0;
        if (has_changes(orig, mod)) {
            printf("%s\n%s\n", orig, mod);
        }
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <carattere1> <carattere2> ...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = argc - 1;
    char chars[MAX_PROC];

    for (int i = 0; i < n; i++) {
        if (strlen(argv[i+1]) != 1 || !isprint(argv[i+1][0])) {
            fprintf(stderr, "Ogni parametro deve essere un singolo carattere stampabile.\n");
            exit(EXIT_FAILURE);
        }
        chars[i] = argv[i+1][0];
        for (int j = 0; j < i; j++) {
            if (chars[i] == chars[j]) {
                fprintf(stderr, "I caratteri devono essere unici.\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    signal(SIGINT, handle_sigint);

    // memoria condivisa
    // CHANGE
    char *shared_mem = mmap(NULL, MAX_LEN, PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // semafori per sincronizzazione, 1 for "string ready" , 1 for "sub done"
    // CHANGE
	int sem_string_ready = 0;
	int sem_sub_done = 1;
    int semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // inizializzazione a 0
	// CHANGE
	unsigned short start_vals[] = {0, 0};
	if (semctl(semid, 0, SETALL, start_vals) == -1) {
	    perror("semctl SETALL");
	    exit(EXIT_FAILURE);
	}

    pid_t pids[MAX_PROC];

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // figlio
            char my_char = chars[i];

            struct sembuf wait_string = {sem_string_ready, -1, 0};
            struct sembuf sub_done = {sem_sub_done, 1, 0};

            while (1) {
                semop(semid, &wait_string, 1); // aspetta nuova stringa

                for (int j = 0; shared_mem[j]; j++) {
                    if (shared_mem[j] == my_char) {
                        shared_mem[j] = '*';
                    }
                }
                semop(semid, &sub_done, 1); // segnala completamento
            }

            exit(0);
        } else if (pid > 0) {
            pids[i] = pid;
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    FILE *fout = fopen("output.txt", "a");
    if (!fout) {
        perror("fopen output.txt");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LEN];
    while (!sigint_received) {
        printf("Inserisci una stringa (CTRL+C per uscire): ");
        if (!fgets(line, MAX_LEN, stdin)) {
            sleep(1);
            continue;
        }

        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;

        strcpy(shared_mem, line);

        // segnala a tutti i figli
            struct sembuf signal_child = {sem_string_ready, n, 0};
            semop(semid, &signal_child, 1);

        // attende che tutti i figli abbiano completato
            struct sembuf wait_parent = {sem_sub_done, -n, 0};
            semop(semid, &wait_parent, 1);

        fprintf(fout, "%s\n%s\n", line, shared_mem);
        fflush(fout);
    }

    printf("\n[INFO] Ricevuto SIGINT. Ecco le righe modificate:\n");
    print_modified_lines("output.txt");

    // termina i figli
    for (int i = 0; i < n; i++) {
        kill(pids[i], SIGKILL);
        waitpid(pids[i], NULL, 0);
    }

    semctl(semid, 0, IPC_RMID); // elimina semafori
    munmap(shared_mem, MAX_LEN);
    fclose(fout);

    return 0;
}

