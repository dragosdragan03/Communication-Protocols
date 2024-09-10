#include <iomanip>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <iostream>

#include "common.hpp"
#include "helpers.h"

using namespace std;

void type_int(Message msg)
{
  uint8_t sign = msg.continut[0]; // retin semnul numarului

  uint32_t value_network_order;
  memcpy(&value_network_order, &(msg.continut[1]), sizeof(uint32_t));

  int value;
  if (sign == 1) { // inseamna ca este numar negativ
    value = -ntohl(value_network_order);
  } else {
    value = ntohl(value_network_order);
  }

  cout << msg.topic << " - " << "INT" << " - " << value << "\n";
}

void type_short_real(Message msg)
{
  uint16_t data;
  memcpy(&data, msg.continut, sizeof(uint16_t));
  uint16_t host_order_data = ntohs(data);

  cout << msg.topic << " - " << "SHORT_REAL" << " - " << fixed << setprecision(2) << host_order_data / 100.0f << "\n";
}

void type_float(Message msg) {

  int8_t sign = msg.continut[0];
  uint32_t modulus;
  memcpy(&modulus, &msg.continut[sizeof(uint8_t)], sizeof(uint32_t));
  modulus = ntohl(modulus);

  uint8_t power = msg.continut[sizeof(uint8_t) + sizeof(uint32_t)];

  float number = static_cast<float>(modulus) / static_cast<int>(pow(10, static_cast<int>(power)));

  if (sign == 1)
    number = -number;

  cout << msg.topic << " - " << "FLOAT" << " - " << fixed << setprecision(power) << number << std::endl;
}

void type_string(Message msg)
{
  cout << msg.topic << " - STRING - " << msg.continut << "\n";
}

void run_client(int sockfd) {
  char buf[MSG_MAXSIZE + 1];
  memset(buf, 0, MSG_MAXSIZE + 1);

  struct chat_packet sent_packet;
  struct chat_packet recv_packet;

  struct pollfd fds[10];

  // creez un pollfd care contine un socket pentru citirea de la tastatura si altul pentru citirea de la server
  fds[0].fd = STDIN_FILENO; // citeste de la tastaura (socket ul pentru asa ceva)
  fds[0].events = POLLIN;

  fds[1].fd = sockfd; // mesaje primite de la server
  fds[1].events = POLLIN;

  while (1) {
    int ret = poll(fds, 2, -1); // Wait indefinitely for events on file descriptors

    if (ret < 0) {
      perror("poll");
      break;
    }

    if (fds[0].revents & POLLIN) { // Check if there is data to read from stdin
      if (!fgets(buf, sizeof(buf), stdin) || isspace(buf[0])) {
        break; // Exit loop if input is empty or whitespace
      }

      sent_packet.len = strlen(buf) + 1;
      strcpy(sent_packet.message, buf);

      send_all(sockfd, &sent_packet, sizeof(sent_packet));

      const char *spacePtr = strchr(sent_packet.message, ' ');

      if (spacePtr == nullptr) {
        continue;
      }


      string topic(spacePtr + 1);
      size_t newline_pos = topic.find('\n');
      if (newline_pos != string::npos) {
        topic[newline_pos] = '\0';
      }
      if (!strncmp(sent_packet.message, "subscribe", spacePtr - sent_packet.message)) {
        cout << "Subscribed to topic " << topic << ".\n";
      } else if (!strncmp(sent_packet.message, "unsubscribe", spacePtr - sent_packet.message)) {
        cout << "Unsubscribed from topic " << topic << ".\n";
      }
    }

    if (fds[1].revents & POLLIN) { // Check if there is data to read from the socket
      int rc = recv_all(sockfd, &recv_packet, sizeof(recv_packet));
      if (rc <= 0) {
        break;
      }

      Message *msg = (Message *)recv_packet.message;

      // -- partea de cod comentata reprezinta receptionarea unui pachet ce contien IP_PORT si PORT ul clientului udp --

      // struct chat_packet recv_packet_details;

      // int ret = recv_all(sockfd, &recv_packet_details,
      //   sizeof(recv_packet_details));
      // DIE(ret < 0, "recv");

      // char ip_port[16];
      // int port_number;
      // sscanf(recv_packet_details.message, "%s %d", ip_port, &port_number);

      string tip_data;
      if ((int)msg->tip_date[0] == 0) {
        type_int(*msg);
      } else if ((int)msg->tip_date[0] == 1) {
        type_short_real(*msg);
      } else if ((int)msg->tip_date[0] == 2) {
        type_float(*msg);
      } else if ((int)msg->tip_date[0] == 3) {
        type_string(*msg);
      }
    }
  }
}

int main(int argc, char *argv[]) {

  // ./subscriber <ID_CLIENT> <IP_SERVER> <PORT_SERVER>
  if (argc != 4) {
    printf("\n Usage: %s <ip> <port>\n", argv[0]);
    return 1;
  }
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  // Parsam port-ul ca un numar
  uint16_t port;
  int rc = sscanf(argv[3], "%hu", &port);
  DIE(rc != 1, "Given port is invalid");

  // Obtinem un socket TCP pentru conectarea la server
  const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "socket");

  // Completăm in serv_addr adresa serverului, familia de adrese si portul
  // pentru conectare
  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);

  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
  DIE(rc <= 0, "inet_pton");

  // Ne conectăm la server
  rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "connect");

  struct chat_packet received_packet;

  received_packet.len = strlen(argv[1]) + strlen(argv[2]) + strlen(argv[3]) + 3;

  memcpy(received_packet.message, argv[1], strlen(argv[1]));
  received_packet.message[strlen(argv[1])] = ' '; // inserez un spatiu dupa argv[1]

  memcpy(received_packet.message + strlen(argv[1]) + 1, argv[2], strlen(argv[2]));
  received_packet.message[strlen(argv[1]) + strlen(argv[2]) + 1] = ' '; // inserez un spatiu dupa argv[2]

  memcpy(received_packet.message + strlen(argv[1]) + strlen(argv[2]) + 2, argv[3], strlen(argv[3]));
  received_packet.message[received_packet.len - 1] = '\0'; // Null terminator

  send_all(sockfd, &received_packet, sizeof(received_packet));

  run_client(sockfd);
  close(sockfd);

  return 0;
}
