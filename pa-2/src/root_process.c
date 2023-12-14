#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/utils.h"

#define WRITE (O_WRONLY | O_CREAT | O_TRUNC)
#define PERM (S_IRUSR | S_IWUSR)
char *output_file_folder = "output/final_submission/";

void redirection(char **dup_list, int size, char *root_dir) {
  // TODO(overview): redirect standard output to an output file in
  // output_file_folder("output/final_submission/")
  // TODO(step1): determine the filename based on root_dir. e.g. if root_dir is
  // "./root_directories/root1", the output file's name should be "root1.txt"
  char root[strlen(root_dir) + 5];
  strcpy(root, root_dir);
  strcpy(root, strrchr(root, '/') + 1);  // move past '/'
  strcat(root, ".txt");
  // TODO(step2): redirect standard output to output file
  // (output/final_submission/root*.txt)
  char buffer[strlen(output_file_folder) + strlen(root) + 1];
  sprintf(buffer, "%s%s", output_file_folder, root);
  // printf("%s\n", buffer);

  int output = open(buffer, WRITE, PERM);
  if (output == -1) {
    perror("Unable to open buffer");
    return;
  }
  if (dup2(output, STDOUT_FILENO) < 0) {
    perror("Dup failed in redirection");
    return;
  }

  // TODO(step3): read the content each symbolic link in dup_list, write the
  // path as well as the content of symbolic link to output file(as shown in
  // expected)
  char retain[4096] = "\0";

  for (int i = 0; i < size; i++) {
    memset(retain, 0, 4096);
    if (readlink(dup_list[i], retain, 4096) < 0) {
      perror("Readlink failed in redirection");
      return;
    }
    printf(
        "[<path of symbolic link> --> <path of retained file>] : [%s --> %s]\n",
        dup_list[i], retain);
  }
  close(output);
}

void create_symlinks(char **dup_list, char **retain_list, int size) {
  // TODO(): create symbolic link at the location of deleted duplicate file
  // TODO(): dup_list[i] will be the symbolic link for retain_list[i]
  for (int i = 0; i < size; i++) {
    if (symlink(retain_list[i], dup_list[i]) < 0) {
      perror("Create symlink failed");
      return;
    }
  }
}

void delete_duplicate_files(char **dup_list, int size) {
  // TODO(): delete duplicate files, each element in dup_list is the path of the
  // duplicate file
  for (int i = 0; i < size; i++) {
    if (remove(dup_list[i]) < 0) {
      perror("Remove failed");
      return;
    }
  }
}

// ./root_directories <directory>
int main(int argc, char *argv[]) {
  if (argc != 2) {
    // dir is the root_directories directory to start with
    // e.g. ./root_directories/root1
    printf("Usage: ./root <dir> \n");
    return 1;
  }

  // TODO(overview): fork the first non_leaf process associated with root
  // directory("./root_directories/root*")

  char *root_directory = argv[1];
  char all_filepath_hashvalue[4098];  // buffer for gathering all data
                                      // transferred from child process
  memset(all_filepath_hashvalue, 0,
         sizeof(all_filepath_hashvalue));  // clean the buffer

  // TODO(step1): construct pipe
  int fd[2];
  if (pipe(fd) < 0) {
    perror("Pipe failed");
    exit(-1);
  }
  // TODO(step2): fork() child process & read data from pipe to
  // all_filepath_hashvalue
  int TEMP_STDOUT_FILENO = dup(STDOUT_FILENO);
  int PIPE_WRITE_FILENO = dup2(fd[1], TEMP_STDOUT_FILENO);
  if (TEMP_STDOUT_FILENO < 0 || PIPE_WRITE_FILENO < 0) {
    perror("Dup failed in root");
    exit(-1);
  }
  // printf("%d, %d,\n", TEMP_STDOUT_FILENO, PIPE_WRITE_FILENO);
  char pipe_number[10];
  sprintf(pipe_number, "%d", PIPE_WRITE_FILENO);

  pid_t pid = fork();
  if (pid < 0) {
    perror("Fork failed in root");
    exit(-1);
  }
  if (pid == 0) {
    if (execl("./nonleaf_process", "./nonleaf_process", root_directory,
              pipe_number, NULL) < 0) {
      perror("Execl failed in root");
      exit(-1);
    }
    // printf("failed\n");
  } else {
    // printf("reach\n");
    waitpid(pid, NULL, 0);
    // printf("waited\n");
    write(PIPE_WRITE_FILENO, "", 1);
    read(fd[0], all_filepath_hashvalue, sizeof(all_filepath_hashvalue));
  }
  if (close(PIPE_WRITE_FILENO) < 0) {
    perror("Unable to close in root");
    exit(-1);
  };

  // TODO(step3): malloc dup_list and retain list & use parse_hash() in utils.c
  // to parse all_filepath_hashvalue
  //  dup_list: list of paths of duplicate files. We need to delete the files
  //  and create symbolic links at the location retain_list: list of paths of
  //  unique files. We will create symbolic links for those files
  // printf("!root received!%s\n", all_filepath_hashvalue);
  char **dup_list = malloc(1000);
  memset(dup_list, 0, 1000);
  char **retain_list = malloc(1000);
  memset(retain_list, 0, 1000);
  if (dup_list == NULL || retain_list == NULL) {
    perror("Malloc failed in root");
    exit(-1);
  }
  int size = parse_hash(all_filepath_hashvalue, dup_list, retain_list);
  // TODO(step4): implement the functions
  delete_duplicate_files(dup_list, size);
  create_symlinks(dup_list, retain_list, size);
  redirection(dup_list, size, argv[1]);
  // TODO(step5): free any arrays that are allocated using malloc!!
  free(dup_list);
  free(retain_list);
  exit(0);
}
