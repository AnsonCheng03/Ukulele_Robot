#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <Arduino.h>

// Define the maximum size of the job queue
#define MAX_QUEUE_SIZE 10

typedef void (*JobFunction)(); // Define a type for job functions

struct Job {
    JobFunction function;
};

// Declare functions for managing the job queue
void enqueueJob(JobFunction job);
Job dequeueJob();
bool isJobQueueEmpty();
bool isJobQueueFull();

#endif // JOBQUEUE_H
    