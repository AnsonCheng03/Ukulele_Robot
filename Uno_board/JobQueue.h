#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <Arduino.h>

// Define the maximum size of the job queue
#define MAX_QUEUE_SIZE 3

// Define a type for job functions that take a parameter
typedef void (*JobFunction)(void* context);

struct Job {
    JobFunction function;
    void* context;  // Context parameter to pass data to the function
};

// Declare functions for managing the job queue
void enqueueJob(JobFunction job, void* context);
Job dequeueJob();
bool isJobQueueEmpty();
bool isJobQueueFull();

#endif // JOBQUEUE_H
