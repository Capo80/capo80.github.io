#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 5  

int winner = 0;

DWORD WINAPI runner(LPVOID lpParam) {
    int id = *(int*)lpParam;
    int wait_time = (rand() % 5) + 1;
    Sleep(wait_time * 1000);

    if (winner == 0) {
        winner = id;
        printf("Corridore %d ha vinto la gara!\n", id);
    }

    return 0;
}

int main() {
    srand((unsigned int)time(NULL));

    HANDLE threads[N];
    int* ids = malloc(N * sizeof(int));

    for (int i = 0; i < N; i++) {
        ids[i] = i + 1;
        threads[i] = CreateThread(
            NULL,
            0,
            runner,
            &ids[i],
            0,
            NULL
        );
    }

    WaitForMultipleObjects(N, threads, TRUE, INFINITE);

    for (int i = 0; i < N; i++) {
        CloseHandle(threads[i]);
    }

    free(ids);
    return 0;
}
