# chat
A chat client and server (partial)

Use make to make the executables server.x and client.x.

Usage:
1. Start the server with:
server.x -p [port number]
The port you choose to use should be high (try between 10000 and 65535).

2. Start any number of clients (different terminal) with:
client.x -a [address] -p [port number]
Address needs to be the ip address of the machine hostingthe server (127.0.0.1 for the same machine). Same port number as well.

3. Client needs to choose a name first, and then can send messages to the server by simply typing them in.

4. Server will then send the message (and who sent it) to all conected clients.

More features will be added soon.
