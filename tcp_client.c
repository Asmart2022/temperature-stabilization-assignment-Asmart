#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <external_index> <initial_temperature>\n", argv[0]);
        return 1;
    }

    int index = atoi(argv[1]);
    float externalTemp = atof(argv[2]);
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) { perror("socket"); return 1; }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        { perror("connect"); return 1; }

    struct msg msgOut = prepare_message(index, externalTemp, 0);
    send(socket_desc, &msgOut, sizeof(msgOut), 0);

    struct msg msgIn;
    int iteration = 0;

    while (1) {
        recv(socket_desc, &msgIn, sizeof(msgIn), 0);
        iteration++;

        if (msgIn.Done) {
            printf("[CLIENT %d] Stabilized after %d iterations. Final ext=%.3f central=%.3f\n",
                   index, iteration, externalTemp, msgIn.T);
            break;
        }

        externalTemp = (3 * externalTemp + 2 * msgIn.T) / 5.0f;
        msgOut = prepare_message(index, externalTemp, 0);
        send(socket_desc, &msgOut, sizeof(msgOut), 0);
    }

    close(socket_desc);
    return 0;
}
