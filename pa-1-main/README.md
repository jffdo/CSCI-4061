# Team 010-35-Project1 / Intermediate

## Developed by

Jeffrey Do (do000043@umn.edu)

Micheal Mulhall (mulha022@umn.edu)

Zhihao Lin (lin01046@umn.edu)

## Testing

Tested on lab machine csel-kh1250-05.cselabs.umn.edu

## Indiviual Contribution

Jeffrey Do, merkle.c, child_process.c, partition_file_data, github repository, README.md

Micheal Mulhall, README.md, child_process.c

Zhihao Lin, merkle.c, README.md

## Additional assumptions
Assumes the id number of the hash files created are less than 20 characters long

## Planned implementation of Merkle Tree

### merkle.c

```

#include "utils.h"
#include "print_tree.h"

// ##### DO NOT MODIFY THESE VARIABLES #####
char *blocks_folder = "output/blocks";
char *hashes_folder = "output/hashes";
char *visualization_file = "output/visualization.txt";


int main(int argc, char* argv[]) {
    if (argc != 3) {
        // The three arguments that must be taken in are the ./merkle, the input file, and the number of data blocks(N) to split the input file into
        // N is the number of data blocks to split <file> into (should be a power of 2)
        // N will also be the number of leaf nodes in the merkle tree
        printf("Usage: ./merkle <file> <N>\n");
        return 1;
    }

    // TODO: Read in the command line arguments and validate them
    char *input_file = argv[1];

    int n = atoi(argv[2]);
    if ((n & (n - 1)) != 0) {
        printf("<N> needs to be a power of 2\n");
        return 1;
    }


    // ##### DO NOT REMOVE #####
    setup_output_directory(blocks_folder, hashes_folder);

    // TODO: Implement this function in utils.c
    partition_file_data(input_file, n, blocks_folder);


    // TODO: Start the recursive merkle tree computation by spawning first child process (root)
    pid_t child_pid = fork();
    if child: exec(child_process);
    if parent: wait for the child process;

    // ##### DO NOT REMOVE #####
    #ifndef TEST_INTERMEDIATE
        // Visually display the merkle tree using the output in the hashes_folder
        print_merkle_tree(visualization_file, hashes_folder, n);
    #endif

    return 0;
}

```

### child_process.c

```

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "hash.h"

#define PATH_MAX 1024

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: ./child_process <blocks_folder> <hashes_folder> <N> <child_id>\n");
        return 1;
    }

    // Use the child_id to detemine if it is a leaf process
    if (child_id >= N - 1) {
        this is a leaf process;
    }
    else {
        this is a non leaf process;
    }

    // TODO: If the current process is a leaf process, read in the associated block file
    // and compute the hash of the block.

    // Compute the hash use void hash_data_block(char *result_hash, char *block_filename);
    hash_data_block(result_hash, blocks_folder);
    // Write to output/hashes/child_id.out
    write("output/hashes/child_id", result_hash, size);


    // TODO: If the current process is not a leaf process, spawn two child processes using
    // exec() and ./child_process.

    // Create left child
    pid_t left_child_pid = fork();

    if (left_child_pid != 0) {
        // Create right child
        pid_t right_child_pid = fork();

        if (right_child_pid == 0) {
            exec(./child_process, param1, param2, ...., 2 * child_id + 1)
        }
    }
    else {
        exec(./child_process, param1, param2, ...., 2 * child_id + 2)
    }


    // TODO: Wait for the two child processes to finish
    waitpid(first_child_pid);
    waitpid(second_child_pid);
    // TODO: Retrieve the two hashes from the two child processes from output/hashes/
    read("output/hashes/left_child_id.out", buffer1, size);
    read("output/hashes/right_child_id.out", buffer2, size);
    // and compute and output the hash of the concatenation of the two hashes.
    compute_dual_hash(result_hash, buffer1, buffer2);
    write("output/hashes/child_id", result_hash, size);
}


```
## Changes in final implementation
### child_process.c
```

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "hash.h"

#define PATH_MAX 1024

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: ./child_process <blocks_folder> <hashes_folder> <N> <child_id>\n");
        return 1;
    }

    if (child_id >= N - 1) {
        hash_data_block(buffer, blocks_folder/[child_id - (n-1].txt);
        fopen(hashes_folder/[child_id].out, "w")
        fwrite(buffer);
        fclose()
        exit(0)
    } else {
        leftpid = fork();
        if (leftpid == 0){
            exec(./child_process, param1, param2, ...., 2 * child_id + 1);
        } else {
            waitpid(leftpid);
            rightpid = fork();
            if (rightpid == 0) {
                exec(./child_process, param1, param2, ...., 2 * child_id + 2);
            } else {
                waitpid(rightpid);
            }
            
    fopen(hashes_folder/[child_id].out); // hash for child_id
    
    fopen(hashes_folder/[2 * child_id +1].out); //read left hash
    fread(buffer1, lefthash)
    fclose(lefthash)
    
    fopen(hashes_folder/[2 * child_id +2].out); //read right hash
    fread(buffer1, righthash)
    fclose(righthash)
    
    compute_dual_hash(result, buffer1, buffer2);
    fwrite(result, hash);
    fclose(hash)
    }

}


```
The changes between the intermediate and final for child_proccess is how the if statements were structured. Moved the leaf and nonleaf functions to handle thier respective cases, instead of directly after the if statement.
