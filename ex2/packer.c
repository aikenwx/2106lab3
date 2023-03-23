#include <semaphore.h>
#include <stddef.h>

#include "packer.h"

// You can declare global variables here
typedef struct {
    int current_id;
    int ball_ids[64];
    int balls_per_pack;
    sem_t mutex;
    sem_t last_ball_arrived;
    sem_t result_retrieved[63];
} package_t;

package_t mutex_package_areas[3];


void packer_init(int balls_per_pack) {

      // Write initialization code here (called once at the start of the program).
    for (int i = 0; i < 3; i++) {
        package_t* mutex_package_area = &mutex_package_areas[i];
        mutex_package_area->current_id = 0;
        mutex_package_area->balls_per_pack = balls_per_pack;
        sem_init(&mutex_package_area->mutex, 0, 1);
        sem_init(&mutex_package_area->last_ball_arrived, 0, 0);

        for (int i = 0; i < balls_per_pack - 1; i++) {
          sem_init(&mutex_package_area->result_retrieved[i], 0, 0);
        }
    }
}

void packer_destroy(void) {
    for (int i = 0; i < 3; i++) {
        package_t* mutex_package_area = &mutex_package_areas[i];
        sem_destroy(&mutex_package_area->mutex);
        sem_destroy(&mutex_package_area->last_ball_arrived);

        for (int i = 0; i < mutex_package_area->balls_per_pack - 1; i++) {
          sem_destroy(&mutex_package_area->result_retrieved[i]);
        }
    }
}

void pack_ball(int colour, int id, int *other_ids) {

    // printf("Starting: colour: %i, id: %i", colour, id);
    
    int colour_idx = colour - 1;
    package_t* mutex_package_area = &mutex_package_areas[colour_idx];
    // Write your code here.
    // If 1st ball is null
    // Record down the first ball's ID
    // Wait for the 2nd ball to arrive before returning the 2nd ball's id
    sem_wait(&mutex_package_area->mutex);


    if (mutex_package_area->current_id < mutex_package_area->balls_per_pack - 1) {
        mutex_package_area->ball_ids[mutex_package_area->current_id] = id;
        int saved_current_id = mutex_package_area->current_id;
        mutex_package_area->current_id += 1;
        sem_post(&mutex_package_area->mutex);
        sem_wait(&mutex_package_area->last_ball_arrived);


        for (int i = 0; i < saved_current_id; i++) {
          other_ids[i] = mutex_package_area->ball_ids[i];
        }

        for (int i = saved_current_id + 1; i < mutex_package_area->balls_per_pack; i++) {
          other_ids[i - 1] = mutex_package_area->ball_ids[i];
        }


        sem_post(&mutex_package_area->result_retrieved[saved_current_id]);

    } else {
    // Record down the 2nd ball's ID
    // Signal that the 2nd ball has arrived in the packing area
    // Store the 1st ball's id in some temp var
    // Reset ball_ids[colour_idx] to be {NULL, NULL}
    // Signal that the packing area is free
    // Return the 1st ball's id
        mutex_package_area->ball_ids[mutex_package_area->current_id] = id;

        for (int i = 0; i < mutex_package_area->balls_per_pack - 1; i++) {
            sem_post(&mutex_package_area->last_ball_arrived);
        }

        for (int i = 0; i < mutex_package_area->current_id; i++) {
          other_ids[i] = mutex_package_area->ball_ids[i];
        }

        for (int i = 0; i < mutex_package_area->balls_per_pack -1; i++) {
          sem_wait(&mutex_package_area->result_retrieved[i]);
        }
        mutex_package_area->current_id = 0;
        for (int i = 0; i < mutex_package_area->balls_per_pack; i++) {
            mutex_package_area->ball_ids[i] = -1;
        }

        sem_post(&mutex_package_area->mutex);
    }

}
