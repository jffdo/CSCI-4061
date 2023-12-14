# Project 3 / Final

## Developed by
Jeffrey Do (do000043@umn.edu)
## Testing

Tested on lab machine csel-kh1250-05.cselabs.umn.edu

## Indiviual Contribution

Jeffrey Do, image_rotation.h, image_rotation.c

## Changes to Makefile
Added rm -f request_log to make clean

## Psuedocode
```
'>>>' is ised to indicate changes from intermediate
```
### image_rotation.h
_only showing changed sections_
```
/********************* [ Helpful Typedefs        ] ************************/
>>> #define TRUE 1
>>> #define FALSE 0

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
>>> int *processing_complete;
} processing_args_t;

>>> Added new worker_args_t
>>> typedef struct worker_args {
  char *input_directory;
  char *output_directory;
  int thread_id;
  int *worker_complete;
  FILE *fp;
  int requests_handled;
} worker_args_t;
~~
```
### image rotation.c
```
#include "image_rotation.h"
// Change from header linked_list
>>> request_t queue_list = NULL
int queue_length = 0;
pthread_mutex_t processing_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t work_cond = PTHREAD_COND_INITIALIZER;

void log_pretty_print(FILE *to_write, int threadId, int requestNumber,
                      char *file_name) {
  fopen(to_write, "w+);
  char* buffer[BUFF_SIZE];
  sprintf(buffer, "[%d][%d][%s]\n", threadID, requestNumber, fileName);
  fwrite(to_write, buffer, strlen(buffer));
  fprintf(stdout, buffer);
  fclose(to_write);
}

void *processing(void *args) {
  processing_args_t *processing_args = (processing_args_t *)args;
  DIR *dir = opendir(processing_args->input_directory);

  pthread_mutex_lock(&processing_lock);
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
        strcpy(new_image->filename, entry->d_name);
        new_image->rotation_angle = processing_args->rotation_angle;
        new_image->next = NULL;
>>>     if (queue_list == NULL) {
          queue_list.next = new_image;
        } else {
          request_t *temp = queue_list.next;
          while (temp->next != NULL) {
            temp = temp->next;
          }
          temp->next = new_image;
        }
        queue_length++;
>>>     pthread_cond_signal(&work_cond);
      }
    }
  }
  pthread_mutex_unlock(&image_lock);
  pthread_cond_signal(&work_lock);
  closedir(dir);
>>> *(processing_args->processing_complete) = TRUE;  
  return NULL;
}

void *worker(void *args) {

>>> worker_args_t *worker_thread = (worker_args_t *)args;
>>> pthread_mutex_lock(&processsing_lock);
>>> while (queue_length == 0) {
    // Check if Queue is complete
    if (*(worker_thread->worker_complete) == TRUE) {
      pthread_mutex_unlock(&processsing_lock);
      free(worker_thread);
      pthread_exit(NULL);
    }
    pthread_cond_wait(&work_cond, &processsing_lock);
}

// Queue manipulation critical section
>>> char *filename = malloc(strlen(queue_list->filename));
>>> strcpy(filename, queue_list->filename);
>>> int rotation = queue_list->rotation_angle;
>>> request_t *current = queue_list;
>>> request_t *temp = current;
>>> queue_list = queue_list->next;
>>> free(temp);
>>> queue_length--;

// Logging
>>> char input_buffer[BUFF_SIZE];
>>> sprintf(input_buffer, "%s/%s", worker_thread->input_directory, filename);
>>> log_pretty_print(worker_thread->fp, worker_thread->thread_id,
                   worker_thread->requests_handled, input_buffer);

  pthread_mutex_unlock(&processsing_lock);


  int width, height, bbp;
  uint8_t *image_result =
      stbi_load("filename", &width, &height, &bbp, CHANNEL_NUM);

  uint8_t **result_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * width);
  uint8_t **img_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * width);
  for (int i = 0; i < width; i++) {
    result_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
    img_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
  }

  linear_to_image(image_result, img_matrix, width, height);
    uint8_t *img_array = malloc(sizeof(uint8_t) * width * height);
  if (rotation == 270) {
    flip_upside_down(img_matrix, result_matrix, width, height);
  } else {
    flip_left_to_right(img_matrix, result_matrix, width, height);
  }
    flatten_mat(result_matrix, img_array, width, height);
  
  char buffer[BUFF_SIZE];
  sprintf(buffer, "%s/%s", worker_thread->output_directory, filename);
  stbi_write_png(buffer, width, height, CHANNEL_NUM, img_array,
                 width * CHANNEL_NUM);
>>> free(worker_thread);
>>> free(filename);
  return NULL;
}

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
>>> int status = FALSE;
  // Create the threads needed
  pthread_t processing_thread;
>>>   processing_args_t pargs = {input_directory, number_thread rotation_angle, &status};
  pthread_t worker_thread[number_thread];

  pthread_create(&processing_thread, NULL, (void *)processing, &pargs);
  pthread_join(processing_thread, NULL);

>>> int requests = 1;
  for (int i = 0; i < number_thread; i++) {
>>> worker_args_t *wargs = (worker_args_t *)malloc(sizeof(worker_args_t));
    wargs->input_directory = input_directory;
    wargs->output_directory = output_directory;
    wargs->thread_id = i;
    wargs->worker_complete = &status;
    wargs->fp = fp;
    wargs->requests_handled = requests;
    pthread_create(&worker_thread[i], NULL, (void *)worker, wargs);
  }

  // Join on created threads
  for (int i = 0; i < number_thread; i++) {
    pthread_join(worker_thread[i], NULL) != 0);
  }

// Create another set of number_threads for remaining queue
>>> while (queue_length > 0) {
    requests++;
    for (int i = 0; i < number_thread;  //    i++) {
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
      pthread_create(&worker_thread[i], NULL, (void *)worker, wargs);
    }
    for (int i = 0; i < number_thread; i++) {
      pthread_join(worker_thread[i], NULL);
    }
  }

  fclose(fp);
}
```