# Socket Programming (Client-Server Model)

A simple Client-Server program, where a client requests for the name of the city and state by sending a postal code. Server fetches the message i.e. postal code and looks it up in a simple database implementation (iteratively) that holds data present in the cityzip.csv file. It sends back the name of the city and state if a match is found and closes the socket/connection, otherwise it sends a "Match Not Found" message back to the client.

Commands to Compile:

```
rm server client
gcc -o server server.c
gcc -o client client.c
```

Commands to Run:

```
./server {portnumber}
./client {hostname} {postnumber}
```


