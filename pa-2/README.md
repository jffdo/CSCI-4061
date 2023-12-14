# Team 010-35-Project2 / Final

## Developed by

Jeffrey Do (do000043@umn.edu)

Micheal Mulhall (mulha022@umn.edu)

Zhihao Lin (lin01046@umn.edu)

## Testing

Tested on lab machine csel-kh1250-05.cselabs.umn.edu

## Indiviual Contribution

Jeffrey Do, root_procces.c, nonleaf_proces.c, ReadMe

Micheal Mulhall, nonleaf_proces.c

Zhihao Lin, leaf_process.c

## Changes to existing files
root_process.c
```
#include <sys/stat.h>
#include <sys/wait.h>
```
Reason: Removing error lines for wait and open("file",WRITE, PERM)

## Assumptions
In root_procces.c
* The pipe file number does not exceed 10 characters
* The number of duplicates from parse_hash for dup and retain list does not execced 1000
* For each index of dup and retain list, the string does not exceed 4096 characters

In nonleaf_proccess.c
* The number of directories/files in a directory does not exceed 100
* The name of directories/files does not exceed 4096 characters
* The pipe file number does not exceed 10 characters
* The hash given by leaf_process does not exceed 4096 characters
* The length of all hashes from all children does not exceed 4096 charcters
## Psuedocode
### leaf_proccess.c
```
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
  // get <file_path> <pipe_write_end> from argv[]
  file_path = (char*)malloc(strlen(argv[1]) + 1);
  strcpy(file_path, argv[1]);
  pipe_write_end = atoi(argv[2]);
  // create the hash of given file
  hash_value[BUFFER_SIZE] = "";
  // printf("%s\n", file_path);
  hash_data_block(hash_value, argv[1]);
  // printf("%s\n", hash_value);
  // construct string write to pipe. The format is
  // "<file_path>|<hash_value>"
  constructed_str =
      (char*)malloc(strlen(file_path) + 1 + SHA256_BLOCK_SIZE * 2 + 1);
  sprintf(constructed_str, "%s|%s|", file_path, hash_value);

  if (pipe_write_end == 0) {
    // (overview): create a file in
    // output_file_folder("output/inter_submission/root*") and write the
    // constructed string to the file
    // (step 1) : extract the file_name from file_path using
    // extract_filename() in utils.c
    file_name = extract_filename(file_path);
    
    // (step 2) : extract the root directory(e.g. root1 or root2 or root3)
    // from file_path using extract_root_directory() in utils.c
    root = extract_root_directory(file_path);
    
    // (step 3) : get the location of the new file (e.g.
    // "output/inter_submission/root1" or "output/inter_submission/root2" or
    // "output/inter_submission/root3")
    location =
        (char*)malloc(strlen(output_file_folder) + strlen(root) + 1);
        
    strcpy(location, output_file_folder);
    strcat(location, root);
    
    // (step 4) : create and write to file, and then close file
    //  create a full path
    full_path =
        (char*)malloc(strlen(location) + 1 + strlen(file_name) + 1);
    sprintf(full_path, "%s/%s", location, file_name);
    
    file = fopen(full_path, "w");
    fprintf(file, "%s", constructed_str);
    fclose(file);

    // (step 5) : free any arrays that are allocated using malloc!! Free the
    // string returned from extract_root_directory()!! It is allocated using
    // malloc in extract_root_directory()
    free(file_path);
    free(constructed_str);
    free(root);
    free(location);
    free(full_path);
  } else {
    // TODO(final submission): write the string to pipe
    close(pipe_read_end);
    open(pipe_write_end);
    write(pipe_write_end, constructed_str, strlen(constructed_str));
    close(pipe_write_end);
    exit(0);
  }

  exit(0);
}

```
### nonleaf_prccess.c
```
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./nonleaf_process <directory_path> <pipe_write_end> \n");
        return 1;
    }
    
    //TODO(step1): get <file_path> <pipe_write_end> from argv[]
    file_path = argv[1];
    pipe_write_end = argv[2];

    //TODO(step2): malloc buffer for gathering all data transferred from child process as in root_process.c
    buffer = (char*) malloc(4098);

    //TODO(step3): open directory
    DIR dir = opendir(file_path);

    //TODO(step4): traverse directory and fork child process
    // Hints: Maintain an array to keep track of each read end pipe of child process
    array[sizeof(file_path)];
    fd[2];
    pipe(fd);
    TEMP_STDOUT_FILENO = dup(STDOUT_FILENO);
    PIPE_WRITE_FILENO = dup2(fd[1], STDOUT_FILENO);
    dup2(TEMP_STDOUT_FILENO, STDOUT_FILENO);

    pipe_number[10];
    sprintf(pipe_number, "%d", PIPE_WRITE_FILENO);
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
      char *name = (char *)malloc(strlen(file_path) + sizeof(entry->d_name) + 1);
      sprintf(name, "%s/%s", file_path, entry->d_name);

      pid_t pid = fork();
      if (pid == 0) {
        if ((strcmp(entry->d_name, ".") == 0) ||
          (strcmp(entry->d_name, "..") == 0)) {
        } else if (entry->d_type == DT_REG) {
        printf("REG found %s\n", entry->d_name);
        execl("./leaf_process", "./leaf_process", name, pipe_number, NULL);
        printf("failed\n");
      } else if (entry->d_type == DT_DIR) {
        printf("DIR found %s\n", entry->d_name);
        execl("./nonleaf_process", "./nonleaf_process", name, pipe_number,
              NULL);
        printf("failed\n");
      }
    } else {
          waitpid(pid, NULL, 0);
    }
    //TODO(step5): read from pipe constructed for child process and write to pipe constructed for parent process
    ??
    closedir(dir);
    close(TEMP_STDOUT_FILENO);
    free(name);
    free(buffer);
}
```
### root_proccess.c
```
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/utils.h"

#define WRITE (O_WRONLY | O_CREAT | O_TRUNC)
#define PERM (S_IRUSR | S_IWUSR)
char *output_file_folder = "output/final_submission/";

void redirection(char **dup_list, int size, char* root_dir){
    // (overview) : redirect standard output to an output file in output_file_folder("output/final_submission/")
    // (step1) : determine the filename based on root_dir. e.g. if root_dir is "./root_directories/root1", the output file's name should be "root1.txt"
    root_dir = extract_root_directory(root_dir);
    strcat(root_dir, ".txt");
    // (step2) : redirect standard output to output file (output/final_submission/root*.txt)
    char buffer[strlen(ouput_file_folder) + strlen(root_dir) + 1]
    sprintf(buffer, ouput_file_folder, root_dir);
    
    int output = open(bufer, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    int TEMP_STDOUT_FILENO = dup(STDOUT_FILENO);
    dup2(output, STDOUT_FILENO);
    // (step3) : read the content each symbolic link in dup_list, write the path as well as the content of symbolic link to output file(as shown in expected)
    for (i = 0, i < size; i++){
      printf("[<path of symbolic link> --> <path of retained file>] : [%s --> %s]", dup_list[i], retain_list[i]);
    }
    dup2(TEMP_STDOUT_FILENO, STDOUT_FILENO);
    close(TEMP_STDOUT_FILENO);
}

void create_symlinks(char **dup_list, char **retain_list, int size) {
    // create symbolic link at the location of deleted duplicate file
    // dup_list[i] will be the symbolic link for retain_list[i]
    for(i = 0; i < size; i++){
      symlink(retain_list[i], dup_list[i]);
    }

}

void delete_duplicate_files(char **dup_list, int size) {
    // delete duplicate files, each element in dup_list is the path of the duplicate file
    for (i = 0; i < size; i++){
      remove(dup_list);
    }
}

// ./root_directories <directory>
int main(int argc, char* argv[]) {
    if (argc != 2) {
        // dir is the root_directories directory to start with
        // e.g. ./root_directories/root1
        printf("Usage: ./root <dir> \n");
        return 1;
    }

    // (overview): fork the first non_leaf process associated with root directory("./root_directories/root*")

    char* root_directory = argv[1];
    char all_filepath_hashvalue[4098]; //buffer for gathering all data transferred from child process
    memset(all_filepath_hashvalue, 0, sizeof(all_filepath_hashvalue));// clean the buffer

    //(step1)
    int fd[2];
    pipe(fd);

    // (step2): fork() child process & read data from pipe to all_filepath_hashvalue
    
    pid_t pid = fork();
    
    if (pid == 0){
      execl("./nonleaf_process.c, root_directory, fd[1]);
    } else {
      wait(NULL);
      close(fd[1]);
      read(fd[0], all_filepath_hashvalue, sizeof(all_filepath_hashvalue);
      close[fd[0]);
    }


    // (step3): malloc dup_list and retain list & use parse_hash() in utils.c to parse all_filepath_hashvalue
    dup_list = (char**) malloc (sizeof(char*) * size);
    retain_list = (char**) malloc (sizeof(char*) * size);
    parse_hash(all_filepath_hashvalue, dup_list, retain_list)

    // (step4): implement the functions
    delete_duplicate_files(dup_list,size);
    create_symlinks(dup_list, retain_list, size);
    redirection(dup_list, size, argv[1]);

    // (step5): free any arrays that are allocated using malloc!!
    for (i = 0; i < size; i++){
      free(dup_list[i]);
      free(retain_list[i]);
    }
    
    duplist = NULL;
    retainlist = NULL;

}

```
## Changes in final submission
```
">>>" To indicate changes/additonal notes, and without psuedocode
```
### root_process.c
```
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
>>> #include <sys/stat.h>
>>> #include <sys/wait.h>
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
>>> Replaced extract_root_directory with strchr  
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
>>> Some confusion on how dup is handled
>>> Current interation does not restore stdout
>>> Adding int TEMP_STDOUT = dup(STDOUT_FILENO) before output is in dup2
>>> and adding dup2(TEMP_STDOUT, STDOUT_FILENO) and close(TEMP_STDOUT)
>>> after restores stdout, but does not in Gradescope.
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
>>> Changed free dup_list[i] and retain_list[i] loop;
  free(dup_list);
  free(retain_list);
  exit(0);
}

```
### nonleaf_process.c
```
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
>>> Array is moved here and index of the most recent hash added in tracked
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
>>> Add the case if entry was ./ or ../
      waitpid(pid, NULL, 0);
      if ((strcmp(entry->d_name, ".") == 0) ||
          (strcmp(entry->d_name, "..") == 0)) {
      } else {
>>> Added hashes to array in parent processs
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
>>> Combined hashes together with strcat
  char hashes[4096] = "\0";
  for (int i = 0; i < index; i++) {
    strcat(hashes, array[i]);
  }
>>> Write to parent pipe
  write(pipe_write_end, hashes, strlen(hashes));
  // printf("Hashes: %s\n", hashes);
>>> Free array of hashes
  for (int i = 0; i < index; i++) {
    free(array[i]);
  }
  exit(0);
}

```
