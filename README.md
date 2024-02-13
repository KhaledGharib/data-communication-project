# Data Communication Project

## Overview

This project implements a simple client-server communication system, demonstrating basic concepts like socket programming and threading.


## Installation

**Prerequisites:**

* C compiler (e.g., GCC)
* POSIX compliant operating system (e.g., macOS, Linux)

**Steps:**

1. Clone the repository: `git clone https://github.com/KhaledGharib/data-communication-project.git`
2. Navigate to the project directory: `cd data-communication-project`

## Usage

**Server:**

1. Compile the server: `gcc -o server server.c -lpthread`
2. Run the server: `./server 80` (start the server on port 80)

**Client:**

1. Compile the client: `gcc -o client client.c -lpthread`
2. Run the client: `./client localhost 80 xed` (connect to the server on localhost:port 80, username "xed")

## Key Features

* Demonstrates socket programming using TCP
* Implements multi-threading for concurrent client connections
* Handles basic data transmission and reception

## Contributions

Contributions are welcome! Please open a pull request with your changes.

