#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <string>

int send_all(int sockfd, void *buff, size_t len);
int recv_all(int sockfd, void *buff, size_t len);

/* Dimensiunea maxima a mesajului */
#define MSG_MAXSIZE 2000

struct chat_packet {
  uint16_t len;
  char message[MSG_MAXSIZE + 1];
};

struct Message {
  char topic[50]; // 50 bytes
  char tip_date[1];
  char continut[1501]; // 1500 bytes + null terminator
  char message[MSG_MAXSIZE + 1];
};

struct tcp_clients {
  int socket;
  std::string id_client; // numele clientului
  std::vector<std::string> topics; // topicurile la care este abonat
  bool is_active; // daca este sau nu activ
};

#endif
