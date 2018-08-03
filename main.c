#include "src/protocolutils.h"
#include "src/socketutils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

char *Homepage (request_t *req) {
    // JSON response example
    // char *r = parseresponse("200 OK", "application/json", "{\"sector\":\"Solar System\",\"planet\":\"Mars\",\"site\":\"Olympus Mons\"}", r);
    char *r;
    r = parseresponse(
                    "200 OK",
                    "text/html",
                    "<html><h1>It is alive!</h1></html>",
                    512,
                    r);
    return r;
}

char *DefaultRoute (request_t *req) {
    char *r;
    r = parseresponse(
                    "404 Not Found",
                    "text/html",
                    "<html><h1>404 Not Found</h1><br><b>Ptahka</b> did not find the requested resource.</html>",
                    512,
                    r);
    return r;
}

int main (int argc,char *argv[]) {

    table_t *table = inittable();
    createroute("GET", "/home", &Homepage, table);

    printf("127.0.0.1:7070\n");

    initservice(&parserequest,
                table,
                &executeroute,
                &DefaultRoute);
    // It does all the job freeing the routing table contents
    freetable(table);
    return 0;
}