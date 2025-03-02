#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "json.hpp"
#include "helpers.hpp"
#include "requests.hpp"
#include <iostream>
#include <poll.h>
#include <sstream>
#include <map>
#include <string>
#include <cstdlib>
#include <cwctype>

using json = nlohmann::json;
#define IP_HOST "34.246.184.49"
#define PORT 8080
std::string current_cookie;
std::string current_token;

bool isalpnum(std::string& str) {
    for (auto& c : str) {
        if (iswalnum(c) == 0) {
            return false;
        }
    }
    return true;
}

bool isalpnumspc(std::string& str) {
    for (auto& c : str) {
        if (iswalnum(c) == 0 && strchr("+-()*,.: ", c) == NULL) {
            return false;
        }
    }
    return true;
}

std::string extract_cookie(const std::string& str) {
    std::string cookie_key = "Set-Cookie: ";
    size_t cookie_begin = str.find(cookie_key);
    if (cookie_begin == std::string::npos)
        return "";

    cookie_begin += cookie_key.length();
    size_t cookie_end = str.find(';', cookie_begin);
    if (cookie_end == std::string::npos)
        cookie_end = str.length();

    return str.substr(cookie_begin, cookie_end - cookie_begin);
}

std::string get_result(const std::string& str) {
    size_t newline = str.find("\r\n\r\n");
    
    if (newline != std::string::npos)
        return str.substr(newline + 4);
    return "";
}

char* return_server_response(char* msg) {
    int sockfd = open_connection(IP_HOST, PORT, AF_INET, SOCK_STREAM, NULL);
    send_to_server(sockfd, msg);
    char* response = receive_from_server(sockfd);
    close_connection(sockfd);
    return response;
}

void register_user(const std::string& user, const std::string& pass) {
    if (user.empty() || pass.empty()) {
        std ::cout << "ERROR - <register user pass>\n";
        return;
    }

    json ex3 = {
        {"username", user},
        {"password", pass},
    };
    char* data[1];
    data[0] = strdup(ex3.dump().c_str());
    char* message = compute_post_request("34.246.184.49:8080",
     "/api/v1/tema/auth/register", "application/json", data, 1, NULL, 0, NULL);
    char* response = return_server_response(message);
    
    std::string result(response);
    std::string lastLine = get_result(result);

    if (strncmp("ok", lastLine.c_str(), 2) == 0) {
        std::cout << "SUCCESS - user registered\n";
    } else {
        auto j3 = json::parse(lastLine);
        std::cout << "ERROR - " << j3["error"] << "\n";
    }
}

void login_user(const std::string& user, const std::string& pass) {
    if (user.empty() || pass.empty()) {
        std ::cout << "ERROR - <register user pass>\n";
        return;
    }

    json ex3 = {
        {"username", user},
        {"password", pass},
    };
    char* data[1];
    data[0] = strdup(ex3.dump().c_str());
    char* message = compute_post_request("34.246.184.49:8080",
     "/api/v1/tema/auth/login", "application/json", data, 1, NULL, 0, NULL);
    char* response = return_server_response(message);

    std::string result(response);
    std::string lastLine = get_result(result);
    if (strcmp("ok", lastLine.c_str()) == 0) {
        std::cout << "SUCCESS - user logged\n";
        std::string result(response);
        current_cookie = extract_cookie(result);
    } else {
        auto j3 = json::parse(lastLine);
        std::cout << "ERROR - " << j3["error"] << "\n";
    }
}

void access_library(const std::string& cookie) {

    char* cookies[1];
    cookies[0] = strdup(cookie.c_str());
    char* message = compute_get_request("34.246.184.49:8080",
     "/api/v1/tema/library/access", "application/json", cookies, 1, NULL);
    char* response = return_server_response(message);

    std::string result(response);
    std::string lastLine = get_result(result);

    json j = json::parse(lastLine);
    if (j["token"] == nullptr) {
        std::cout << "ERROR - " << j["error"] << "\n";
    } else {
        std::cout << "SUCCESS - got access to the library\n";
        current_token = j["token"];
    }
}

void get_books(const std::string& token) {

    const char* auth = token.c_str();
    char* message = compute_get_request("34.246.184.49:8080",
     "/api/v1/tema/library/books", "application/json", NULL, 0, auth);
    char* response = return_server_response(message);

    std::string lastLine = get_result(response);
    json j = json::parse(lastLine);
    if (j.contains("error") && j["error"] != nullptr) {
        std::cout << "ERROR - " << j["error"] << "\n";
    } else {
        std ::cout << lastLine << "\n";
    }
}

void get_book(const std::string& token, const std::string& id) {
    if (id.empty()) {
        std::cout << "ERROR - <get_book id>\n";
        return;
    }
    std::string path = "/api/v1/tema/library/books/" + id;
    const char* final_path = path.c_str();
    const char* auth = token.c_str();
    char* message = compute_get_request("34.246.184.49:8080",
     final_path, "application/json", NULL, 0, auth);
    char* response = return_server_response(message);

    std::string lastLine = get_result(response);
    json j = json::parse(lastLine);
    if (j.contains("error") && j["error"] != nullptr) {
        std::cout << "ERROR - " << j["error"] << "\n";
    } else {
        std ::cout << lastLine << "\n";
    }
}

void upload_book(const std::string& token, const std::string& title, const std::string& author,
    const std::string& genre, const std::string& publisher, int page_count) {

    const char* auth = token.c_str();
    json ex3 = {
        {"title", title},
        {"author", author},
        {"genre", genre},
        {"page_count", page_count},
        {"publisher", publisher},
    };
    char* data[1];
    data[0] = strdup(ex3.dump().c_str());
    char* message = compute_post_request("34.246.184.49:8080",
     "/api/v1/tema/library/books", "application/json", data, 1, NULL, 0, auth);
    char* response = return_server_response(message);

    std::string result(response);
    std::string lastLine = get_result(result);
    if (strcmp("ok", lastLine.c_str()) == 0) {
        std::cout << "SUCCESS - book uploaded\n";
    } else {
        auto j3 = json::parse(lastLine);
        std::cout << "ERROR - " << j3["error"] << "\n";
    }
}

void delete_book(const std::string& token, const std::string& id) {

    std::string path = "/api/v1/tema/library/books/" + id;
    const char* final_path = path.c_str();
    const char* auth = token.c_str();
    char* message = compute_delete_request("34.246.184.49:8080",
     final_path, "application/json", NULL, 0, auth);
    char* response = return_server_response(message);

    std::string result(response);
    std::string lastLine = get_result(result);
    if (strcmp("ok", lastLine.c_str()) == 0) {
        std::cout << "SUCCESS - book deleted\n";
    } else {
        auto j3 = json::parse(lastLine);
        std::cout << "ERROR - " << j3["error"] << "\n";
    }
}

void logout_user(const std::string& cookie) {

    char* cookies[1];
    cookies[0] = strdup(cookie.c_str());
    char* message = compute_get_request("34.246.184.49:8080",
     "/api/v1/tema/auth/logout", "application/json", cookies, 1, NULL);
    char* response = return_server_response(message);

    std::string result(response);
    std::string lastLine = get_result(result);
    if (strcmp("ok", lastLine.c_str()) == 0) {
        std::cout << "SUCCESS - user logout\n";
        current_cookie = "";
        current_token = "";
    } else {
        auto j3 = json::parse(lastLine);
        std::cout << "ERROR - " << j3["error"] << "\n";
    }
}

int main(int argc, char *argv[])
{
    while (true) {
        char buf[1024];
        std::cin >> buf;

        std::stringstream ss(buf);
        std::string message;
        ss >> message;

        if ("register" == message) {
            bool valid = true;
            std::string user = "", pass = "";

            std::cout << "username= ";
            std::getline(std::cin >> std::ws, user);
            if (!isalpnum(user)) valid = false;

            std::cout << "password= ";
            std::getline(std::cin >> std::ws, pass);
            if (!isalpnum(pass)) valid = false;

            if (valid)
                register_user(user, pass);
            else
                std::cout << "ERROR - wrong parameter\n";
        } else if ("login" == message) {
            bool valid = true;
            std::string user = "", pass = "";

            std::cout << "username= ";
            std::getline(std::cin >> std::ws, user);
            if (!isalpnum(user)) valid = false;

            std::cout << "password= ";
            std::getline(std::cin >> std::ws, pass);
            if (!isalpnum(pass)) valid = false;

            if (valid)
                login_user(user, pass);
            else
                std::cout << "ERROR - wrong parameter\n";
        } else if ("enter_library" == message) {
            access_library(current_cookie);
        } else if ("get_books" == message) {
            get_books(current_token);
        } else if ("get_book" == message) {
            std::string id = "";
            std::cout << "id= ";
            std::cin>>id;
            if (atoi(id.c_str()) != 0)
                get_book(current_token, id);
            else
                std::cout << "ERROR - wrong parameter\n";
        } else if ("add_book" == message) {
            bool valid = true;
            std::string title = "", author = "", genre = "", publisher = "", page_count = "";

            std::cout << "title= ";
            std::getline(std::cin >> std::ws, title);
            if (!isalpnumspc(title)) valid = false;

            std::cout << "author= ";
            std::getline(std::cin, author);
            if (!isalpnumspc(author)) valid = false;

            std::cout << "genre= ";
            std::getline(std::cin, genre);
            if (!isalpnumspc(genre)) valid = false;

            std::cout << "publisher= ";
            std::getline(std::cin, publisher);
            if (!isalpnumspc(publisher)) valid = false;

            std::cout << "page_count= ";
            std::cin >> page_count;
            if (atoi(page_count.c_str()) != 0 && valid)
                upload_book(current_token, title, author, genre, publisher, atoi(page_count.c_str()));
            else
                std::cout << "ERROR - wrong parameter\n";
        } else if ("delete_book" == message) {
            std::string id = "";
            std::cout << "id= ";
            std::cin >> id;
            if (atoi(id.c_str()) != 0)
                delete_book(current_token, id);
            else
                std::cout << "ERROR - wrong parameter\n";
        } else if ("logout" == message) {
            logout_user(current_cookie);
        } else if ("exit" == message) {
            return 0;
        }
    }

    return 0;
}
