# Project 4 / Final

## Developed by
Jeffrey Do (do000043@umn.edu)

## Testing
Tested on lab machine csel-kh1250-05.cselabs.umn.edu

## Indiviual Contribution
Jeffrey Do, client.c, server.c

## Psuedocode
### Additional notes
```
'>>>' - Is for additional notes/ changes from Intermediate

Removed print statements in psuedocode for convenience.

The server will send ACK packet instead of simultaneously with the processed image. I found that the client may have started sending images before server can recieve. The packet sent by the server will notify the client to start sending, if it was ACK.
```
### Showing changes from Intermediate submission
### client.h
```
// Only showing changes
typedef struct request_queue {
  int rotation_angle;
  char *file_name;
  // Added next
  struct request_queue *next;
} request_t;
```
### server.c
```
#include "server.h"

#define PORT 1043  //>>> do000043, changed to 1043 to fit in range 1023-65536
#define MAX_CLIENTS 5
#define BUFFER_SIZE 4024

void *clientHandler(void *socket) {
  // Receive packets from the client
  int *client_fd = (int *)socket;
>>> Added while loop for multiple sends from a single client  
  while (1) {
    char recvdata[BUFF_SIZE];
    memset(recvdata, 0, BUFF_SIZE);
    int ret = recv(*client_fd, recvdata, BUFF_SIZE, 0);
    if (ret < 0) {
      perror("Receive failed in server");
      exit(-1);
    }

    packet_t *recvpacket = (packet_t *)malloc(sizeof(packet_t));
    memset(recvpacket, 0, sizeof(packet_t));
    memcpy(recvpacket, recvdata, sizeof(packet_t));
>>> Added check for IMG_OP_EXIT to break while loop    
    if (recvpacket->operation == IMG_OP_EXIT) {
      free(recvpacket);
      close(*client_fd);
      break;
    }
    // Determine the packet operatation and flags
    int image_size = ntohs(recvpacket->size);
    int current = 0;

    char buffer[BUFF_SIZE];
    memset(buffer, 0, BUFF_SIZE);
    // Receive the image data using the size

    unsigned char *image = malloc(image_size);
    memset(image, 0, image_size);

    //  Acknowledge the request and return the processed image data
>>> The server sends ACK to sync with client   
    packet_t ack_packet;
    if (recvpacket->operation == IMG_OP_ROTATE) {
      ack_packet.operation = IMG_OP_ACK;
    } else {
      ack_packet.operation = IMG_OP_NAK;
    }
>>> Server will send ACK if it recieved and IMG_OP_ROTATE operation    
    int PACKET_SIZE = sizeof(packet_t);
    char *serialized_data = (char *)malloc(sizeof(char) * PACKET_SIZE);
    memset(serialized_data, 0, PACKET_SIZE);
    memcpy(serialized_data, &ack_packet, PACKET_SIZE);
    int ret2 = send(*client_fd, serialized_data, sizeof(ack_packet), 0);
    if (ret2 < 0) {
      perror("Send failed in server");
      exit(-1);
    }
    free(serialized_data);

>>> This code will continue receiving until the number of bytes received total are the size of image. I did not notice until later that BUFF_SIZE was larger than the image, but still kept it.  
    int image_ret;
    while (1) {
      image_ret = recv(*client_fd, buffer, BUFF_SIZE, 0);
      if (image_ret <= 0) {
        break;
      }
      memcpy(image + current, buffer, image_ret);
      current += image_ret;

      if (current >= image_size) {
        break;
      }
    }
    //   Process the image data based on the set of flags
    int width, height, bbp;
    uint8_t *image_result = stbi_load_from_memory(image, image_size, &width,
                                                  &height, &bbp, CHANNEL_NUM);
    uint8_t **result_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * width);
    uint8_t **img_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * width);

    for (int i = 0; i < width; i++) {
      result_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
      img_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
    }
    linear_to_image(image_result, img_matrix, width, height);

    uint8_t *img_array = malloc(sizeof(uint8_t) * width * height);
    if (recvpacket->flags == IMG_FLAG_ROTATE_270) {
      flip_upside_down(img_matrix, result_matrix, width, height);
    } else {
      flip_left_to_right(img_matrix, result_matrix, width, height);
    }
    flatten_mat(result_matrix, img_array, width, height);

>>> This will create a temparary file titled pthread_id.png for the rotated image. 
    char temp_file[BUFF_SIZE];
    sprintf(temp_file, "%ld.png", pthread_self());
    FILE *fp = fopen(temp_file, "w");
    if (fp == NULL) {
      perror("Fopen failed in server");
      exit(-1);
    }
    if (fclose(fp) < 0) {
      perror("Fclose failed in server");
      exit(-1);
    }

    stbi_write_png(temp_file, width, height, CHANNEL_NUM, img_array,
                   width * CHANNEL_NUM);

    char image_data[BUFF_SIZE];
    memset(image_data, 0, BUFF_SIZE);
    FILE *fp2 = fopen(temp_file, "r");
    if (fp2 == NULL) {
      perror("Fopen fp2 failed in server");
      exit(-1);
    }

    int bytes_read = fread(image_data, sizeof(char), BUFF_SIZE, fp2);

    if (send(*client_fd, image_data, bytes_read, 0) < 0) {
      perror("Image send failed in server");
      exit(-1);
    }

    if (fclose(fp2) < 0) {
      perror("Fclose failed in server");
      exit(-1);
    }
    if (remove(temp_file) < 0) {
      perror("Remove failed in server");
      exit(-1);
    }
    free(image);
    free(recvpacket);
  }
  free(client_fd);
  return NULL;
}

int main(int argc, char *argv[]) {
  // Creating socket file descriptor
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("Socket failed in server");
    exit(-1);
  }
  printf("Server starting...\n");
  // Bind the socket to the port
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htons(INADDR_ANY);

  int ret = bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("Bind failed in server");
  }
  // Listen on the socket
  ret = listen(socket_fd, MAX_CLIENTS);
  if (ret < 0) {
    perror("Listen failed in server");
  }
  // Accept connections and create the client handling threads
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  int connect_fd;
  while (1) {
>>> Removed the signal handling from intermediate. Found out that the client sent a bad packet that causing the server to still run after pkill, and not pkill was causing the issue.
    connect_fd =
        accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (connect_fd < 0) {
      perror("Accept failed in server");
      exit(-1);
    }

>>> Also found out that connect_fd was being overwritten in the next loop before the thread could run, causing an error Socket operation on non-socket. Malloc'd the connect pointer to fix. 
    int *connect_fd_ptr = (int *)malloc(sizeof(int));
    *connect_fd_ptr = connect_fd;
    // >>> Create thread for each client
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, (void *)clientHandler,
                       connect_fd_ptr) < 0) {
      perror("Unable to create processing thread\n");
    }
    if (pthread_detach(client_thread) < 0) {
      perror("Detatch client thread failed");
    }
    // Release any resources
  }
  if (close(socket_fd) < 0) {
    perror("Close socket_fd failed in server");
  }
  printf("Server Ending...\n");
  return 0;
}

```
### client.c
```
#define _DEFAULT_SOURCE
#include "client.h"

#include <dirent.h>

#define PORT 1043  //>>> do000043, changed to 1043 to fit in range 1023-65536
#define BUFFER_SIZE 1024

int send_file(int socket, const char *filename) {
>>> Re-written the send_file from intermediate to handle ACK and images.
  // Open the file
  // Set up the request packet for the server and send it
  // Send the file data
  char image_data[BUFF_SIZE];
  memset(image_data, 0, BUFF_SIZE);

  char buffer[BUFF_SIZE];
  int ret = recv(socket, buffer, BUFF_SIZE, 0);

  packet_t *recvpacket = (packet_t *)malloc(sizeof(packet_t));
  memset(recvpacket, 0, sizeof(packet_t));
  memcpy(recvpacket, buffer, sizeof(packet_t));

  if (recvpacket->operation == IMG_OP_NAK) {
    free(recvpacket);
    return -1;
  }
  free(recvpacket);

>>> As discussed earlier in the server.c the client will be sending chunks of BUFF_SIZE untill it has done reading. Found out that the image size is less than BUFF_SIZE.
  FILE *fp = fopen(filename, "r");
  int bytes_read;
  while (1) {
    bytes_read = fread(image_data, sizeof(char), BUFF_SIZE, fp);
    if (bytes_read <= 0) {
      break;
    }
    ret = send(socket, image_data, bytes_read, 0);
  }

  if (fclose(fp) < 0) {
    perror("Fclose failed in client");
    exit(-1);
  }
  return 0;
}

int receive_file(int socket, const char *filename) {
>>> Re-written received file to use the number of bytes sent by server
  // Open the file
  // Receive response packet
  char new_image[BUFF_SIZE];
  memset(new_image, 0, BUFF_SIZE);
  // Receive the file data
  int bytes = recv(socket, new_image, BUFF_SIZE, 0);
  if (bytes < 0) {
    perror("Recv for image failed in client");
    exit(-1);
  }

  FILE *new_image_fp = fopen(filename, "w");
  if (new_image_fp == NULL) {
    perror("New Image fopen failed in client\n");
    exit(-1);
  }
  // Write the data to the file
  if (fwrite(new_image, bytes, 1, new_image_fp) < 0) {
    perror("New image fwrite failed");
    exit(-1);
  }
  if (fclose(new_image_fp) < 0) {
    perror("Fclose failed in client");
    exit(-1);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr,
            "Usage: ./client File_Path_to_images File_Path_to_output_dir "
            "Rotation_angle. \n");
    return 1;
  }

  // Set up socket
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("Socket failed in client");
    exit(-1);
  }
  printf("Client starting...\n");
  // Connect the socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  int ret = connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("Connect failed in client");
    exit(-1);
  }
  // Read the directory for all the images to rotate
  char *input_dir = argv[1];
  char *output_dir = argv[2];
  int rotation = atoi(argv[3]);

  DIR *dir = opendir(input_dir);
  if (dir == NULL) {
    perror("Opendir failed in client");
    exit(-1);
  }

  request_t *queue = NULL;
  int queue_len = 0;

  // Add to queue
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if ((entry->d_type == DT_REG) &&
        (strstr(entry->d_name, ".png") != NULL)) {  // Only use .png
      request_t *image = malloc(sizeof(request_t));
      if (image == NULL) {
        perror("Malloc image failed in client");
        exit(-1);
      }
      image->rotation_angle = rotation;
      image->file_name = malloc(strlen(entry->d_name));
      strcpy(image->file_name, entry->d_name);
      image->next = NULL;
      if (queue == NULL) {
        queue = image;
        queue_len++;
      } else {
        request_t *temp = queue;
        while (temp->next != NULL) {
          temp = temp->next;
        }
        temp->next = image;
        queue_len++;
      }
    }
  }

  if (closedir(dir) < 0) {
    perror("Closedir failed in client");
    exit(-1);
  }
  // Send the image data to the server
>>> Added loop to handle multiple images in queue  
  while (queue_len > 0) {
    packet_t packet;
    // packet.operation
    packet.operation = IMG_OP_ROTATE;
    // packet.flags
    if (rotation == 180) {
      packet.flags = IMG_FLAG_ROTATE_180;
    } else if (rotation == 270) {
      packet.flags = IMG_FLAG_ROTATE_270;
    } else {
      perror("Rotation value not supported");
      exit(-1);
    }
    // packet.size
    char buffer[BUFF_SIZE] = "\0";
    sprintf(buffer, "%s/%s", input_dir, queue->file_name);
    FILE *fp = fopen(buffer, "r");

    if (fp == NULL) {
      perror("File open failed in client");
      exit(-1);
    }
    if (fseek(fp, 0, SEEK_END) < 0) {
      perror("Fseek failed in client");
      exit(-1);
    }
    int image_size = ftell(fp);
    if (image_size < 0) {
      perror("Ftell failed in client");
      exit(-1);
    }
    fclose(fp);

    packet.size = htons(image_size);

>>> The packet here was the error caused by a typo in the intermediate, serialized data was (char *)malloc(sizeof(sizeof(char)) * PACKET_SIZE) instead of (char *)malloc(sizeof(char) * PACKET_SIZE);
    int PACKET_SIZE = sizeof(packet_t);    
    char *serializedData = (char *)malloc(sizeof(char) * PACKET_SIZE);
    memset(serializedData, 0, PACKET_SIZE);
    memcpy(serializedData, &packet, PACKET_SIZE);

    ret = send(socket_fd, serializedData, sizeof(packet), 0);
    if (ret < 0) {
      perror("Send failed in client");
      exit(-1);
    }
    free(serializedData);

    send_file(socket_fd, buffer);

    char new_output_dir[BUFF_SIZE];
    sprintf(new_output_dir, "%s/%s", output_dir, queue->file_name);

    receive_file(socket_fd, new_output_dir);
    // Check that the request was acknowledged
    // Receive the processed image and save it in the output dir

    request_t *prev = queue;
    queue = queue->next;
    queue_len--;
    free(prev->file_name);
    free(prev);
  }
  // Terminate the connection once all images have been processed
>>> Added send EXIT packet to server.  
  packet_t end_packet;
  end_packet.operation = IMG_OP_EXIT;
  int PACKET_SIZE = sizeof(packet_t);
  char *serialized_data2 = (char *)malloc(sizeof(char) * PACKET_SIZE);
  memset(serialized_data2, 0, PACKET_SIZE);
  memcpy(serialized_data2, &end_packet, PACKET_SIZE);

  ret = send(socket_fd, serialized_data2, sizeof(end_packet), 0);
  if (ret < 0) {
    perror("Send Exit failed in client");
    exit(-1);
  }
  free(serialized_data2);

  if (close(socket_fd) < 0) {
    perror("Close socket_fd failed in client");
    exit(-1);
  }
  // Release any resources
  //>>> Free malloc queue and print testing
  printf("Client: Files found left in queue:\n");
  request_t *current = queue;
  while (current != NULL) {
    printf("NAME:%10s ROTATION:%4d\n", current->file_name,
           current->rotation_angle);
    request_t *temp = current;
    current = current->next;
    free(temp->file_name);
    free(temp);
  }
  printf("Client ending...\n\n");
  return 0;
}
```