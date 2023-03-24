#include "packer.h"
#include <stdio.h>
#include <semaphore.h>

#define RED 1
#define GREEN 2
#define BLUE 3
#define BLACK 4
#define N 2

// You can declare global variables here
typedef struct {
    int first_id;
    int second_id;
    int leaders;
    int followers;
    sem_t mutex;
    sem_t leaderQueue;
    sem_t followerQueue;
    sem_t first_ball_arrived;
    sem_t second_ball_arrived;
    sem_t result_retrieved;
} package_t;

package_t mutex_package_areas[N];

void packer_init(void) {
    // Write initialization code here (called once at the start of the program).
    for (int i = 0; i < 3; i++) {
        package_t* mutex_package_area = &mutex_package_areas[i];
        mutex_package_area->leaders = 0;
        mutex_package_area->followers = 0;
        mutex_package_area->first_id = -1; // null
        mutex_package_area->second_id = -1; // null
        sem_init(&mutex_package_area->mutex, 0, 1);
        sem_init(&mutex_package_area->leaderQueue, 0, 0);
        sem_init(&mutex_package_area->followerQueue, 0, 0);
        sem_init(&mutex_package_area->first_ball_arrived, 0, 0);
        sem_init(&mutex_package_area->second_ball_arrived, 0, 0);
        sem_init(&mutex_package_area->result_retrieved, 0, 0);
    }
}

void packer_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    for (int i = 0; i < 3; i++) {
        package_t* mutex_package_area = &mutex_package_areas[i];
        sem_destroy(&mutex_package_area->mutex);
        sem_destroy(&mutex_package_area->second_ball_arrived);
        sem_destroy(&mutex_package_area->result_retrieved);
        sem_destroy(&mutex_package_area->leaderQueue);
        sem_destroy(&mutex_package_area->followerQueue);
        sem_destroy(&mutex_package_area->first_ball_arrived);
    }
}

int pack_ball(int colour, int id) {
    // printf("Starting: colour: %i, id: %i", colour, id);
    int colour_idx;

    if (colour == RED || colour == GREEN) {
        colour_idx = 0;
    } else {
        colour_idx = 1;
    }
    package_t* mutex_package_area = &mutex_package_areas[colour_idx];
    int other_id;

    sem_wait(&mutex_package_area->mutex);


    if (colour == RED || colour == BLUE) { // leader
        if (mutex_package_area->followers > 0) {
            mutex_package_area->followers--;
            sem_post(&mutex_package_area->followerQueue); // wake up john
            // we don't release mutex, since we don't want another leader/follower pair to start executing
        } else {
            mutex_package_area->leaders++;
            sem_post(&mutex_package_area->mutex); // release the mutex since nothing we can do here
            sem_wait(&mutex_package_area->leaderQueue); // wake me, when you need me
        }

        mutex_package_area->first_id = id;
        sem_post(&mutex_package_area->first_ball_arrived);
        sem_wait(&mutex_package_area->second_ball_arrived);
        other_id = mutex_package_area->second_id;
        sem_post(&mutex_package_area->result_retrieved);
    } else { // follower
        if (mutex_package_area->leaders > 0) {
            mutex_package_area->leaders--;
            sem_post(&mutex_package_area->leaderQueue); // wake up john
            // we don't release mutex, since we don't want another leader/follower pair to start executing
        } else {
            mutex_package_area->followers++;
            sem_post(&mutex_package_area->mutex); // release the mutex since nothing we can do here
            sem_wait(&mutex_package_area->followerQueue); // don't make a girl a promise, if you know you can't keep it
        }
        
        sem_wait(&mutex_package_area->first_ball_arrived);
        mutex_package_area->second_id = id;

        sem_post(&mutex_package_area->second_ball_arrived);
        other_id = mutex_package_area->first_id;
        sem_wait(&mutex_package_area->result_retrieved);
        mutex_package_area->first_id = -1;
        mutex_package_area->second_id = -1;

        sem_post(&mutex_package_area->mutex);
    }

    return other_id;
}