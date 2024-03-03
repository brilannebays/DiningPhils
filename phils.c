// Name: BRILANNE BAYS 
// Class: 345 - Operating Systems
// Instructor: Zakaria El-Awadi


// inclusions
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// definitions 
#define THINKING 0 
#define EATING 1
#define HUNGRY 2

// global variables
int numOfPhils; // number of phils, passed in as argument[1]
int numOfTimesToEat; // number of times to eat each, passed in as argument[2]
sem_t *chopsticks; // array of semaphores that represent the chopsticks
sem_t mutualExclusion; // semaphore for mutual exclusion
int *state; // state of each philosopher (eating, thinking, hungry)
pthread_t *threads; // an array for threads once they're created 


// tests if a philosopher at a given index is able to eat
void test(int index) { 
    if (state[index] == HUNGRY && state[(index + 1) % numOfPhils] != EATING && state[(index + numOfPhils - 1) % numOfPhils] != EATING) {
        state[index] = EATING; // changes the state of the philosopher at the given index to EATING 
        sem_post(&chopsticks[index]); // increments the semaphore at that address of the given index by 1, allowing a waiting thread to proceed

    }
} 

// waits to grab chopsticks for philospher (denotes when philospher is hungry)
void pickupChopstick(int index, int rightcs, int leftcs) {
    sem_wait(&mutualExclusion); // decrements the semaphore by 1, blocking other threads from accessing it 
    state[index] = HUNGRY; // philosopher is in a state where they are ready to eat 
    test(index); // test if philosopher can change states without causing a deadlock
    sem_post(&mutualExclusion); // increments the semaphore and releases the lock

    sem_wait(&chopsticks[rightcs]); // will pick up first chopstick if available (can't if it's not)
    sem_wait(&chopsticks[leftcs]); // if able to pick up first one, is allowed to pick up second one
} 

// puts chopsticks back down (denotes when philospher is thinking)
void putDownChopstick(int index, int rightcs, int leftcs) { 
    sem_post(&chopsticks[rightcs]); 
    sem_post(&chopsticks[leftcs]); // both chopsticks are released (semaphore incremented) 

    sem_wait(&mutualExclusion); // blocks the other threads from accessing critical section - this semaphore is locked
    state[index] = THINKING; // change state back to thinking 
    test((index + 1) % numOfPhils); // test if the right philosopher is available to eat 
    test((index + numOfPhils - 1) % numOfPhils); // test if the left philosopher is available to eat
    sem_post(&mutualExclusion); // release this semaphore

}

// function that simulates a philosopher taking turns thinking and eating a certain number of times
void *philosopher(void *arg) { 
    int index = *(int *)arg; // extract the pointer
    int rightcs = index;
    int leftcs = (index + 1) % numOfPhils;
    int timesEaten = 0;

    while (timesEaten < numOfTimesToEat) {
        // thinking 
        printf("Philosopher %d is thinking\n", index + 1);
        sleep(2);

        // eating  
        pickupChopstick(index, rightcs, leftcs);
        printf("Philosopher %d is eating\n", index + 1);
        sleep(2);
        putDownChopstick(index, rightcs, leftcs);

        timesEaten++;
    }
    pthread_exit(NULL);
}

// MAIN DRIVER CODE
int main(int argc, char *argv[]) {
    // define arguments
    if (argc != 3) {
        printf("ERROR: %i arguments given when 2 expected\n", argc - 1);
        return -1;
    }
    numOfPhils = atoi(argv[1]); // both of these convert string values from the command line to integer values
    numOfTimesToEat = atoi(argv[2]);

    // memory allocation for chopsticks, state, and threads
    threads = malloc(numOfPhils * sizeof(pthread_t));
    chopsticks = malloc(numOfPhils * sizeof(sem_t));
    state = malloc(numOfPhils * sizeof(int));

    // initialize semaphores 
    sem_init(&mutualExclusion, 0, 1);
    for (int i=0; i < numOfPhils; i++) {
        sem_init(&chopsticks[i], 0, 1);
        state[i] = THINKING;    // set the initial state of each philosopher to thinking
    }

    // create philosphers and give them a state based on numOfPhils then create a thread for each 
    for (int i=0; i < numOfPhils; i++) {
        int *index = malloc(sizeof(int));   // memory allocation for philosophers at each index
        *index = i;
        pthread_create(&threads[i], NULL, philosopher, index); 
        // creates threads and stores them in threads[i]. Threads scheduled to start at philosopher
    }
    
    // wait for all threads to finish executing before continuing 
    for (int i=0; i < numOfPhils; i++) {
        pthread_join(threads[i], NULL);
    }

    // deallocate memory
    free(threads);
    free(chopsticks);
    free(state);
    return 0;
}