#include "packer.h"
#include <stdio.h>
#include <semaphore.h>

#define RED 1
#define GREEN 2
#define BLUE 3
#define N 2

// You can declare global variables here
typedef struct {
    int first_id;
    int second_id;
    int first_id_is_null;
    sem_t mutex;
    sem_t second_ball_arrived;
    sem_t result_retrieved;
} package_t;

package_t mutex_package_areas[3];

void packer_init(void) {
    // Write initialization code here (called once at the start of the program).
    for (int i = 0; i < 3; i++) {
        package_t* mutex_package_area = &mutex_package_areas[i];
        mutex_package_area->first_id_is_null = 1;
        mutex_package_area->first_id = -1; // null
        mutex_package_area->second_id = -1; // null
        sem_init(&mutex_package_area->mutex, 0, 1);
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
    }
}

int pack_ball(int colour, int id) {
    // printf("Starting: colour: %i, id: %i", colour, id);
    int colour_idx = colour - 1;
    package_t* mutex_package_area = &mutex_package_areas[colour_idx];
    int other_id;
    // Write your code here.
    // If 1st ball is null
    // Record down the first ball's ID
    // Wait for the 2nd ball to arrive before returning the 2nd ball's id
    sem_wait(&mutex_package_area->mutex);


    if (mutex_package_area->first_id_is_null) {
        mutex_package_area->first_id_is_null = 0;
        mutex_package_area->first_id = id;
        sem_post(&mutex_package_area->mutex);
        sem_wait(&mutex_package_area->second_ball_arrived);
        other_id = mutex_package_area->second_id;
        sem_post(&mutex_package_area->result_retrieved);
    } else {
    // Record down the 2nd ball's ID
    // Signal that the 2nd ball has arrived in the packing area
    // Store the 1st ball's id in some temp var
    // Reset ball_ids[colour_idx] to be {NULL, NULL}
    // Signal that the packing area is free
    // Return the 1st ball's id
        mutex_package_area->second_id = id;
        sem_post(&mutex_package_area->second_ball_arrived);
        other_id = mutex_package_area->first_id;
        sem_wait(&mutex_package_area->result_retrieved);
        mutex_package_area->first_id = -1;
        mutex_package_area->second_id = -1;
        mutex_package_area->first_id_is_null = 1;
        sem_post(&mutex_package_area->mutex);
    }

    return other_id;
}