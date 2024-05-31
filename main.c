#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#define FILENAME "dataset.txt"
#define REGEX_EXPRESSION "^.{2,3}\\s[0-9]*,[A-Z]+,[A-Z]+,[0-9]+,[0-9]{4},[0-9]+,[0-9]{1},\n*.*$"

// Return true lines in the files and return -1 if a empty line is found 
int trueLinecount(FILE *fp){
    char line[256]; //assume each line max is 256
    int trueLine = 0;
    int formatting_issue;

    while (fgets(line, sizeof(line), fp) != NULL){
        int i = 0;
        
        //move file pointer to a non white space character
        while(isspace((unsigned char)line[i])){
            if (line[i] == '\n'){
                fclose(fp);
                fprintf(stderr, "Error: blank line at line %d of %s\n", trueLine, FILENAME);
                exit(EXIT_FAILURE);              }
            i++;
        }
        // if character is \n then dont count the line
        if (!isspace((unsigned char)line[i]) && line[i]!='\n'){
            trueLine++;
            // printf("%s", line);
        }
    }
    rewind(fp);
    return trueLine;
}

// Validate the format of lines inside the file using regex
int validateFile(FILE  *fp){
    regex_t regex;
    int reti = regcomp(&regex, REGEX_EXPRESSION , REG_EXTENDED);
    int i = 1;
    char line[256];

    // validate format of each line and 
    while (fgets(line, sizeof(line), fp) != NULL){
        reti = regexec(&regex, line, 0, NULL, 0);
        if (reti && i!=1){
            regfree(&regex);
            fclose(fp);
            fprintf(stderr, "Error: format error at line %d of %s\n", i, FILENAME);
            exit(EXIT_FAILURE);        }
        i++;
    }
    rewind(fp);
    regfree(&regex);
}

int main (void){

    FILE *fp = fopen(FILENAME, "r+");
    if (!fp){
        perror("Error Opening File");
    }
    int lineCount = trueLinecount(fp);
    validateFile(fp);

    fclose(fp);

}