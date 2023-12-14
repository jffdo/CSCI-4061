#define _DEFAULT_SOURCE
#include "client.h"

#include <dirent.h>

#define PORT 1043  //>>> do000043, changed to 1043 to fit in range 1023-65536
#define BUFFER_SIZE 1024

int send_file(int socket, const char *filename) {
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
  printf("Client: Unpackaged packed and recieved: %d\n", recvpacket->operation);

  if (recvpacket->operation == IMG_OP_NAK) {
    free(recvpacket);
    return -1;
  }
  free(recvpacket);

  FILE *fp = fopen(filename, "r");
  int bytes_read;
  while (1) {
    bytes_read = fread(image_data, sizeof(char), BUFF_SIZE, fp);
    if (bytes_read <= 0) {
      break;
    }
    ret = send(socket, image_data, bytes_read, 0);
    printf("Client sent: %d\n", ret);
  }

  if (fclose(fp) < 0) {
    perror("Fclose failed in client");
    exit(-1);
  }
  return 0;
}

int receive_file(int socket, const char *filename) {
  // Open the file
  // Receive response packet
  char new_image[BUFF_SIZE];
  memset(new_image, 0, BUFF_SIZE);
  // Receive the file data
  int bytes = recv(socket, new_image, BUFF_SIZE, 0);
  printf("Received %d bytes\n", bytes);
  if (bytes < 0) {
    perror("Recv for image failed in client");
    exit(-1);
  }

  printf("Client recieved %d bytes\n", bytes);

  printf("Opening... %s\n", filename);
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
    printf("opening %s...\n", buffer);
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
    printf("Client: Image size: %d\n", image_size);

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
  printf("Client: SENDING END\n");
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
