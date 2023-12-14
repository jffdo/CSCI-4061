#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "hash.h"

#define PATH_MAX 1024

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf(
        "Usage: ./child_process <blocks_folder> <hashes_folder> <N> "
        "<child_id>\n");
    return 1;
  }
  char *blocks_folder = argv[1];
  char *hashes_folder = argv[2];
  int n = atoi(argv[3]);
  int child_id = atoi(argv[4]);
  // TODO: If the current process is a leaf process, read in the associated
  // block file and compute the hash of the block.
  if (child_id >= n - 1) {
    char hash_txt[PATH_MAX];
    sprintf(hash_txt, "%s/%d.txt", blocks_folder, child_id - (n - 1));
    char block_hash[SHA256_BLOCK_SIZE * 2 + 1];
    hash_data_block(block_hash, hash_txt);
    char hash_o[PATH_MAX];
    sprintf(hash_o, "%s/%d.out", hashes_folder, child_id);
    FILE *fp = fopen(hash_o, "w");

    if (fp == NULL) {
      fprintf(stderr, "Error: Unable to open file %s\n", hash_o);
      exit(-1);
    }

    fwrite(block_hash, sizeof(block_hash), 1, fp);
    fclose(fp);
    exit(0);
  } else {
    // TODO: If the current process is not a leaf process, spawn two child
    // processes using exec() and ./child_process.
    pid_t left_pid = fork();

    if (left_pid < 0) {
      fprintf(stderr, "Error: Failed to create left child in child_proccess\n");
      exit(-1);
    } else if (left_pid == 0) {
      char left_id[20];
      sprintf(left_id, "%d", 2 * (child_id) + 1);
      if (execl("./child_process", "./child_process", blocks_folder,
                hashes_folder, argv[3], left_id, NULL) < 0) {
        fprintf(stderr,
                "Error: execl failed for left child in child_process\n");
        exit(-1);
      }
    } else {
      waitpid(left_pid, NULL, 0);  // wait for left child
      // Create right child
      pid_t right_pid = fork();
      if (right_pid < 0) {
        fprintf(stderr,
                "Error: Failed to create right child in child_process\n");
        exit(-1);
      } else if (right_pid == 0) {
        char right_id[20];
        sprintf(right_id, "%d", 2 * (child_id) + 2);
        if (execl("./child_process", "./child_process", blocks_folder,
                  hashes_folder, argv[3], right_id, NULL) < 0) {
          fprintf(stderr,
                  "Error: execl failed for right child in child_process\n");
          exit(-1);
        }
      }
      // TODO: Wait for the two child processes to finish
      else {  // wait for right child
        waitpid(right_pid, NULL, 0);
      }
    }
    // TODO: Retrieve the two hashes from the two child processes from
    // output/hashes/ and compute and output the hash of the concatenation of
    // the two hashes.
    char filename[PATH_MAX];
    char left_filename[PATH_MAX];
    char right_filename[PATH_MAX];
    sprintf(filename, "%s/%d.out", hashes_folder, child_id);
    sprintf(left_filename, "%s/%d.out", hashes_folder, 2 * (child_id) + 1);
    sprintf(right_filename, "%s/%d.out", hashes_folder, 2 * (child_id) + 2);

    char left_hash[SHA256_BLOCK_SIZE * 2 + 1];
    char right_hash[SHA256_BLOCK_SIZE * 2 + 1];
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
      fprintf(stderr, "Error: Unable to open file %s\n", filename);
      exit(-1);
    }

    FILE *left_fp = fopen(left_filename, "r");  // read left hash
    if (left_fp == NULL) {
      fprintf(stderr, "Error: Unable to open file %s\n", left_filename);
      exit(-1);
    }

    fread(left_hash, sizeof(left_hash), 1, left_fp);
    fclose(left_fp);

    FILE *right_fp = fopen(right_filename, "r");  // read right hash
    if (right_fp == NULL) {
      fprintf(stderr, "Error: Unable to open file %s\n", right_filename);
      exit(-1);
    }

    fread(right_hash, sizeof(right_hash), 1, right_fp);
    fclose(right_fp);

    char hash[SHA256_BLOCK_SIZE * 2 + 1];
    compute_dual_hash(hash, left_hash, right_hash);
    fwrite(hash, sizeof(hash), 1, fp);

    fclose(fp);
  }
}
