#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

/* turns string "string" (with length len_string) to uppercase */
/* returns 1 if there has been an error, 0 if there has been no error */

int toUppercase(char* string, int len_string)
{
    for (u_int64_t i = 0; i < len_string; i++)
    {
        string[i] = toupper(string[i]);
        i++;
    }
}

/* turns string "string" (with length len_string) to lowercase */
/* returns 1 if there has been an error, 0 if there has been no error */

int toLowercase(char* string, int len_string)
{
    for (u_int64_t i = 0; i < len_string; i++)
    {
        string[i] = tolower(string[i]);
        i++;
    }
   return 0;
}

/* counts the appearences of character "c" in string "string" (with length len_string) */
/* returns -1 if there has been an error, and the number of appearences if there has been no error */

int countChar(char* string, int len_string, char c){}


int main(int argc, char *argv[])
{
    FILE *fptr;
    fptr = fopen("string.txt", "r");
    char string = fptr.read();
    {
        perror("Unable to open input file");
        exit(2);
    }
    toLowercase(string, 3);
    printf("%s", string);
}
