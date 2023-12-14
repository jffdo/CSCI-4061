#include "server.h"

#define PORT 1043  //>>> do000043, changed to 1043 to fit in range 1023-65536
#define MAX_CLIENTS 5
#define BUFFER_SIZE 4024

void *clientHandler(void *socket) {
  // Receive packets from the client
  int *client_fd = (int *)socket;
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
    printf("Server: Unpackaged packed and recieved: %d, %d, %d\n",
           recvpacket->operation, recvpacket->flags, ntohs(recvpacket->size));
    if (recvpacket->operation == IMG_OP_EXIT) {
      printf("SERVER: RECIEVED END\n");
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
    printf("Sending ACK\n");
    packet_t ack_packet;
    if (recvpacket->operation == IMG_OP_ROTATE) {
      ack_packet.operation = IMG_OP_ACK;
    } else {
      ack_packet.operation = IMG_OP_NAK;
    }
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

    printf("Recieving\n");
    int image_ret;
    while (1) {
      image_ret = recv(*client_fd, buffer, BUFF_SIZE, 0);
      if (image_ret <= 0) {
        break;
      }
      memcpy(image + current, buffer, image_ret);
      current += image_ret;

      printf("Server recieved for image_size %d: +(%d), %d\n", image_size,
             image_ret, current);

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

    // >>> Create temp file buffer to send to client
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
    printf("Server sent %d\n", bytes_read);
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
    connect_fd =
        accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (connect_fd < 0) {
      perror("Accept failed in server");
      exit(-1);
    }

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
