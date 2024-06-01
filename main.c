#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#define FILENAME "dataset.txt"
#define REGEX_EXPRESSION "^.{2,3}\\s[0-9]*,[A-Z]+,[A-Z]+,[0-9]+,[0-9]{4},[0-9]+,[0-9]{1},\n*.*$" // Narrow down acceptable char lengths to avoid overflow

typedef struct dataSet
{
    char flightNumber[20];
    char origin[5];
    char destination[5];
    short capacity;
    short departureHour;
    short departureMinutes;
    int price;
    short stops;
    struct dataSet *nextNode;
}dataSet;

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
    printf("File Structure is Correct! File has %d lines\n", trueLine);
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
            exit(EXIT_FAILURE);        
        }
        i++;
    }
    printf("Content Validation Successful!\n");
    rewind(fp);
    regfree(&regex);
}

// Return 0 if time is correct, split time in String to two hour and minutes in short
int validateTime(char time[], short *hour, short *minutes){
    char temp[3];
    strncpy(temp, time, 2), temp[2]='\0';
    *hour = atoi(temp);
    strncpy(temp, time+2, 2), temp[2]='\0';
    *minutes = atoi(temp);

    if (*minutes>= 0 && *minutes <= 59 && *hour>=0 && *hour<=23){
        return 0;
    }else{
        return 1;
    }
}

// Load file into a linked list memory
dataSet *loadFile(FILE *fp, int lineCount){
    char line[256], time[5];
    dataSet temp;
    int err, i = 0;

    dataSet *curr = (dataSet *)calloc(1, sizeof(dataSet));
    dataSet *head = curr, *tail;
    // printf("%p\n", head);
    //jump to first newline
    while( fgetc(fp) != '\n' ){}

    for (i = 1; i <= lineCount-1; i++){

        fgets(line, sizeof(line), fp);
        sscanf(line, "%19[^,],%5[^,],%5[^,],%hd,%4[^,],%d,%hd", curr->flightNumber, curr->origin, curr->destination,
               &(curr->capacity), time, &(curr->price), &(curr->stops));

        err = validateTime(time, &(curr->departureHour), &(curr->departureMinutes));

        if (err){
            fclose(fp);
            fprintf(stderr, "Error: Date Format Error at line %d of %s\n",i+1, FILENAME);
            exit(EXIT_FAILURE);
        }
        if (i != lineCount -1){
            curr->nextNode = (dataSet *)calloc(1, sizeof(dataSet));
            curr = curr->nextNode;
        }
        // printf("%d\n", i);
    }
    // curr = head;
    // printf("1 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;
    // printf("2 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;    
    // printf("3 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;
    // printf("4 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;
    // printf("5 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;    
    // printf("6 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;
    // printf("7 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;
    // printf("8 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;    
    // printf("9 %s %p\n", curr->flightNumber, curr->nextNode);
    // curr = curr->nextNode;
    // printf("10 %s %p\n", curr->flightNumber, curr->nextNode);

    return head;
}

int main (){

    FILE *fp = fopen(FILENAME, "r+");
    if (!fp){
        perror("Error Opening File");
    }
    int lineCount = trueLinecount(fp);
    validateFile(fp);

    dataSet *db = loadFile(fp, lineCount);
    fclose(fp);
}