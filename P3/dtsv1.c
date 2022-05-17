#include "BXP/bxp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#define UNUSED __attribute__((unused))

#define PORT 19999
#define SERVICE "DTS"
#define USAGE "./dtsv1"

int main(UNUSED int argc, UNUSED char *argv[]) {
    BXPEndpoint sender;
    char *query = (char *)malloc(BUFSIZ);
    char *resp = (char *)malloc(BUFSIZ);
    char *cmd, *rest;
    unsigned len;
    BXPService bxps;
    char *service;
    unsigned short port;

    service = SERVICE;
    port = PORT;
    assert(bxp_init(port, 1));
    bxps = bxp_offer(service);
    if (bxps == NULL) {
        fprintf(stderr, "Failure offering Echo service\n");
	    free(query);
	    free(resp);
        exit(EXIT_FAILURE);
    }

    while ((len = bxp_query(bxps, &sender, query, BUFSIZ)) > 0) {
        query[len] = '\0';
	cmd = query;
	rest = strchr(query, ':');
	*rest++ = '\0';
        if (strcmp(cmd, "ECHO") == 0) {
            resp[0] = '1';
            strcpy(resp+1, rest);
        } else if (strcmp(cmd, "SINK") == 0) {
            sprintf(resp, "1");
        } else {
            sprintf(resp, "0Illegal command %s", cmd);
        }
        bxp_response(bxps, &sender, resp, strlen(resp) + 1);
    }
    return 0;
}