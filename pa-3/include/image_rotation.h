#ifndef IMAGE_ROTATION_H_
#define IMAGE_ROTATION_H_

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define CHANNEL_NUM 1

#include "stb_image.h"
#include "stb_image_write.h"

/********************* [ Helpful Macro Definitions ] **********************/
#define BUFF_SIZE 1024
#define LOG_FILE_NAME "request_log"  // Standardized log file name
#define INVALID -1  // Reusable int for marking things as invalid or incorrect
#define MAX_THREADS 100    // Maximum number of threads
#define MAX_QUEUE_LEN 100  // Maximum queue length
#define TRUE 1
#define FALSE 0
/********************* [ Helpful Typedefs        ] ************************/

typedef struct request_queue {
  // what data do you need here?
  char filename[BUFF_SIZE];
  int rotation_angle;
  struct request_queue *next;
} request_t;

typedef struct processing_args {
  // what data do you need here?
  char *input_directory;
  int number_thread;
  int rotation_angle;
  int *processing_complete;
} processing_args_t;

typedef struct worker_args {
  char *input_directory;
  char *output_directory;
  int thread_id;
  int *worker_complete;
  FILE *fp;
  int requests_handled;
} worker_args_t;

/********************* [ Function Prototypes       ] **********************/
void *processing(void *args);
void *worker(void *args);
void log_pretty_print(FILE *to_write, int threadId, int requestNumber,
                      char *file_name);

#endif
