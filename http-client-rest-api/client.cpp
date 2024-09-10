#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <string>
#include <cstring>
#include <iostream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

char HOST_IP[] = "34.246.184.49";
const int PORT = 8080;
#define MAX_NAME_LEN 60

bool is_number(const string &str) {

    if (str.empty()) {
        return false;
    }

    for (char ch : str) {
        if (!isdigit(ch)) {
            return false;
        }
    }

    return true;
}

void display_error(char *response)
{
    char *ptr = strstr(response, "error") + 8;
    if (!ptr) {
        cout << "EROARE";
        return;
    }
    char *end_quote = strchr(ptr, '\"');

    if (end_quote != NULL) {
        *end_quote = '\0';
    } else {
        cout << "EROARE";
        return;
    }

    cout << "EROARE: " << ptr << "\n";
}

bool verify_code(char *response)
{
    const char *http_version = strstr(response, "HTTP/");
    if (!http_version) {
        cout << "EROARE";
        return false;
    }

    char *response_copy = new char[strlen(response) + 1];
    strcpy(response_copy, response);

    char *token = strtok(response_copy, " ");
    if (!token) {
        cout << "EROARE";
        return false;
    }

    token = strtok(NULL, " ");

    if (!token) {
        cout << "EROARE";
        return false;
    }


    if (is_number(token)) { // verific daca numarul este int
        int status_code = -1;
        status_code = stoi(token);

        if (200 <= status_code && status_code < 300)
            return true;
        else {
            display_error(response);
            return false;
        }
    } else {
        display_error(response);
        return false;
    }

}

void register_command()
{
    char *message;
    char *response;
    string username, password;
    cout << "username=";
    getline(cin, username);
    cout << "password=";
    getline(cin, password);

    json j;
    j["username"] = username;
    j["password"] = password;
    string json_string = j.dump();
    char *json_pointer = new char[json_string.length() + 1];
    strcpy(json_pointer, json_string.c_str());

    char *body_data[] = { json_pointer };

    char url[] = "/api/v1/tema/auth/register";
    char content_type[] = "application/json";
    message = compute_post_request(HOST_IP, url, content_type, body_data, 1, NULL, 0, NULL);
    int sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    close(sockfd);

    if (!verify_code(response))
        return;

    cout << "SUCCES: Utilizator înregistrat cu succes!\n";
}

char *login_command(bool isActive, char *cookie)
{
    char *message;
    char *response;
    string username, password;
    cout << "username=";
    getline(cin, username);
    cout << "password=";
    getline(cin, password);

    if (isActive) { // inseamna ca este deja activ
        cout << "EROARE: Utilizatorul este deja logat\n";
        return cookie;
    }

    json j;
    j["username"] = username;
    j["password"] = password;
    string json_string = j.dump();
    char *json_pointer = new char[json_string.length() + 1];
    strcpy(json_pointer, json_string.c_str());

    char *body_data[] = { json_pointer };

    char url[] = "/api/v1/tema/auth/login";
    char content_type[] = "application/json";
    message = compute_post_request(HOST_IP, url, content_type, body_data, 1, NULL, 0, NULL);
    int sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    close(sockfd);

    if (!verify_code(response))
        return NULL;

    char *ptr = strstr(response, "Set-Cookie:");
    cookie = strtok(ptr, " ");
    cookie = strtok(NULL, ";");

    cout << "SUCCES: Utilizatorul a fost logat cu succes\n";
    return cookie;
}

char *enter_library_command(char *cookie)
{
    char *message;
    char *response;
    char url[] = "/api/v1/tema/library/access";
    message = compute_get_request(HOST_IP, url, NULL, &cookie, 1, NULL);
    int sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    close(sockfd);

    char *token_start = strstr(response, "token");
    token_start += strlen("token") + 3;
    char *token_end = strstr(token_start, "\"}");
    if (token_end != NULL) {
        *token_end = '\0';
    }

    if (!verify_code(response))
        return NULL;

    cout << "SUCCES: Utilizatorul are acces la biblioteca\n";

    return token_start;
}

void get_book_command(char *token, bool isActive)
{
    char *message;
    char *response;
    cout << "id=";
    string stringId;
    getline(cin, stringId);
    if (!isActive) { // daca nu este logat
        cout << "EROARE: Utilizatorul nu este logat\n";
        return;
    }

    if (!token) { // inseamna ca deja a intrat in librarie
        cout << "EROARE: Utilizatorul nu are acces la biblioteca\n";
        return;
    }

    if (!is_number(stringId)) { // inseamna ca nu sunt doar cifre
        cout << "Is not number\n";
        return;
    }

    char url[255];
    snprintf(url, sizeof(url), "%s%s", "/api/v1/tema/library/books/", stringId.c_str());

    message = compute_get_request(HOST_IP, url, NULL, NULL, 0, token); // creez un pachet de tip get
    int sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    close(sockfd);

    char *ptr = strstr(response, "{");
    cout << ptr << "\n";
}

void get_books_command(char *token)
{
    char *message;
    char *response;
    char url[] = "/api/v1/tema/library/books";
    message = compute_get_request(HOST_IP, url, NULL, NULL, 0, token);  // creez un pachet de tip get
    int sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    close(sockfd);

    char *ptr = strstr(response, "id");
    if (!ptr) {
        cout << "[]\n";
        return;
    } else {
        ptr -= 3;
    }

    if (!verify_code(response))
        return;

    cout << ptr << "\n";
}

void add_book_command(char *token, bool isActive)
{
    string title, author, genre, publisher, page_count;
    char *message;
    char *response;

    cout << "title=";
    getline(cin, title);
    cout << "author=";
    getline(cin, author);
    cout << "genre=";
    getline(cin, genre);
    cout << "publisher=";
    getline(cin, publisher);
    cout << "page_count=";
    getline(cin, page_count);
    if (!isActive) { // daca nu este logat
        cout << "EROARE: Utilizatorul nu este logat\n";
        return;
    }

    if (!token) { // inseamna ca deja a intrat in librarie
        cout << "EROARE: Utilizatorul nu are acces la biblioteca\n";
        return;
    }

    json j;
    j["title"] = title;
    j["author"] = author;
    j["genre"] = genre;
    j["publisher"] = publisher;
    j["page_count"] = page_count;

    string json_string = j.dump();
    char *json_pointer = new char[json_string.length() + 1];
    strcpy(json_pointer, json_string.c_str());

    char *body_data[] = { json_pointer };
    char url[] = "/api/v1/tema/library/books";
    char content_type[] = "application/json";
    message = compute_post_request(HOST_IP, url, content_type, body_data, 1, NULL, 0, token);
    int sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    close(sockfd);

    if (!verify_code(response))
        return;

    cout << "Cartea a fost adaugata cu succes!\n";
}

void delete_book_command(char *token, char *cookie, bool isActive)
{
    char *message;
    char *response;
    cout << "id=";
    string stringId;
    getline(cin, stringId);

    if (!isActive) { // daca nu este logat
        cout << "EROARE: Utilizatorul nu este logat\n";
        return;
    }

    if (!token) { // inseamna ca deja a intrat in librarie
        cout << "EROARE: Utilizatorul nu are acces la biblioteca\n";
        return;
    }

    if (!is_number(stringId)) { // inseamna ca nu sunt doar cifre
        cout << "EROARE: Nu este numar\n";
        return;
    }

    char url[255];
    snprintf(url, sizeof(url), "%s%s", "/api/v1/tema/library/books/", stringId.c_str());

    message = delete_request(HOST_IP, url, NULL, NULL, 0, token); // creez un pachet de tip get
    int sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    close(sockfd);

    if (!verify_code(response))
        return;

    cout << "Cartea cu id " << stringId << " a fost stearsa cu succes!\n";
}

bool logout_command(char *cookie)
{
    char *message;
    char *response;
    char url[] = "/api/v1/tema/auth/logout";
    message = compute_get_request(HOST_IP, url, NULL, &cookie, 1, NULL); // creez un pachet de tip get
    int sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    close(sockfd);

    if (!verify_code(response))
        return false;

    return true;
}

int main(int argc, char *argv[])
{
    bool isActive = false;  // prima oara il setez ca nefiind activ
    char *cookie;
    char *token;

    while (1) {
        string command;
        getline(cin, command);

        if (!command.compare("register")) {
            register_command();
        } else if (!command.compare("login")) {
            cookie = login_command(isActive, cookie);
            if (cookie) {
                isActive = true; // inseamna ca a putut fi logat
            }
        } else if (!command.compare("enter_library")) {
            if (!isActive) { // daca nu este logat
                cout << "EROARE: Utilizatorul nu este logat\n";
                continue;
            }

            if (token) { // inseamna ca deja a intrat in librarie
                cout << "EROARE: Utilizatorul a accesat deja libraria\n";
                continue;
            }

            token = enter_library_command(cookie);

        } else if (!command.compare("get_book")) {
            // verific daca este logat si daca are acces la biblioteca
            get_book_command(token, isActive);
        } else if (!command.compare("get_books")) {
            // verific daca este logat si daca are acces la biblioteca
            if (!isActive) { // daca nu este logat
                cout << "EROARE: Utilizatorul nu este logat\n";
                continue;
            }

            if (!token) { // inseamna ca deja a intrat in librarie
                cout << "EROARE: Utilizatorul nu are acces la biblioteca\n";
                continue;
            }

            get_books_command(token);
        } else if (!command.compare("add_book")) {
            // verific daca este logat si daca are acces la biblioteca
            add_book_command(token, isActive);
        } else if (!command.compare("delete_book")) {
            // verific daca este logat si daca are acces la biblioteca
            delete_book_command(token, cookie, isActive);
        } else if (!command.compare("logout")) {
            if (isActive) {  // inseamna ca este logat
                if (logout_command(cookie)) { // daca s a putut deloga
                    cookie = NULL;
                    token = NULL;
                    isActive = false;
                    cout << "SUCCES: Utilizatorul s-a delogat cu succes!\n";
                }
            } else {
                cout << "EROARE: Utilizatorul nu este logat.\n";
            }

        } else if (!command.compare("exit")) {
            break;
        } else {
            cout << "Command not found!\n";
        }
    }

    return 0;
}
