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
#include <iostream>
#include <cmath>
#include <netinet/tcp.h>
#include <iomanip>

#define MSG_MAXSIZE 1024

struct packet {
  uint16_t len;
  char message[MSG_MAXSIZE + 1];
};

struct client_to_server {
  char topic[MSG_MAXSIZE + 1];
  char type;
};

int recv_all(int sockfd, void *buffer, size_t len) {

  size_t bytes_received = 0;
  size_t bytes_remaining = len;

    while(bytes_remaining) {
        int rc = recv(sockfd, buffer + bytes_received, bytes_remaining, 0);
        if (rc <= 0) return rc;
        bytes_remaining -= rc;
        bytes_received += rc;
    }

  return bytes_received;
}

void run_client(int sockfd) {
  char buf[MSG_MAXSIZE + 1];
  memset(buf, 0, MSG_MAXSIZE + 1);

  struct client_to_server sent_packet;
  struct packet recv_packet;
  char udp_msg[1501];

  struct pollfd pfds[10];
  int nfds = 0;

  pfds[nfds].fd = STDIN_FILENO;
  pfds[nfds].events = POLLIN;
  nfds++;

  pfds[nfds].fd = sockfd;
  pfds[nfds].events = POLLIN;
  nfds++;

  while (1) {
    poll(pfds, nfds, -1);

    if ((pfds[0].revents & POLLIN) != 0) {
      fgets(buf, sizeof(buf), stdin);
      if (strncmp(buf, "subscribe", 9) == 0) {
        char* p = strtok(buf, " "); 
        p = strtok(NULL, " ");
        strcpy(sent_packet.topic, p);
        sent_packet.type = 1;

        send(sockfd, &sent_packet, sizeof(sent_packet), 0);
        std::cout << "Subscribed to topic " << sent_packet.topic;
      } else if (strncmp(buf, "unsubscribe", 11) == 0) {
        char* p = strtok(buf, " "); 
        p = strtok(NULL, " ");
        strcpy(sent_packet.topic, p);
        sent_packet.type = 0;

        send(sockfd, &sent_packet, sizeof(sent_packet), 0);
      } else if (strncmp(buf, "exit", 4) == 0) {
        return;
      }
    } else if ((pfds[1].revents & POLLIN) != 0) {
      char buffer[1552];
      char str[2];
      recv(sockfd, &str, 2, 0);
      short len = str[0] + (str[1] << 8);;

      int n = recv_all(sockfd, &buffer, len);
      if (n <= 0) {
        break;
      } else {
        char topic[50], type;
        memcpy(topic, buffer, 50);
        topic[50] = '\0';
        type = *(buffer + 50);
        switch ((int)type) {
          case 0: {
            int val;
            memcpy(&val, buffer + 52, n - 52);
            char sign;
            memcpy(&sign, buffer + 51, 1);
            val = (-2 * sign + 1) * htonl(val);
            std::cout << topic << " - INT - " << val << "\n";
            break;
          }
          case 1: {
            uint16_t val;
            memcpy(&val, buffer + 51, n - 51);
            val = htons(val);
            std::cout << topic << " - SHORT_REAL - " << std::fixed << std::setprecision(3) << val / 100.0 << "\n";
            break;
          }
          case 2: {
            int val;
            memcpy(&val, buffer + 52, 4);
            char sign;
            memcpy(&sign, buffer + 51, 1);
            u_int8_t mant;
            memcpy(&mant, buffer + 56, 1);
            val = (-2 * sign + 1) * htonl(val);
            std::cout << topic << " - FLOAT - " << std::fixed << std::setprecision((int)mant) <<  val / pow(10.0, (int)mant) << "\n";
            break;
          }
          case 3: {
            char value[1501];
            memcpy(value, buffer + 51, n - 51);
            value[n] = '\0';
            std::cout << topic << " - STRING - " << value << "\n";
            break;
          }
        }
      }
    }
  }

}

int main(int argc, char *argv[]) {
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  if (argc != 4) {
      std::cerr << "\n Usage: " << argv[0] <<" <id> <ip> <port>\n";
      return -1;
  }

  uint16_t port;
  int rc = sscanf(argv[3], "%hu", &port);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);

  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);

  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
      perror("setsockopt(SO_REUSEADDR) failed");
  enable = 1;
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int)) < 0)
      perror("setsockopt(TCP_NODELAY) failed");

  // create connection between server-client
  rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  packet send_pkt;
  strcpy(send_pkt.message, argv[1]);
  send_pkt.len = strlen(argv[1]) + 1;
  // send ID to server
  send(sockfd, &send_pkt, sizeof(send_pkt), 0);
  bool close_connection = false;
  recv(sockfd, &close_connection, sizeof(close_connection), 0);
  if (close_connection) {
    close(sockfd);
    return 0;
  }

  run_client(sockfd);

  close(sockfd);
  return 0;
}

