#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"

int main (int argc, char *argv[])
{
    int socket_desc;
    struct sockaddr_in server_addr;
    struct msg the_message; 
    
    int externalIndex = atoi(argv[1]); 
    float externalTemp = atof(argv[2]); 

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");
    printf("--------------------------------------------------------\n\n");

    // Send initial temperature
    the_message = prepare_message(externalIndex, externalTemp, 0);
    send(socket_desc, (const void *)&the_message, sizeof(the_message), 0);

    int iteration = 0;
    while (1) {
        // Receive updated central temperature
        if (recv(socket_desc, (void *)&the_message, sizeof(the_message), 0) < 0) {
            printf("Error while receiving server's msg\n");
            return -1;
        }

        iteration++;

        // Check for done flag ★
        if (the_message.Done == 1) {
            printf("[CLIENT %d] Stabilized after %d iterations. Final ext=%.3f central=%.3f\n",
                   externalIndex, iteration, externalTemp, the_message.T);
            break;
        }

        // Update external temperature ★
        externalTemp = (3 * externalTemp + 2 * the_message.T) / 5.0f;

        // Send updated temperature to server ★
        the_message = prepare_message(externalIndex, externalTemp, 0);
        send(socket_desc, (const void *)&the_message, sizeof(the_message), 0);
    }

    close(socket_desc);
    return 0;
}
