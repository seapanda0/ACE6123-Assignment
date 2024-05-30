#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "dataset.txt"

int main (void){
    FILE *fp = fopen(FILENAME, "r+");
    if (!fp){
        perror("Error Opening File");
    }
    char c;
    int linecount = 0;
    while ((c = fgetc(fp)) != EOF){
        if (c == '\n'){
            linecount++;
        }
    };

    fclose(fp);

}