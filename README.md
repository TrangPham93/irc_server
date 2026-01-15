# irc-server

> A functional Internet Relay Chat (IRC) server written in **C++**.  
> This project implements the core IRC protocol (RFC 2119 protocol), in which irc server allows multiple IRC clients (**irssi** is used as reference client) to connect, communicate, and manage channels in real time, without forking or blocking calls.

---

## Features
The server accept multiple authenticated clients to connect to the server through available port.
Client can:
- JOIN a channel
- INVITE a client to channel
- Change or view TOPIC
- Set MODE for the channel:
  - +k : set key for channel
  - +t : limit the topic change to only channel operator
  - +i : limit the channel to only invited clients
  - +o : add channel operator priviledge to certain client
  - +l : limit the number of clients can freely join the channel without invitation
- KICK a client
- PART a channel
- QUIT the server    

---

## Usage

## Installment
### 1. Clone the Repository
```bash
git clone git@github.com:TrangPham93/irc_server.git irc_server
cd irc_server
```
### 2. Compile the Project
```bash
make
```

### 3. Run the project
```bash
./ircserv <port> <password>
```
### 4. Examples


## Contributors
Karoliina - [Github](https://github.com/kaloliina)

Anton - [Github](https://github.com/Kiiskii)
