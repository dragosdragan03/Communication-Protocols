#include "common.hpp"

#include <sys/socket.h>
#include <sys/types.h>

/*
    face primirea a exact len octeți din buffer.
*/
int recv_all(int sockfd, void *buffer, size_t len) {
  size_t bytes_received = 0;
  size_t bytes_remaining = len;
  char *buff = (char *)buffer;

  size_t offset = 0;
  while (bytes_remaining) {
    bytes_received = recv(sockfd, buff + offset, len, 0);
    if (bytes_received == 0) {
      break;
    }
    offset += bytes_received;

    bytes_remaining -= bytes_received;
  }

  return offset;
}

/*
    face trimiterea a exact len octeți din buffer.
*/
int send_all(int sockfd, void *buffer, size_t len) {
  size_t bytes_sent = 0;
  size_t bytes_remaining = len;
  char *buff = (char *)buffer;

  size_t offset = 0;
  while (bytes_remaining) {
    bytes_sent = send(sockfd, buff + offset, len, 0);
    if (bytes_sent == 0) {
      break;
    }
    offset += bytes_sent;
    bytes_remaining -= bytes_sent;
  }
  return offset;
}
