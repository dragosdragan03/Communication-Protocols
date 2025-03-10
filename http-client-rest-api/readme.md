# HTTP Client REST API

## Overview

This project implements a command-line HTTP client that communicates with a RESTful API through HTTP messages for managing a digital library system. The client handles the construction and parsing of HTTP requests and responses, allowing users to register, log in, browse the library, and manage books through a simple text-based interface. All operations are performed by sending properly formatted HTTP GET, POST, and DELETE requests and processing the corresponding responses.

## Key Features

- **User Authentication**: Register new accounts and log in with existing credentials
- **Library Access Management**: Secure access to the library's resources using tokens
- **Book Operations**: View, add, and delete books in the digital library
- **JSON-based Communication**: API requests and responses using JSON format
- **Error Handling**: Comprehensive error checking and user-friendly messages

### Available Commands

| Command | Description |
|---------|-------------|
| `register` | Create a new user account |
| `login` | Authenticate with username and password |
| `enter_library` | Access the library with your authenticated session |
| `get_books` | View all books in the library |
| `get_book` | View details of a specific book by ID |
| `add_book` | Add a new book to the library |
| `delete_book` | Remove a book from the library |
| `logout` | End the current session |
| `exit` | Quit the application |

### Command Flow

1. Start by registering an account (`register`) or logging in (`login`)
2. Access the library (`enter_library`) to get authorization for book operations
3. Perform book operations (`get_books`, `get_book`, `add_book`, `delete_book`)
4. Log out when finished (`logout`)

## Implementation Details

### Core Functionality

- **HTTP Request Management**: Custom implementation of GET, POST, and DELETE HTTP methods
- **Session Management**: Maintaining user sessions with cookies and tokens
- **Input Validation**: Ensuring proper format for IDs and other inputs
- **Error Reporting**: Clear error messages for debugging and user guidance

### Main Components

#### Utility Functions
- `display_error()`: Formats and displays server error messages
- `verify_code()`: Interprets HTTP status codes for error handling

#### User Authentication
- `register_command()`: Handles new user registration
- `login_command()`: Manages user login and session cookies
- `logout_command()`: Terminates user sessions

#### Library Access
- `enter_library_command()`: Obtains library access token

#### Book Management
- `get_books_command()`: Retrieves all books in the library
- `get_book_command()`: Fetches details for a specific book
- `add_book_command()`: Creates new book entries
- `delete_book_command()`: Removes books from the library

## API Endpoints

The client interacts with the following endpoints:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/v1/tema/auth/register` | POST | Register a new user |
| `/api/v1/tema/auth/login` | POST | Authenticate user |
| `/api/v1/tema/auth/logout` | GET | End user session |
| `/api/v1/tema/library/access` | GET | Get library access token |
| `/api/v1/tema/library/books` | GET | Get all books |
| `/api/v1/tema/library/books` | POST | Add a new book |
| `/api/v1/tema/library/books/{id}` | GET | Get book by ID |
| `/api/v1/tema/library/books/{id}` | DELETE | Delete book by ID |

## Example Workflow

```
$ ./client
register
username=testuser
password=testpass
SUCCES: Utilizator înregistrat cu succes!

login
username=testuser
password=testpass
SUCCES: Utilizatorul a fost logat cu succes

enter_library
SUCCES: Utilizatorul are acces la biblioteca

get_books
[{"id":1234,"title":"Test Book"}]

add_book
title=My New Book
author=John Doe
genre=Mystery
publisher=Test Publishing
page_count=250
Cartea a fost adaugata cu succes!

logout
SUCCES: Utilizatorul s-a delogat cu succes!

exit
```


## Prerequisites

- C++ compiler with C++11 support
- [nlohmann/json](https://github.com/nlohmann/json) library for JSON handling

## Usage

### Compiling
```bash
g++ -o client client.cpp requests.cpp helpers.cpp -std=c++11
```

### Running
```bash
./client
```


## License

Copyright © 2023-2024 Dragan Dragos Ovidiu