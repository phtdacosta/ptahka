#ifndef PROTOCOLUTILS_H
#define PROTOCOLUTILS_H

#include <stdint.h>

#define TINFIN_BUILD_VERSION "1.0.0"

// The function ExecuteRoute will receive an instance of the struct request_t
// and will actively find which stored route the resource address owns to,
// then, if found, the ExecuteRoute function will finally call the function
// associated with the route, otherwise will return an error response.
// The output of the function associated with the resource and route will be
// parsed by any function that can build an appropriate response accordingly
// with the protocol used by the whole project, HTTP in that case
typedef struct request_t {
    // method_t method;
    char *method;
    char *resource;
    // const char *http_version;
    struct header_t *headers;
    char *body;
} request_t;

typedef struct route_t {
    // method_t method;
    char *method;
    char *resource;
    char *(*function)(request_t *request);
} route_t;

typedef struct table_t {
    int size;
    route_t *area;    
} table_t;

// Built-in utility functions
char *strcat_(char *dest, const char *src);
char *charcat_ (char *dest, char ch);
void free_ (void *ptr);
uint8_t countdigits_ (uint64_t num);
char *uitoa_ (uint64_t num);
char **strseptillim_ (const char *str, char ch, char lim);
char **strsep_ (const char *str, char ch);
void freestrsep_(char **arr);
char *ptraftersep_ (char* str, char ch);
// Main functions
table_t *inittable (void);
void freetable (table_t *table);
int createroute (
    // method_t method,
    char *method,
    char *resource,
    char *(*function)(request_t *args),
    table_t *table);
int locateroute_ (char *resource, table_t *table);
char *executeroute (request_t* request, table_t *table);
request_t *parserequest (char *raw);
char *addheader (const char *field, const char *value, char *ptr);
char *parseresponse (
    char *status_code,
    char *content_type,
    // char *access_control_allow_origin,
    // char *allow,
    // char *connection,
    // char *date,
    // char *server,
    char *body,
    size_t buffer_size,
    char *res);
    
#endif