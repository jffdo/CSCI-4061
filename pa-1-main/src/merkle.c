#include "print_tree.h"
#include "utils.h"

// ##### DO NOT MODIFY THESE VARIABLES #####
char *blocks_folder = "output/blocks";
char *hashes_folder = "output/hashes";
char *visualization_file = "output/visualization.txt";

int main(int argc, char *argv[]) {
  if (argc != 3) {
    // N is the number of data blocks to split <file> into (should be a power of
    // 2) N will also be the number of leaf nodes in the merkle tree
    printf("Usage: ./merkle <file> <N>\n");
    return 1;
  }

  // TODO: Read in the command line arguments and validate them
  char *input_file = argv[1];

  int n = atoi(argv[2]);
  if ((n & (n - 1)) != 0) {
    fprintf(stderr, "Error: <N> needs to be a power of 2\n");
    return -1;
  }

  // ##### DO NOT REMOVE #####
  setup_output_directory(blocks_folder, hashes_folder);

  // TODO: Implement this function in utils.c
  partition_file_data(input_file, n, blocks_folder);

  // TODO: Start the recursive merkle tree computation by spawning first child
  // process (root)
  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Error: Failed to create child proccess in merkle\n");
    return -1;
  } else if (pid == 0) {
    if (execl("./child_process", "./child_procces", blocks_folder,
              hashes_folder, argv[2], "0", NULL) < 0) {
      fprintf(stderr, "Error: execl failed\n");
      return -1;
    };
  } else {
    waitpid(pid, NULL, 0);
  }

// ##### DO NOT REMOVE #####
#ifndef TEST_INTERMEDIATE
  // Visually display the merkle tree using the output in the hashes_folder
  print_merkle_tree(visualization_file, hashes_folder, n);
#endif

  return 0;
}