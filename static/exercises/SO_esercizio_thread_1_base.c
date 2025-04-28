#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define N 5 

int winner = 0; 

void* runner(void* arg) {
    int id = *(int*)arg;
    int wait_time = rand() % 5 + 1; 
    sleep(wait_time);
    
    if (winner == 0) {
        winner = id;
        printf("Corridore %d ha vinto la gara!\n", id);
    }
    
    return NULL;
}

int main() {
    srand(0);
    pthread_t threads[N];
    int ids[N];
    
    for (int i = 0; i < N; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, runner, &ids[i]);
    }
    
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}
