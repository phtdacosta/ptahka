#include "protocolutils.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TINFIN_BUILD_VERSION "1.0.0"

char *strcat_(char *dest, const char *src) {
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = '\0';
    return dest;
}

char *charcat_ (char *dest, char ch) {
    *(dest++) = ch;
    *dest = '\0';
    return dest;
}

void free_ (void *ptr) {
    free(ptr);
    ptr = NULL;
}

/* static inline */ uint8_t countdigits_ (uint64_t num) {
    uint8_t len = 1;
    // if (num < 0) num *= -1;
    while (num /= 10) len++;
    return len;
}

// REQUIRES free() AFTER USE!
char *uitoa_ (uint64_t num) {
    uint8_t len = countdigits_(num);
    // Enough memory for the characters plus null terminator
    char *arr = (char*)malloc(sizeof(char) * (len + 1));
    // Using the length to place the null terminator before operating over it
    arr[len] = '\0';
    do {
        // The length is going to decrease systematically
        len--;
        // Adds up the character 0 to the modulus calculation integer and typecasts it to ensure the char conversion  
        arr[len] = (char)('0' + (num - 10 * (num / 10)));
        // Slice out the concatenated numeral
        num /= 10;
    } while (len > 0);

    return arr;
}

// Just a minor change compared to the "strsep_" function
// Parses the char pointer unTIL a given deLIMiter character
char **strseptillim_ (const char *str, char ch, char lim) {
    int len = 0;
    int pos = 0;
    char **arr = (char **)malloc(sizeof(char**));
    if (arr == NULL)
        return NULL;
    for (;;) {
        if (*str == ch || *str == lim) {
            char *aux_ = (char *)malloc(sizeof(char) * (len + 1));
            char **aux = (char **)realloc(arr, sizeof(char**) * (pos + 2));
            if (aux == NULL || aux_ == NULL) {
                free(aux_);
                if (aux)
                    arr = aux;
                while (pos-- > 0)
                    free(arr[pos]);
                free(arr);
                return NULL;
            }
            arr = aux;
            memcpy(aux_, str - len, len);
            aux_[len] = '\0';
            arr[pos] = aux_;
            pos++;
            len = 0;
            if (*str == lim)
                break;
        } else {
            len++;
        }
        str++;
    }
    arr[pos] = NULL;
    return arr;
}

// Functions used to parse requests
// NEED MULTIPLE free() CALLS! Or the special one written right after that one
char **strsep_ (const char *str, char ch) {
    char **arr = strseptillim_(str, ch, '\0');
    return arr;
}

void freestrsep_(char **arr) {
    int i = 0;
    // Free the array elements 
    for (; arr[i] != NULL; i++) {
        free(arr[i]);
        arr[i] = NULL;
    }
    // Clears the array itself 
    free(arr);
    arr = NULL;
}

// Subtract the pointer given as argument to this function from the pointer
// returned by the function to get the length of the char array piece
char *ptraftersep_ (char* str, char ch) {
    int len = 0;
    while (*str) {
        if (*str == ch)
            return str++;
        str++;
    }
    return NULL;
}

table_t *inittable (void) {
    table_t *table = (table_t *)malloc(sizeof(struct table_t));
    if (table != NULL) {
        table->size = 0;
        table->area = NULL;
    }
    return table;
}

void freetable (table_t *table) {
    // Test for emptiness
    if (table->area != NULL) {
        int i = 0;
        for(; i < table->size; i++) {
            // (table->area + i)->method = NONE;
            (table->area + i)->method = NULL;
            (table->area + i)->resource = NULL;
            (table->area + i)->function = NULL;
        }
        table->size = 0;
        free(table->area);
        table->area = NULL;
    }
    free(table);
    table = NULL;
}

int createroute (
    // method_t method,
    char *method,
    char *resource,
    char *(*function)(request_t *args),
    table_t *table) {
    if (table->area == NULL)
        table->area = (route_t *)malloc(sizeof(struct route_t));
    else
        table->area = (route_t *)realloc(table->area, sizeof(struct route_t) * (table->size + 1));
    // The memory allocation failed if the area pointer still empty
    if (table->area == NULL)
        return 1;

    (table->area + table->size)->method = method;
    (table->area + table->size)->resource = resource;
    (table->area + table->size)->function = function;
    table->size++;
    return 0;
}

// This function can be greatly improved
// Using quick sort after every insertion and deletion
// And then using binary search over the sequential
int locateroute_ (char *resource, table_t *table) {
    int i = 0;
    for (; i < table->size; i++) {
        if (strcmp((table->area + i)->resource, resource) == 0) 
            return i;
    }
    return -1;
}

// This is the most important function of the whole library!
// It will run as the main engine of the route system,
// receiving or a raw char pointer request data,
// void ExecuteRoute (const char* req, const char *resource, table_t *table) {
// or the pre parsed request struct (which should be default)
char *executeroute (request_t* request, table_t *table) {
	int i = locateroute_(request->resource, table);
    // Default failure code
    if (i == -1)
        return NULL;
    else {
        route_t *route = (table->area + i);

        free(request); // Is it really needed here?
        return route->function(request);
    }
}

// NEED free() AFTER USE!
request_t *parserequest (char *raw) {
    // Basic HTTP protocol text addressing requested method, resource and protocol version
    char **common = strseptillim_(raw, ' ', '\r');
    // Further data can store unknown headers and a message body
    char *uncommon = ptraftersep_(raw, '\n');

    request_t *request = (request_t *)malloc(sizeof(request_t));
    // Further tests are needed to benchmark if it is worth to use the enum instead!
    // request->method = parsemethod(common[0]);
    request->method = common[0];
    request->resource = common[1];
    // request->http_version = common[2];

    // Need to build the function to parse additional request headers!
    // For instance, only 
    request->headers = NULL;
    // request->body = NULL;
    // Marked for future fixes/updates
    request->body = uncommon++;

    return request;
}

char *addheader (const char *field, const char *value, char *ptr) {
    ptr = strcat_(ptr, field);
    ptr = strcat_(ptr, value);
    ptr = strcat_(ptr, "\r\n");
    return ptr;
}

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
    char *res) {

    // char *aux = (char*)malloc(sizeof(char) * MAX_BUFF_SIZE);
    // memset(aux, 0, MAX_BUFF_SIZE);
    char *aux = (char*)calloc(buffer_size, sizeof(char));
    if (aux == NULL) return NULL;
    // Secures the pointer address since the "aux" is going to be consumed thru the function
    res = aux;
    // Build response
    aux - addheader("HTTP/1.0 ", status_code, aux);
    // More straight-forward way to add a header
    aux = addheader("Content-Type: ", content_type, aux);
    // There is some pre processing to be done to get the right data
    char *content_length = uitoa_(strlen(body));
    aux = addheader("Content-Length: ", content_length, aux);
    free_(content_length);
    // Body is the last data to concatenate
    // A blank line is part of the 1.0 HTTP protocol
    aux = strcat_(aux, "\r\n");
    aux = strcat_(aux, body);

    return res;
}