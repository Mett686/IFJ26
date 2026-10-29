// scanner_private.h
// Autor: Filip Maxa, FIT

#include <stdlib.h>
#define STRING_STARTING_CAPACITY 64

typedef struct{
    unsigned capacity;
    unsigned length; //bez NTC
    char* str;
} string_t;

void stringAppend(string_t* string, int c){
    if(string->length == string->capacity){
        string->str = realloc(string->str, string->capacity*2);
        if(string->str == NULL)
            abort();
        string->capacity = string->capacity*2;
    }
    string->str[string->length++] = c;
    string->str[string->length] = '\0';
}