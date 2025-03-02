#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <iostream>
#include <cstring>
#include <sys/poll.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <sstream>
#define MAXLINE 1024
#define SA struct sockaddr

std::vector<std::string> id;
std::unordered_map<std::string, int> id_client;
std::unordered_map<std::string, std::vector<std::string>> topic_subscribers;
std::queue<char[MAXLINE]> data_queue;
struct udp_structure {
    char topic[50];
    char type;
    char value[1500];
};
#define MSG_MAXSIZE 1024
struct packet {
  uint16_t len;
  char message[MSG_MAXSIZE + 1];
};

struct client_to_server {
  char topic[MSG_MAXSIZE + 1];
  char type;
};

int send_all(int sockfd, void *buffer, size_t len) {

  size_t bytes_sent = 0;
  size_t bytes_remaining = len;

  while(bytes_remaining) {
    int sd = send(sockfd, buffer + bytes_sent, bytes_remaining, 0);
    if (sd < 0) return sd;
    bytes_remaining -= sd;
    bytes_sent += sd;
  }

  return bytes_sent;
}

bool check_topic(const std::string& topic, const std::string& key) {
  bool found = false;
  std::stringstream ss(topic);
  std::vector<std::string> vec_topic;

  while( ss.good() )
  {
    std::string substr;
    std::getline( ss, substr, '/' );
    vec_topic.push_back( substr );
  }

  for (auto& subscribed : topic_subscribers[key]) {
    std::stringstream ss(subscribed);
    std::vector<std::string> vec_sub;

    while( ss.good() )
    {
      std::string substr;
      std::getline( ss, substr, '/' );
      vec_sub.push_back( substr );
    }

    int i = 0, j = 0;
    int n = vec_topic.size(), m = vec_sub.size();

    bool star = false;
    while (i < n && j < m) {
      if (strncmp(vec_sub[j].c_str(), "+", 1) == 0) {
        i++;
        j++;
        continue;
      }
      if (strncmp(vec_sub[j].c_str(), "*", 1) == 0) {
        i++;
        j++;
        star = true;
        continue;
      }
      if (star == true) {
        if (strncmp(vec_sub[j].c_str(), vec_topic[i].c_str(), vec_sub[j].size()) == 0) {
          i++;
          j++;
          star = false;
          continue;
        }
        i++;
        continue;
      }
      if (strncmp(vec_sub[j].c_str(), vec_topic[i].c_str(), vec_sub[j].size()) == 0) {
        i++;
        j++;
        continue;
      } else {
        break;
      }
    }
    if ((i >= n && j >= m) || star) {
      found = true;
      break;
    }
  }

  return found;
}

void run_multi_server(int listenfd, int sockfd) {

  struct pollfd poll_fds[1000];
  int num_clients = 3;
  int rc;

  struct packet received_packet;

  rc = listen(listenfd, 1000);

  // initialize the default sockets
  poll_fds[0].fd = 0;
  poll_fds[0].events = POLLIN;
  poll_fds[1].fd = sockfd;
  poll_fds[1].events = POLLIN;
  poll_fds[2].fd = listenfd;
  poll_fds[2].events = POLLIN;

  while (1) {
    rc = poll(poll_fds, num_clients, -1);

    for (int i = 0; i < num_clients; i++) {
      if (poll_fds[i].revents & POLLIN) {
        if (poll_fds[i].fd == listenfd) {
          // received a new connection from TCP socket
          struct sockaddr_in cli_addr;
          socklen_t cli_len = sizeof(cli_addr);
          int newsockfd =
              accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len);

          packet sock_id;
          recv(newsockfd, &sock_id, sizeof(sock_id), 0);
          std::string int_id = sock_id.message;
          bool k = false;
          for (auto& c : id_client) {
            if (c.first == int_id) k = true;
          }
          if (!k) {
            bool close_connection = k;
            send(newsockfd, &close_connection, sizeof(close_connection), 0);
            id.push_back(int_id);
            id_client[int_id] = newsockfd;
          } else {
            bool close_connection = k;
            send(newsockfd, &close_connection, sizeof(close_connection), 0);
            close(newsockfd);
            std::cout << "Client " + int_id + " already connected.\n";
            continue;
          }
          
          // new TCP connection
          poll_fds[num_clients].fd = newsockfd;
          poll_fds[num_clients].events = POLLIN;
          poll_fds[num_clients].revents = 0;
          num_clients++;

          std::cout << "New client " << int_id << " connected from " << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << "\n";
        } else if (poll_fds[i].fd == 0) {
            char buff[100];
            int len = read(0, buff, 99);
            if (strncmp("exit", buff, len-1) == 0) {
              printf("Closing server\n");
              for (int i = num_clients - 1; i >= 0; i--) {
                close(poll_fds[i].fd);
              }
              return;
            }
        } else if (poll_fds[i].fd == sockfd) {
          char buffer[MAXLINE * 2];
          int n = recv(sockfd, &buffer, sizeof(buffer), 0);

          char topic[50];
          strncpy(topic, buffer, 50);
          topic[50] = '\0';

          for (int i = 3; i < num_clients; i++) {
            std::string key = "";
            for (auto &c : id_client) {
              if (c.second == poll_fds[i].fd) {
                key = c.first;
                break; // to stop searching
              }
            }

            if (!check_topic(topic, key)) continue;
            topic[51] = *(buffer + 51);
            char str[2];
            str[0] = n & 255;
            str[1] = ((n & 65280) >> 8);
            int sd = send (poll_fds[i].fd, &str, 2, 0);
            sd = send_all(poll_fds[i].fd, &buffer, n);
            if (sd < 0) {
              std::cout << "TCP send caught an error\n";
            }
          }
        } else {
          // TCP event
          client_to_server received_pkt;
          int rc = recv(poll_fds[i].fd, &received_pkt,
                            sizeof(received_pkt), 0);

          if (rc == 0) {
            // connection closed
            close(poll_fds[i].fd);
            std::string temp_id = "";
            for (auto& c : id_client) {
              if (c.second == poll_fds[i].fd) temp_id = c.first;
            }
            std::cout << "Client " + temp_id + " disconnected.\n";
            id_client.erase(temp_id);
            close(poll_fds[i].fd);

            // reorder the vector of sockets after the removal
            for (int j = i; j < num_clients - 1; j++) {
              poll_fds[j] = poll_fds[j + 1];
            }

            num_clients--;

          } else {
            std::string topic = received_pkt.topic;
            std::replace(topic.begin(), topic.end(), '\n', '\0');
            std::string key = "";
            for (auto &c : id_client) {
              if (c.second == poll_fds[i].fd) {
                key = c.first;
                break; // to stop searching
              }
            }
            if (key != "") {
              if (received_pkt.type == 1) {
                topic_subscribers[key].push_back(topic);
              } else {
                auto it = find(topic_subscribers[key].begin(), topic_subscribers[key].end(), 
                        topic);
  
                // If element is found found, erase it 
                if (it != topic_subscribers[key].end()) { 
                  topic_subscribers[key].erase(it); 
                } 
              }
            }
          }
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  char buffer[MAXLINE];
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  if (argc != 2) {
      std::cerr << "\n Usage: " << argv[0] <<" <port>\n";
      return 1;
  }

  int sockfd;
  struct sockaddr_in servaddr;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      std::cerr << "socket creation failed\n";
      exit(EXIT_FAILURE);
  }

  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
      perror("setsockopt(SO_REUSEADDR) failed");

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(std::stoi(argv[1]));
  if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
          sizeof(servaddr)) < 0 ) 
  { 
      perror("bind failed"); 
      exit(EXIT_FAILURE); 
  }

  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  enable = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
      perror("setsockopt(SO_REUSEADDR) failed");
  enable = 1;
  if (setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int)) < 0)
      perror("setsockopt(TCP_NODELAY) failed");
  
  if ( bind(listenfd, (const struct sockaddr *)&servaddr,  
          sizeof(servaddr)) < 0 ) 
  { 
      perror("bind failed"); 
      exit(EXIT_FAILURE); 
  }

  run_multi_server(listenfd, sockfd);
     
  return 0; 
}
