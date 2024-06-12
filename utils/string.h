#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef STRING_H
#define STRING_H

size_t strlen(const char* str);
void addCharToString(char *str, char c);
void removeLastChar(char *str);
void reverse(char str[], int length);
void size_t_to_str(size_t num, char* buffer);
void strcpy(char* dest, const char* src);
int compareDifferentLengths(const char *longer, const char *shorter); 
char* charToStr(char character);

#endif
