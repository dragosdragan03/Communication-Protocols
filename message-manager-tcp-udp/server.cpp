#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include "common.hpp"
#include "helpers.h"
#include <vector>
#include <netinet/tcp.h>
#include <iostream>

using namespace std;

#define MAX_CONNECTIONS 32
#define IP_ADDR "127.0.0.1"

int index_found_client(vector<tcp_clients> clients, string new_id_client)
{
  for (long unsigned int i = 0; i < clients.size(); i++) {
    if (!strcmp(clients[i].id_client.c_str(), new_id_client.c_str())) {
      return i;
    }
  }
  return -1;
}

int find_index_topic(vector<string> topics, string topic_unsubscribe)
{
  for (long unsigned int i = 0; i < topics.size(); i++) {
    if (!strcmp(topics[i].c_str(), topic_unsubscribe.c_str())) {
      return i;
    }
  }
  return -1;
}

bool wild_card(vector<string> topics, string topic_send)
{

  vector<string> vector_topic_send;
  char *token_received = strtok(const_cast<char *>(topic_send.c_str()), "/");
  while (token_received != nullptr) {
    string aux = token_received;
    vector_topic_send.push_back(aux);
    token_received = strtok(nullptr, "/");
  }
  int length = vector_topic_send.size();

  for (long unsigned int i = 0; i < topics.size(); i++) {
    int iterator = 0;
    char *top = const_cast<char *>(topics[i].c_str());
    char *token_topic = strtok(top, "/"); // aici se pot afla + sau *
    while (iterator < length && token_topic != nullptr) {
      if (!strcmp(token_topic, "+")) { // daca este plus atunci atunci trec peste acest nivel
        token_topic = strtok(nullptr, "/");
        iterator++;
      } else if (!strcmp(token_topic, "*")) { // parcurg pana gasesc 2 la fel
        token_topic = strtok(nullptr, "/");
        iterator++;
        if (token_topic == nullptr) { // inseamna ca * e la finalul stringului
          return true;
        }

        while (iterator < length && strcmp(vector_topic_send[iterator].c_str(), token_topic)) { // verific cat timp sunt diferite
          iterator++;
        }

        if (length == iterator) { // inseamna ca a ajuns la final si nu s-au fct egale
          break;
        }
      }
      if (iterator == length && token_topic == nullptr) {
        return true;
      }
      if (token_topic != nullptr) {
        if (iterator > length || strcmp(token_topic, vector_topic_send[iterator].c_str())) { // daca nu sunt egale inseamna ca este fals
          break;
        }
      }

      iterator++;
      token_topic = strtok(nullptr, "/");
    }
    if (iterator == length && token_topic == nullptr) {
      return true;
    }
  }
  return false;
}

int index_client_to_remove(const vector<tcp_clients> &clients, int sockfd)
{
  for (long unsigned int i = 0; i < clients.size(); i++) {
    if (clients[i].socket == sockfd) {
      return i;
    }
  }
  return -1;
}

/*comunicarea intre mai multi clienti*/
void run_chat_multi_server(int listenfd, int udp_fd) {

  vector<pollfd> poll_fds;

  char buf[MSG_MAXSIZE + 1];
  memset(buf, 0, MSG_MAXSIZE + 1);

  struct chat_packet sent_packet;
  struct chat_packet received_packet;

  vector<tcp_clients> clients;
  int num_sockets = 3;
  int rc;

  // Setam socket-ul listenfd pentru ascultare
  // imi deschid o conexiune noua prin listenfd ca sa creez un socket
  rc = listen(listenfd, MAX_CONNECTIONS);
  DIE(rc < 0, "listen");

  // Adaugam noul file descriptor (socketul pe care se asculta conexiuni) in
  // multimea poll_fds
  struct pollfd new_poll_listen; // file descriptorul pe care se accepta conexiuni
  new_poll_listen.fd = listenfd;
  new_poll_listen.events = POLLIN;
  poll_fds.push_back(new_poll_listen);

  struct pollfd new_poll_udp;
  new_poll_udp.fd = udp_fd; // file descriptorul clientului UDP
  new_poll_udp.events = POLLIN;
  poll_fds.push_back(new_poll_udp);

  struct pollfd new_poll_in; // citeste de la tastaura in server (socket ul pentru asa ceva)
  new_poll_in.fd = STDIN_FILENO;
  new_poll_in.events = POLLIN;
  poll_fds.push_back(new_poll_in);

  while (1) {
    // Asteptam sa primim ceva pe unul dintre cei num_sockets socketi
    rc = poll(poll_fds.data(), num_sockets, -1);
    DIE(rc < 0, "poll");

    for (int i = 0; i < num_sockets; i++) { // itereaza prin toate socketurile si verifica daca am eveniment pe ele
      if (poll_fds[i].revents & POLLIN) { // verifica daca pe socketul "i" a primit un semnal
        if (poll_fds[i].fd == listenfd) { // verificam daca s a facut o conexiune noua
          // Am primit o cerere de conexiune pe socketul de listen, pe care
          // o acceptam
          struct sockaddr_in cli_addr;
          socklen_t cli_len = sizeof(cli_addr);
          // conexiunea intre server si client
          const int newsockfd =
            accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len);
          DIE(newsockfd < 0, "accept");

          int subscriber = recv_all(newsockfd, &received_packet,
            sizeof(received_packet));
          DIE(subscriber < 0, "recv");

          char ip_addr[16]; // adresa ip
          int port_number;
          char id_client[50]; // id_client

          sscanf(received_packet.message, "%s %s %d", id_client, ip_addr, &port_number);
          string aux(id_client);

          int index_client = index_found_client(clients, aux);
          if (index_client != -1) { // inseamna ca deja exista un client activ sau inactiv
            if (clients[index_client].is_active == true) { // inseamna ca este deja activ
              //Client <ID_CLIENT> already connected.
              cout << "Client " << id_client << " already connected.\n";
              close(newsockfd);
              continue;
            }
          }
          // received_packet e de forma <ID_CLIENT> <IP_SERVER> <PORT_SERVER>
          // Adaugam noul socket intors de accept() la multimea descriptorilor
          // de citire
          struct pollfd new_poll;
          new_poll.fd = newsockfd;
          new_poll.events = POLLIN;
          poll_fds.push_back(new_poll);

          num_sockets++;

          if (index_client == -1) { // inseamna ca nu a existat clientul pana acum
            tcp_clients new_client;
            new_client.id_client.assign(aux);
            new_client.is_active = true;
            new_client.socket = newsockfd;

            clients.push_back(new_client);
          } else { // inseamna ca este inactiv si reconectat
            clients[index_client].is_active = true;
            clients[index_client].socket = newsockfd;
          }

          // New client <ID_CLIENT> connected from IP:PORT.
          cout << "New client " << id_client << " connected from " << ip_addr << ":" << port_number << ".\n";
        } else if (poll_fds[i].fd == udp_fd) {
          char buffer[1552];
          struct sockaddr_in sin;
          socklen_t sin_len = sizeof(sin);
          int bytes_received = recvfrom(poll_fds[i].fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sin, &sin_len);
          buffer[bytes_received] = '\0';
          Message *msg = (Message *)buffer;

          memcpy(sent_packet.message, msg, sizeof(Message));
          sent_packet.len = sizeof(Message);
          // -- partea de cod comentata reprezinta trimiterea unui pachet care contine <IP_SERVER> <PORT_SERVER>
          // ale clientului udp --

          // struct chat_packet details;

          // char ip_port[16];
          // sprintf(ip_port, "%s %d", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
          // strcpy(details.message, ip_port);
          // details.len = strlen(ip_port) + 1;

          for (long unsigned int j = 0; j < clients.size(); j++) { // parcurg toti clientii 
            if (wild_card(clients[j].topics, msg->topic) && clients[j].is_active) {
              send_all(clients[j].socket, &sent_packet, sizeof(sent_packet)); // trimit pachetul primit la toti ceilalti clienti
              // send_all(clients[j].socket, &details, sizeof(details)); // trimit pachetul care contine <IP_SERVER> <PORT_SERVER>
            }
          }
        } else if (poll_fds[i].fd == STDIN_FILENO) { // verific daca primesc de la stdin

          if (!fgets(buf, sizeof(buf), stdin) || isspace(buf[0])) {
            break; // Exit loop if input is empty or whitespace
          }
          sent_packet.len = strlen(buf) + 1;
          strcpy(sent_packet.message, buf);
          if (!strcmp(sent_packet.message, "exit\n")) {
            for (int j = 0; j < num_sockets; j++) {
              close(poll_fds[j].fd);
            }
            return;
          }
        } else { // daca s a primit ceva pe socketul lui inseamna ca am primit un mesaj. (subscriberul deja exista)
          // Am primit date pe unul din socketii de client, asa ca le receptionam
          int rc = recv_all(poll_fds[i].fd, &received_packet,
            sizeof(received_packet));
          DIE(rc < 0, "recv");

          int index = index_client_to_remove(clients, poll_fds[i].fd);
          if (index == -1) {
            continue;
          }

          if (rc == 0 || !strcmp(received_packet.message, "exit\n")) { // verific daca trebuie inchis socketul
            // Client <ID_CLIENT> disconnected.
            cout << "Client " << clients[index].id_client << " disconnected.\n";
            close(poll_fds[i].fd);
            poll_fds[i].revents = 0;
            // Scoatem din multimea de citire socketul inchis
            poll_fds.erase(poll_fds.begin() + i);

            clients[index].is_active = false;
            clients[index].socket = -1;
            num_sockets--;
            i--;
          } else { // inseamna ca serverul a primit un mesaj de pe un socket
            const char *spacePtr = strchr(received_packet.message, ' ');
            if (spacePtr == nullptr) {
              continue;
            }

            string topic(spacePtr + 1);
            size_t newline_pos = topic.find('\n');
            if (newline_pos != string::npos) {
              topic[newline_pos] = '\0';
            }
            if (!strncmp(received_packet.message, "subscribe", spacePtr - received_packet.message)) {
              clients[index].topics.push_back(topic); // adaug topicul in vectorul clientui
            } else if (!strncmp(received_packet.message, "unsubscribe", spacePtr - received_packet.message)) {
              int index_topic = find_index_topic(clients[index].topics, topic);
              if (index_topic == -1) { // inseamna ca nu s a gasit topicul
                continue;
              }

              clients[index].topics.erase(clients[index].topics.begin() + index_topic);
            }
          }
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {

  // primim ./server si PORTUL
  if (argc != 2) {
    printf("\n Usage: %s <ip> <port>\n", argv[0]);
    return 1;
  }

  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  // Parsam port-ul ca un numar
  uint16_t port;
  int rc = sscanf(argv[1], "%hu", &port);
  DIE(rc != 1, "Given port is invalid");

  // Obtinem un socket TCP pentru receptionarea conexiunilor
  const int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(listenfd < 0, "socket");

  // dezactivarea algoritmului lui Nagle
  int flag = 1;
  int result = setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
  DIE(result == -1, "Failed to set socket options");

  const int upd_fd = socket(AF_INET, SOCK_DGRAM, 0); // creez un file descriptor pentru clientii udp
  DIE(upd_fd < 0, "socket");

  // Completăm in serv_addr adresa serverului, familia de adrese si portul
  // pentru conectare
  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);

  // Facem adresa socket-ului reutilizabila, ca sa nu primim eroare in caz ca
  // rulam de 2 ori rapid
  const int enable = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");

  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr.s_addr);
  DIE(rc <= 0, "inet_pton");

  // Asociem adresa serverului cu socketul tcp creat folosind bind
  rc = bind(listenfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "bind");

  // Asociem adresa serverului cu socketul udp creat folosind bind 
  rc = bind(upd_fd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "bind");

  run_chat_multi_server(listenfd, upd_fd);

  // Inchidem listenfd
  close(listenfd);

  return 0;
}