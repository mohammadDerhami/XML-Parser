#XML Parser

- [XML Parser](#XML_Parser)
- [Description](#Description)
- [Features](#Features)
- [Installation](#installation)


## Description

**XML Parser** This project is a socket server application that process XML data received from clients and stores it in a database.

## Features
- Reads the first 15 bytes as the size of incoming data.
- Processes the received XML data and validates its structure.
- Stores the processed data in a database.
- Allows retrieval of data in XML format from the database.
- Supports multi-client communication, enabling reception of data multiple times.

## Installation

Installation of `XML Parser` is based on build system.

1. First step is to clone the project in desired directory:

	```bash
	#Your favorite directory
	$ git clone https://github.com/mohammadDerhami/XML-Parser.git
	```

2. Then for create build directory:

	```bash
	#Creates build directory.
	$ mkdir build

	$ cd build
	```

3. Run cmake to configure the project:

	```bash
	$ cmake..
	```

4. Compile the project:

	```bash
	#Compile the project.
	$ make
	```
5. Run the project:

	```bash
	#Run the project
	$ ./dbm -f <config file path> : use the specified configuration file(../src/config.json).
	$ ./dbm -h : display help message.
	$ ./dbm -v : display the current version of the application.
	```
6. Connecting with client:

	```bash
	telnet <ip><port> -> default: telnet localhost 8080
