#include "BXP/bxp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "ADTs/prioqueue.h"
#define UNUSED __attribute__((unused))

#define PORT 19999
#define SERVICE "DTS"
#define USAGE "./dtsv2"

void *recieve();

int extractWords(char *buf, char *sep, char *words[]) {
    int i;
    char *p;

    for (p = strtok(buf, sep), i = 0; p != NULL; p = strtok(NULL, sep),i++)
        words[i] = p;
    words[i] = NULL;
    return i;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    pthread_t reciever;
    pthread_create(&reciever, NULL, recieve, NULL);

    pthread_join(reciever, NULL);

    return 0;
}

void *recieve(){
    const PrioQueue *pq = PrioQueue_create(int (*cmp)(void*, void*),
                                  void (*freePrio)(void *prio),
                                  void (*freeValue)(void *value)
                                 );
    BXPEndpoint sender;
    char *query = (char *)malloc(BUFSIZ);
    char *resp = (char *)malloc(BUFSIZ);
    unsigned len;
    BXPService bxps;
    char *service;
    unsigned short port;
    char *w[25];
    int N;

    service = SERVICE;
    port = PORT;
    
    //Initialize BXP system - bind to ‘port’ if non-zero
    assert(bxp_init(port, 1));
    
    //Offer service named `service' in this process
    bxps = bxp_offer(service);
    
    if (bxps == NULL) {
        fprintf(stderr, "Failure offering Echo service\n");
        free(query);
        free(resp);
        exit(EXIT_FAILURE);
    }
    //obtain the next query message from `bxps' - blocks until message available
    while ((len = bxp_query(bxps, &sender, query, BUFSIZ)) > 0) {
        query[len] = '\0';
        N = extractWords(query, "|", w);
        if(strcmp(w[0], "OneShot") == 0){
            if(N == 7)
                sprintf(resp, "1%s", query);
        }
        else if(strcmp(w[0], "Repeat") == 0){
            if(N == 7)
                sprintf(resp, "1%s", query);
        }
        else if(strcmp(w[0], "Cancel") == 0){
            if(N == 2)
                sprintf(resp, "1%s", query);
        }
        else{
            sprintf(resp, "0%s", query);
        }
        bxp_response(bxps, &sender, resp, strlen(resp) + 1);
    }
    pthread_exit(NULL);
}