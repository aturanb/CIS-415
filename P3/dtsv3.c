#include "BXP/bxp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include "ADTs/heapprioqueue.h"
#include "ADTs/queue.h"
#include "ADTs/hashmap.h"
#define UNUSED __attribute__((unused))

#define PORT 19999
#define SERVICE "DTS"
#define USAGE "./dtsv3"

const PrioQueue *q = NULL;
const Map *map = NULL;

int globid = 0;

void *receive();

typedef struct event {
    int oneshot;
    int repeat;
    int cancelled;
    unsigned long numRepeats;
    unsigned long interval;
    unsigned long id;
    unsigned long clid;
    unsigned int port;
    char *host; 
    char *service;
} Event;

int hash(void *key, int N){ 
    return (((unsigned long)key)%N);
}

int compare(void *p1, void *p2){
    struct timeval *s1 = (struct timeval *)p1;
    struct timeval *s2 = (struct timeval *)p2;
    int c;
    if((s1->tv_sec) < (s2->tv_sec))
        c = -1;
    else if((s1->tv_sec) > (s2->tv_sec))
        c = 1;
    else{
        if((s1->tv_usec) < (s2->tv_usec))
            c = -1;
        else if((s1->tv_usec) > (s2->tv_usec))
            c = 1;
        else
            c = 0;
    }
    return c;

}

int extractWords(char *buf, char *sep, char *words[]) {
    int i;
    char *p;

    for (p = strtok(buf, sep), i = 0; p != NULL; p = strtok(NULL, sep),i++)
        words[i] = p;
    words[i] = NULL;
    return i;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    q = HeapPrioQueue(compare, doNothing, doNothing);
    map = HashMap(100, 0.f, hash, compare, NULL, NULL);
    
    pthread_t receiver;
    pthread_create(&receiver, NULL, receive, NULL);
    pthread_join(receiver, NULL);

    return 0;
}

int oneshot_insert(char *words[]){
    Event *eve = (Event *)malloc(sizeof(Event));
    struct timeval t = malloc(sizeof(struct timeval)); 
    scanf(words[2], "%lu", &(t->tv_sec));
    scanf(words[3], "%lu", &(t->tv_usec));
    eve->oneshot = 1;
    eve->repeat = 0;
    eve->cancelled = 0;
    eve->numRepeats = 0;
    eve->id = globid;
    scanf(words[1], "%lu", &(eve->clid));
    scanf(words[6], "%u", &(eve->port));
    scanf(words[4], "%lu", &(eve->host));
    eve->host = strdup(words[4]); 
    eve->service = strdup(words[5]);
    //inset pq
    q->pq_insert(q, timeval, (void *)eve);
    //insert map
    map->putUnique(map, (void *)id, (void *)eve);

    globid++;  
}

int repeat_insert(char *words[]){
    Event *eve = (Event *)malloc(sizeof(Event));
    struct timeval t = malloc(sizeof(struct timeval));
    //convert words[2](msecs) to sec and usec
    scanf(//sec, "%lu", &(t->tv_sec));
    scanf(//usec, "%lu", &(t->tv_usec)); 
    
    eve->oneshot = 0;
    eve->repeat = 1;
    eve->cancelled = 0;
    scanf(words[3], "%lu", &(eve->numRepeats));
    eve->id = globid;
    scanf(words[1], "%lu", &(eve->clid));
    scanf(words[6], "%u", &(eve->port));
    scanf(words[4], "%lu", &(eve->host));
    eve->host = strdup(words[4]); 
    eve->service = strdup(words[5]);
    //inset pq
    q->pq_insert(q, timeval, (void *)eve);
    //insert map
    map->putUnique(map, (void *)id, (void *)eve);
    globid++;  
}

void *receive(){
    
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

void *timer(void *args) {
    unsigned long long counter = 0;
    struct timeval now;

    for(;;) {
        usleep(USECS);
        gettimeofday(&now, NULL);
        counter++;
        if((counter % 100) == 0)
            printf("Another second has passed\n");
        
    }
}