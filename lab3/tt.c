#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t m1, m2;

void* first(void* data) { 
    printf("First\n");
    sem_post(&m1);
}

void* second(void* data) { 
    sem_wait(&m1);
    printf("Second\n"); 
    sem_post(&m1);
    sem_post(&m2);
}

void* third(void* data) { 
    sem_wait(&m2);
    printf("Third\n");
}

int main () {
    
    pthread_t t1, t2, t3;

    sem_init(&m1, 0, 0);
    sem_init(&m2, 0, 0);

    pthread_create(&t3, NULL, third, NULL);
    pthread_create(&t2, NULL, second, NULL);
    pthread_create(&t1, NULL, first, NULL);

    /* wait for all threads */
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
}