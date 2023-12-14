#include "utils.h"

#include <math.h>
// Create N files and distribute the data from the input file evenly among them
// See section 3.1 of the project writeup for important implementation details
void partition_file_data(char *input_file, int n, char *blocks_folder) {
  // Hint: Use fseek() and ftell() to determine the size of the file
  FILE *fp = fopen(input_file, "r");

  if (fp == NULL) {
    fprintf(stderr, "Error: Unable to open file %s\n", input_file);
    exit(-1);
  }

  fseek(fp, 0, SEEK_END);  // Seek to EOF to find size
  int size = ftell(fp);    // Return to beginning
  fseek(fp, 0, SEEK_SET);

  int block_size = floor(size) / n;                       // for 0.. (N-2)
  int final_block_size = (floor(size) / n) + (size % n);  // (N-1)

  char *buffer = (char *)malloc(block_size * sizeof(char) + 1);

  if (buffer == NULL) {
    fprintf(stderr, "Error: Unable to allocate to \"buffer\"\n");
    exit(-1);
  }

  buffer[0] = '\0';
  char *final_buffer = (char *)malloc(final_block_size * sizeof(char) + 1);
  final_buffer[0] = '\0';
  char filename[PATH_MAX] = "";

  if (final_buffer == NULL) {
    fprintf(stderr, "Error: Unable to malloc to \"fianl_buffer\"\n ");
    exit(-1);
  }

  // Read and write block size into blocks folder
  for (int i = 0; i <= n - 2; i++) {
    fread(buffer, block_size, 1, fp);  // reads block size into buffer
    sprintf(filename, "%s/%d.txt", blocks_folder,
            i);  // naming scheme for block text files
    FILE *fp_block = fopen(filename, "w+");

    if (fp_block == NULL) {
      fprintf(stderr, "Error: Unable to open file %s\n", filename);
      exit(-1);
    }

    fprintf(fp_block, "%s", buffer);
    fclose(fp_block);
  }

  // Read and write for final block before EOF
  fread(final_buffer, final_block_size, 1, fp);  // reads final_block
  sprintf(filename, "%s/%d.txt", blocks_folder,
          n - 1);  // naming scheme for final block
  FILE *fp_final_block = fopen(filename, "w+");

  if (fp_final_block == NULL) {
    fprintf(stderr, "Error: Unable to open file %s\n", filename);
    exit(-1);
  }

  fprintf(fp_final_block, "%s", final_buffer);
  fclose(fp_final_block);

  fclose(fp);

  free(buffer);
  free(final_buffer);
}

// ##### DO NOT MODIFY THIS FUNCTION #####
void setup_output_directory(char *block_folder, char *hash_folder) {
  // Remove output directory
  pid_t pid = fork();
  if (pid == 0) {
    char *argv[] = {"rm", "-rf", "./output/", NULL};
    if (execvp(*argv, argv) < 0) {
      printf("ERROR: exec failed\n");
      exit(1);
    }
    exit(0);
  } else {
    wait(NULL);
  }

  sleep(1);

  // Creating output directory
  if (mkdir("output", 0777) < 0) {
    printf("ERROR: mkdir output failed\n");
    exit(1);
  }

  sleep(1);

  // Creating blocks directory
  if (mkdir(block_folder, 0777) < 0) {
    printf("ERROR: mkdir output/blocks failed\n");
    exit(1);
  }

  // Creating hashes directory
  if (mkdir(hash_folder, 0777) < 0) {
    printf("ERROR: mkdir output/hashes failed\n");
    exit(1);
  }
}
