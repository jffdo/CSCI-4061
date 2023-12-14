#include "image_rotation.h"

// Global integer to indicate the length of the queue??
// Global integer to indicate the number of worker threads
// Global file pointer for writing to log file in worker??
// Might be helpful to track the ID's of your threads in a global array
// What kind of locks will you need to make everything thread safe? [Hint you
// need multiple] What kind of CVs will you need  (i.e. queue full, queue empty)
// [Hint you need multiple] How will you track the requests globally between
// threads? How will you ensure this is thread safe? How will you track which
// index in the request queue to remove next? How will you update and utilize
// the current number of requests in the request queue? How will you track the
// p_thread's that you create for workers? How will you know where to insert the
// next request received into the request queue?

/*
    The Function takes:
    to_write: A file pointer of where to write the logs.
    requestNumber: the request number that the thread just finished.
    file_name: the name of the file that just got processed.

    The function output:
    it should output the threadId, requestNumber, file_name into the logfile and
   stdout.
*/
request_t *queue_list = NULL;
int queue_length = 0;

pthread_mutex_t processsing_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t work_cond = PTHREAD_COND_INITIALIZER;

void log_pretty_print(FILE *to_write, int threadId, int requestNumber,
                      char *file_name) {
  char buffer[BUFF_SIZE];
  sprintf(buffer, "[%d][%d][%s]\n", threadId, requestNumber, file_name);
  if (fprintf(to_write, "%s", buffer) < 0) {
    perror("Unable to write\n");
    exit(-1);
  }
  // Print to stdout
  fprintf(stdout, "%s", buffer);
}

/*

    1: The processing function takes a void* argument called args. It is
   expected to be a pointer to a structure processing_args_t that contains
   information necessary for processing.

    2: The processing thread need to traverse a given dictionary and add its
   files into the shared queue while maintaining synchronization using lock and
   unlock.

    3: The processing thread should pthread_cond_signal/broadcast once it finish
   the traversing to wake the worker up from their wait.

    4: The processing thread will block(pthread_cond_wait) for a condition
   variable until the workers are done with the processing of the requests and
   the queue is empty.

    5: The processing thread will cross check if the condition from step 4 is
   met and it will signal to the worker to exit and it will exit.

*/

void *processing(void *args) {
  processing_args_t *processing_args = (processing_args_t *)args;
  DIR *dir = opendir(processing_args->input_directory);
  if (dir == NULL) {
    perror("Unable to open directory\n");
    exit(-1);
  }

  if (pthread_mutex_lock(&processsing_lock) != 0) {
    perror("Unable to create lock\n");
    exit(-1);
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if ((strcmp(entry->d_name, ".") == 0) ||
        (strcmp(entry->d_name, "..") == 0)) {
      continue;
    } else {
      if ((entry->d_type == DT_REG) &&
          (strstr(entry->d_name, ".png") != NULL)) {  // Only use .png
        // printf("FILE FOUND: %s\n", entry->d_name);
        request_t *new_image = malloc(sizeof(request_t));
        if (new_image == NULL) {
          perror("Unable to malloc\n");
          exit(-1);
        }
        strcpy(new_image->filename, entry->d_name);
        new_image->rotation_angle = processing_args->rotation_angle;
        new_image->next = NULL;
        if (queue_list == NULL) {
          queue_list = new_image;
        } else {
          request_t *temp = queue_list;
          while (temp->next != NULL) {
            temp = temp->next;
          }
          temp->next = new_image;
        }
        queue_length++;
        if (pthread_cond_signal(&work_cond) != 0) {
          perror("Unable to signal in process");
          exit(-1);
        }
      }
    }
  }
  if (pthread_mutex_unlock(&processsing_lock) != 0) {
    perror("Unable to unlock mutex\n");
    exit(-1);
  }
  if (closedir(dir) < 0) {
    perror("Unable to close directory\n");
    exit(-1);
  }
  *(processing_args->processing_complete) = TRUE;
  return NULL;
}

/*
    1: The worker threads takes an int ID as a parameter

    2: The Worker thread will block(pthread_cond_wait) for a condition
   variable that there is a requests in the queue.

    3: The Worker threads will also block(pthread_cond_wait) once the queue is
   empty and wait for a signal to either exit or do work.

    4: The Worker thread will processes request from the queue while
   maintaining synchronization using lock and unlock.

    5: The worker thread will write the data back to the given output dir as
   passed in main.

    6:  The Worker thread will log the request from the queue while
   maintaining synchronization using lock and unlock.

    8: Hint the worker thread should be in a While(1) loop since a worker
   thread can process multiple requests and It will have two while loops in
   total that is just a recommendation feel free to implement it your way :)
   9: You may need different lock depending on the job.

*/

void *worker(void *args) {
  /*
      Stbi_load takes:
          A file name, int pointer for width, height, and bpp

  */
  worker_args_t *worker_thread = (worker_args_t *)args;
  if (pthread_mutex_lock(&processsing_lock) != 0) {
    perror("Mutex lock failed\n");
    exit(-1);
  }

  while (queue_length == 0) {
    if (*(worker_thread->worker_complete) == TRUE) {
      if (pthread_mutex_unlock(&processsing_lock) != 0) {
        perror("Mutex unlock failed\n");
        exit(-1);
      }
      // Queue is complete
      free(worker_thread);
      pthread_exit(NULL);
    }
    if (pthread_cond_wait(&work_cond, &processsing_lock) != 0) {
      perror("Wait in worker failed\n");
      exit(-1);
    }
  }

  char *filename = malloc(strlen(queue_list->filename));
  if (filename == NULL) {
    perror("Unable to malloc\n");
    exit(-1);
  }
  strcpy(filename, queue_list->filename);
  int rotation = queue_list->rotation_angle;
  request_t *current = queue_list;
  request_t *temp = current;
  queue_list = queue_list->next;
  free(temp);
  queue_length--;

  char input_buffer[BUFF_SIZE];
  sprintf(input_buffer, "%s/%s", worker_thread->input_directory, filename);
  log_pretty_print(worker_thread->fp, worker_thread->thread_id,
                   worker_thread->requests_handled, input_buffer);
  if (pthread_mutex_unlock(&processsing_lock) != 0) {
    perror("Mutex unlock failed\n");
    exit(-1);
  }

  int width, height, bbp;
  uint8_t *image_result =
      stbi_load(input_buffer, &width, &height, &bbp, CHANNEL_NUM);
  // uint8_t* image_result = stbi_load("??????","?????", "?????",
  // "???????", CHANNEL_NUM);
  uint8_t **result_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * width);
  uint8_t **img_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * width);
  for (int i = 0; i < width; i++) {
    result_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
    img_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
  }
  /*
  linear_to_image takes:
      The image_result matrix from stbi_load
      An image matrix
      Width and height that were passed into stbi_load

  */
  // linear_to_image("??????", "????", "????", "????");
  linear_to_image(image_result, img_matrix, width, height);

  ////TODO: you should be ready to call flip_left_to_right or flip_upside_down
  /// depends on the angle(Should just be 180 or 270)
  // both take image matrix from linear_to_image, and result_matrix to store
  // data, and width and height. Hint figure out which function you will call.
  // flip_left_to_right(img_matrix, result_matrix, width, height); or
  // flip_upside_down(img_matrix, result_matrix ,width, height);

  // uint8_t* img_array = NULL; ///Hint malloc using sizeof(uint8_t) * width *
  // height
  uint8_t *img_array = malloc(sizeof(uint8_t) * width * height);
  if (rotation == 270) {
    flip_upside_down(img_matrix, result_matrix, width, height);
  } else {
    flip_left_to_right(img_matrix, result_matrix, width, height);
  }
  /// TODO: you should be ready to call flatten_mat function, using
  /// result_matrix
  // img_arry and width and height;
  // flatten_mat("??????", "??????", "????", "???????");
  flatten_mat(result_matrix, img_array, width, height);

  /// TODO: You should be ready to call stbi_write_png using:
  // New path to where you wanna save the file,
  // Width
  // height
  // img_array
  // width*CHANNEL_NUM
  // stbi_write_png("??????", "?????", "??????", CHANNEL_NUM, "??????",
  // "?????"*CHANNEL_NUM);
  char buffer[BUFF_SIZE];
  sprintf(buffer, "%s/%s", worker_thread->output_directory, filename);
  stbi_write_png(buffer, width, height, CHANNEL_NUM, img_array,
                 width * CHANNEL_NUM);
  free(worker_thread);
  free(filename);
  return NULL;
}

/*
    Main:
        Get the data you need from the command line argument
        Open the logfile
        Create the threads needed
        Join on the created threads
        Clean any data if needed.


*/

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr,
            "Usage: File Path to image dirctory, File path to output dirctory, "
            "number of worker thread, and Rotation angle\n");
  }
  // Get the data you need from the command line argument
  char *input_directory = argv[1];
  char *output_directory = argv[2];
  int number_thread = atoi(argv[3]);
  int rotation_angle = atoi(argv[4]);

  // Open the logfile
  FILE *fp = fopen(LOG_FILE_NAME, "w");
  if (fp == NULL) {
    perror("Unable to open file\n");
    exit(-1);
  }
  // Create the threads needed
  int status = FALSE;
  pthread_t processing_thread;
  processing_args_t pargs = {input_directory, number_thread, rotation_angle,
                             &status};
  pthread_t worker_thread[number_thread];
  if (pthread_create(&processing_thread, NULL, (void *)processing, &pargs) <
      0) {
    perror("Unable to create processing thread\n");
    exit(-1);
  }
  if (pthread_join(processing_thread, NULL) != 0) {
    perror("Join thread failed\n");  // wait for processing thread
    exit(-1);
  };
  int requests = 1;
  for (int i = 0; i < number_thread; i++) {
    worker_args_t *wargs = (worker_args_t *)malloc(sizeof(worker_args_t));
    if (wargs == NULL) {
      perror("Unable to malloc in main\n");
      exit(-1);
    }
    wargs->input_directory = input_directory;
    wargs->output_directory = output_directory;
    wargs->thread_id = i;
    wargs->worker_complete = &status;
    wargs->fp = fp;
    wargs->requests_handled = requests;
    if (pthread_create(&worker_thread[i], NULL, (void *)worker, wargs) < 0) {
      perror("Unable to create worker thread\n");
      exit(-1);
    }
  }
  // Join on created threads
  for (int i = 0; i < number_thread; i++) {
    if (pthread_join(worker_thread[i], NULL) != 0) {
      perror("Join worker thread failed\n");
      exit(-1);
    }
  }
  // Create another set of number_threads for remaining queue
  while (queue_length > 0) {
    requests++;
    for (int i = 0; i < number_thread; i++) {
      worker_args_t *wargs = (worker_args_t *)malloc(sizeof(worker_args_t));
      if (wargs == NULL) {
        perror("Unable to malloc in main\n");
        exit(-1);
      }
      wargs->input_directory = input_directory;
      wargs->output_directory = output_directory;
      wargs->thread_id = i;
      wargs->worker_complete = &status;
      wargs->fp = fp;
      wargs->requests_handled = requests;
      if (pthread_create(&worker_thread[i], NULL, (void *)worker, wargs) < 0) {
        perror("Unable to create worker thread");
        exit(-1);
      }
    }
    for (int i = 0; i < number_thread; i++) {
      if (pthread_join(worker_thread[i], NULL) != 0) {
        perror("Join worker thread failed");
        exit(-1);
      }
    }
  }
  if (fclose(fp) < 0) {
    perror("Unable to close file");
  }
  // Check for remaining items in queue
  // request_t *currents = queue_list;
  // printf("Files found left in queue:\n");
  // while (currents != NULL) {
  //   printf("NAME:%10s ROTATION:%4d\n", currents->filename,
  //          currents->rotation_angle);
  //   currents = currents->next;
  // }
}