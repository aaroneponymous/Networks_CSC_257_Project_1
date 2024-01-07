#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Standard Symbolic Constant and Types
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_LINE_LENGTH 1024
#define INITIAL_CAPACITY 10

// Data Structure to Maintain Database
typedef struct {
    char state[50];
    char city[50];
    char postal[10];
    double latitude;
    double longitude;
    int num_poi;
} Record;

// Wrapper Error Function
void error(const char *msg) {
    perror(msg);
    exit(1);
}

Record *create_database(const char *filename, int *size) {

    FILE *file = fopen(filename, "r"); // Argument 2 "r" : Defines Read Mode
    if (file == NULL) {
        error("Unable to Open File");
    }

    Record *records = NULL;
    int capacity = INITIAL_CAPACITY;
    *size = 0;

    records = (Record *) malloc(capacity * sizeof(Record));
    if (records == NULL) {
        error("Memory Allocation for Database Set Up Failed");
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (*size >= capacity) {
            capacity *= 2;
            records = (Record *) realloc(records, capacity * sizeof(Record));
            if (records == NULL) {
                error("Memory Allocation for Increasing Capacity of Database Failed");
            }
        }

        Record *record = &records[*size];

        // Parse the line and populate the record fields
        sscanf(line, "%[^,],%[^,],%[^,],%lf,%lf,%d", 
              record->state, record->city, record->postal,
              &record->latitude, &record->longitude, &record->num_poi);

        (*size)++; 
    }

    fclose(file);
    return records;
}

int lookup_postal(const Record *records, int size, const char *postal, char *city, char *state) {
    for (int i = 0; i < size; i++) {
        if (strcmp(records[i].postal, postal) == 0) {
            strcpy(city, records[i].city);
            strcpy(state, records[i].state);
            return 1;  // Match found
        }
    }
    return 0;  // Match not found
}



// int main() {

//     int size;
//     Record *records = create_database("cityzip.csv", &size);

//     char postal[10];
//     char city[50];
//     char state[50];
//     strcpy(postal, "14620");
//     printf("%s", postal);

//     lookup_postal(records, size, postal, city, state);
//     printf("%s", city);
//     printf("%s", state);

//     return 0;
// }

int main(int argc, char *argv[]) {

    int size;
    Record *records = create_database("cityzip.csv", &size);

    char postal[10];
    char city[50];
    char state[50];

    // // For example, printing all records
    // for (int i = 0; i < size; i++) {
    //     printf("%s, %s, %s, %lf, %lf, %d\n",
    //            records[i].state, records[i].city, records[i].postal,
    //            records[i].latitude, records[i].longitude, records[i].num_poi);
    // }


    // sockfd and newsockfd: 
    // File Descriptors (Array subscripts into the file descriptor table)
    // Store the values returned by the socket sys call and the accept sys call
    // portno : port for the server to accept the connections on
    // clilen : stores the size of the address of the client (needed for the accept system call)
    // n      : return value for the read() and write() calls i.e. it contains the number of characters read or written

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) error("ERROR on accept");
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    printf("Requested Postal Code: %s\n", buffer);

    // Remove the trailing newline character from the buffer
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    strcpy(postal, buffer);
    
    if (!lookup_postal(records, size, postal, city, state)) {
        printf("Requested Postal Code: %s\n", postal);
        n = write(newsockfd, "Postal Code not Found in Database!", strlen("Postal Code not Found in Database!") + 1);  // +1 for the null terminator
        if (n < 0) error("ERROR writing to socket");
    } else {
        char formattedString[256];
        sprintf(formattedString, "City: %s\nState: %s", city, state);
        printf("Match Found:\n%s\n", formattedString);
        n = write(newsockfd, formattedString, strlen(formattedString) + 1);  // +1 for the null terminator
        if (n < 0) error("ERROR writing to socket");
    }

    free(records);

    close(newsockfd);
    close(sockfd);
    return 0;
}




