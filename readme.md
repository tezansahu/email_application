# Email Application

An application that implements a simplified version of PoP3 email protocol between a client and a server using socket programming.

## Getting started...

1. Clone the repository using `git clone https://github.com/tezansahu/email_application.git`.

2. Move into the repository using `cd email_application`.

3. To start the server, type:

```console
g++ -std=c++0x -c SimpleEmailServer.cpp
g++ -std=c++0x -o server SimpleEmailServer.o
./server <port-number> <password-file-name>
```

4. Open another terminal (or another computer) and start the client by typing:

```console
g++ -std=c++0x -c SimpleEmailClient.cpp
g++ -std=c++0x -o client SimpleClientServer.o
./client <server-IP-Adderss>:<port-number> <username> <password>
```

