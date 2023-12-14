#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/hash.h"
#include "../include/utils.h"

char* output_file_folder = "output/inter_submission/";

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage: Inter Submission --> ./leaf_process <file_path> 0\n");
    printf(
        "Usage: Final Submission --> ./leaf_process <file_path> "
        "<pipe_write_end>\n");
    return -1;
  }
  // TODO(): get <file_path> <pipe_write_end> from argv[]
  char* file_path = (char*)malloc(strlen(argv[1]) + 1);
  strcpy(file_path, argv[1]);
  int pipe_write_end = atoi(argv[2]);

  // TODO(): create the hash of given file
  char hash_value[BUFFER_SIZE] = "";
  hash_data_block(hash_value, argv[1]);

  // TODO(): construct string write to pipe. The format is
  // "<file_path>|<hash_value>"
  char* constructed_str =
      (char*)malloc(strlen(file_path) + 1 + SHA256_BLOCK_SIZE * 2 + 1);
  sprintf(constructed_str, "%s|%s|", file_path, hash_value);

  if (pipe_write_end == 0) {
    // TODO(inter submission)
    // TODO(overview): create a file in
    // output_file_folder("output/inter_submission/root*") and write the
    // constructed string to the file
    // TODO(step1): extract the file_name from file_path using
    // extract_filename() in utils.c
    char* file_name = extract_filename(file_path);
    // TODO(step2): extract the root directory(e.g. root1 or root2 or root3)
    // from file_path using extract_root_directory() in utils.c
    char* root = extract_root_directory(file_path);
    // TODO(step3): get the location of the new file (e.g.
    // "output/inter_submission/root1" or "output/inter_submission/root2" or
    // "output/inter_submission/root3")
    char* location =
        (char*)malloc(strlen(output_file_folder) + strlen(root) + 1);
    strcpy(location, output_file_folder);
    strcat(location, root);
    // TODO(step4): create and write to file, and then close file
    //  create a full path
    char* full_path =
        (char*)malloc(strlen(location) + 1 + strlen(file_name) + 1);
    sprintf(full_path, "%s/%s", location, file_name);
    // create new file
    FILE* file = fopen(full_path, "w");
    if (file == NULL) {
      perror("Inter subimission: Unable to open file in leaf");
      exit(-1);
    }
    // write to the new file
    fprintf(file, "%s", constructed_str);
    fclose(file);
    // TODO(step5): free any arrays that are allocated using malloc!! Free the
    // string returned from extract_root_directory()!! It is allocated using
    // malloc in extract_root_directory()
    free(root);
    free(location);
    free(full_path);
  } else {
    // TODO(final submission): write the string to pipe
    if (write(pipe_write_end, constructed_str, strlen(constructed_str)) == -1) {
      perror("Final submission: Unable to write to pipe in leaf");
    }
  }
  free(file_path);
  free(constructed_str);
  exit(0);
}
