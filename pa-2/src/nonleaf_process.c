#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: ./nonleaf_process <directory_path> <pipe_write_end> \n");
    return 1;
  }
  // TODO(overview): fork the child processes(non-leaf process or leaf process)
  // each associated with items under <directory_path>

  // TODO(step1): get <file_path> <pipe_write_end> from argv[]
  char *file_path = argv[1];
  int pipe_write_end = atoi(argv[2]);

  // TODO(step2): malloc buffer for gathering all data transferred from child
  // process as in root_process.c

  // TODO(step3): open directory
  DIR *dir = opendir(file_path);
  if (dir == NULL) {
    perror("Unable to opendir in nonleaf");
    exit(-1);
  }

  // TODO(step4): traverse directory and fork child process
  //  Hints: Maintain an array to keep track of each read end pipe of child
  //  process
  int fd[2];
  if (pipe(fd) < 0) {
    perror("Unable to create pipe inf nonleaf");
    exit(-1);
  }
  int TEMP_STDOUT_FILENO = dup(STDOUT_FILENO);
  int PIPE_WRITE_FILENO = dup2(fd[1], TEMP_STDOUT_FILENO);
  if (TEMP_STDOUT_FILENO < 0 || PIPE_WRITE_FILENO < 0) {
    perror("Dup failed in nonleaf");
    exit(-1);
  }

  char pipe_number[10];
  sprintf(pipe_number, "%d", PIPE_WRITE_FILENO);
  // printf("%s pipe number \n", pipe_number);

  char *array[100];
  int index = 0;

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    char name[4096];
    sprintf(name, "%s/%s", file_path, entry->d_name);

    pid_t pid = fork();
    if (pid == 0) {
      if ((strcmp(entry->d_name, ".") == 0) ||
          (strcmp(entry->d_name, "..") == 0)) {
        exit(0);
      } else if (entry->d_type == DT_REG) {
        // printf("REG found %s\n", entry->d_name);
        if (closedir(dir) < 0) {
          perror("Closedir failed in nonleaf/REG");
          exit(-1);
        }
        if (execl("./leaf_process", "./leaf_process", name, pipe_number, NULL) <
            0) {
          perror("Execl failed in nonleaf/REG");
          exit(-1);
        }
        // printf("failed\n");
      } else if (entry->d_type == DT_DIR) {
        // printf("DIR found %s\n", entry->d_name);
        if (closedir(dir) < 0) {
          perror("Closedir failed in nonleaf/DIR");
          exit(-1);
        }
        if (execl("./nonleaf_process", "./nonleaf_process", name, pipe_number,
                  NULL) < 0) {
          perror("Execl failed in nonleaf/DIR");
          exit(-1);
        }
        // printf("failed\n");
      }
    } else {
      // TODO(step5): read from pipe constructed for child process and write to
      // pipe constructed for parent process
      waitpid(pid, NULL, 0);
      if ((strcmp(entry->d_name, ".") == 0) ||
          (strcmp(entry->d_name, "..") == 0)) {
      } else {
        write(PIPE_WRITE_FILENO, "", 1);
        char *buffer = malloc(4096);
        memset(buffer, 0, 4096);
        read(fd[0], buffer, 4096);
        // printf("%s %d\n", buffer, index);
        array[index] = buffer;
        index++;
        // for (int i = 0; i < index; i++) {
        //   printf("Current state of Array %d %s\n", i, array[i]);
        // }
      }
    }
  }
  if (closedir(dir) < 0) {
    perror("Closedir failed in nonleaf");
    exit(-1);
  }
  char hashes[4096] = "\0";
  for (int i = 0; i < index; i++) {
    strcat(hashes, array[i]);
  }
  write(pipe_write_end, hashes, strlen(hashes));
  // printf("Hashes: %s\n", hashes);

  for (int i = 0; i < index; i++) {
    free(array[i]);
  }
  exit(0);
}
