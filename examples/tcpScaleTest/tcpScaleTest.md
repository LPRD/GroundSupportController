## Build
build this example using either the g++ commands listed at the top of the tcpSocketTest.cpp file
OR
use the included Makefile
'make' will build the executable

## Run
1. start the tcpSocketTest.js server in the GroundSupportServer chat example using the command 
'node tcpSocketTest.js'
when in the chat directory (clone the GroundSupportServer repo if needed).
This script only runs the TCP Socket Server.

Alternatively, use the commands
'npm i'
'npm start'
in the same directoy to run index.js
The index.js the main script in the GroundSupportServer chat example. It uses the Socket.io library to create a Socket.io server (not a TCP Socket server that tcpSocketTest.cpp can connect with). When accessing the 
Socket.io server through a browser window, a basic chat GUI will appear. 
index.js also includes the TCP Socket server, so this script is ok to run for testing tcpSocketTest.cpp


2. Start the tcpSocketTest.cpp client in this repo using
'make run'
This will build the executable if any dependencies are out of date and then run the executable.

## More information about the tcpSocketTest example
This example demonstrates the transfer of multiple data types (strings, integers, and floats) to and from a C++ program and a javascript program. A  TCP socket server is hosted in the javascript program and the client is ran from the C++ program. A basic command word interface is set up and used. The code in this example can be expanded upon to allow for sensor data to be sent to the javascript program.


