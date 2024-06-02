#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <regex.h>

#define FILENAME "dataset.txt"
#define REGEX_EXPRESSION "^.{2,3}\\s[0-9]*,[A-Z]+,[A-Z]+,[0-9]+,[0-9]{4},(0|[1-9][0-9]*)(\\.[0-9]+)?,[0-9]{1},\n*.*$" // Narrow down acceptable char lengths to avoid overflow

typedef struct dataSet{
    char flightNumber[20];
    char origin[5];
    char destination[5];
    short capacity;
    short departureHour;
    short departureMinutes;
    float price;
    short stops;
    struct dataSet *nextNode;
    struct dataSet *prevNode;
}dataSet;

// To rearrange original linked list by creating another layer of linked list, used by search function;
typedef struct customOrder{
    struct dataSet *element;
    struct customOrder *nextElement;
    struct customOrder *previousElement;
}customOrder;

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

void printTable(dataSet *head){
    dataSet *curr = head;
    int i = 1;
    printf("Num\tFlight Number\tOrigin\tDestination\tCapacity\tDeparture Time\tPrice\t\tStops\tPrev\tCurrent\t\tNext\n");

    while(curr!= NULL){
        printf("%d\t%s\t\t%s\t%s\t\t%hd\t\t%hd%hd\t\t%f\t%hd\t%p\t%p\t%p\n",i, curr->flightNumber, curr->origin, curr->destination, 
        curr->capacity, curr->departureHour, curr->departureMinutes, curr->price, curr->stops, curr->prevNode,curr, curr->nextNode);
        curr = curr->nextNode;
        i++;
    }
    return;
}

// Print a custom order such as those returned by search function
void printCustomOrder(customOrder *order){
    int i = 1;
    printf("Num\tFlight Number\tOrigin\tDestination\tCapacity\tDeparture Time\tPrice\t\tStops\tPrev\tCurrent\t\tNext\n");
    while (order != NULL){
        printf("%d\t%s\t\t%s\t%s\t\t%hd\t\t%hd%hd\t\t%f\t%hd\t%p\t%p\t%p\n", i, order->element->flightNumber, order->element->origin, order->element->destination,
        order->element->capacity, order->element->departureHour, order->element->departureMinutes, order->element->price, order->element->stops, 
        order->element->prevNode, order->element, order->element->nextNode);
        order = order->nextElement;
        i++;
    }
    return;
}

// Load file into a linked list memory
dataSet *loadFile(FILE *fp, int lineCount){
    char line[256], time[5];
    dataSet temp;
    int err, i = 0;

    dataSet *curr = (dataSet *)calloc(1, sizeof(dataSet));
    dataSet *head = curr;
    // printf("%p\n", head);
    //jump to first newline
    while( fgetc(fp) != '\n' ){}

    for (i = 1; i <= lineCount-1; i++){

        fgets(line, sizeof(line), fp);
        sscanf(line, "%19[^,],%5[^,],%5[^,],%hd,%4[^,],%f,%hd", curr->flightNumber, curr->origin, curr->destination,
               &(curr->capacity), time, &(curr->price), &(curr->stops));

        err = validateTime(time, &(curr->departureHour), &(curr->departureMinutes));

        if (err){
            fclose(fp);
            fprintf(stderr, "Error: Date Format Error at line %d of %s\n",i+1, FILENAME);
            exit(EXIT_FAILURE);
        }
        if (i != lineCount -1){
            curr->nextNode = (dataSet *)calloc(1, sizeof(dataSet));
            curr->nextNode->prevNode = curr;
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

// Swap nodeA and nodeB position, the head will be modified if neccessary nodeA is the head;
void swap(dataSet **head, dataSet *nodeA, dataSet *nodeB){
    nodeB->prevNode=nodeA->prevNode;
    nodeA->nextNode=nodeB->nextNode;
    if(nodeA->prevNode != NULL){
        nodeA->prevNode->nextNode = nodeB;
    }else{
        *head = nodeB;
    }
    if(nodeB->nextNode != NULL){
        nodeB->nextNode->prevNode = nodeA;
    }
    nodeA->prevNode = nodeB;
    nodeB->nextNode = nodeA;
    return;
}

// Bubble sort
void sortByFlightNumber(dataSet **head)
{
    dataSet *curr = *head;
    bool swapped = false;
    while (true){
        if (curr->nextNode != NULL){
            int result = strcmp(curr->flightNumber, curr->nextNode->flightNumber);
            if (result > 0){
                swap(head, curr, curr->nextNode);
                swapped = true;
            }
            else{
                curr = curr->nextNode;
            }
        }else{
            if (swapped == true){
                curr = *head;
                swapped = false;
            }else{
                break;
            }
        }
    }
}

// Linear search using strstr function, return true if there is at least one match
bool searchFlightNumber(dataSet *head, char input[], customOrder **headSearch){
    customOrder *currSearch = (customOrder*)calloc(1,sizeof(customOrder));
    *headSearch = currSearch;
    dataSet *curr = head;
    char *result;
    bool isEmpty = true;
    while(curr != NULL){
        result = strstr(curr->flightNumber, input);
        // if the input is a substring
        if (result != NULL){
            currSearch->element = curr;
            currSearch->nextElement = (customOrder*)calloc(1,sizeof(customOrder));
            currSearch->nextElement->previousElement = currSearch;
            currSearch = currSearch->nextElement;
            isEmpty =  false;
        }
        curr = curr->nextNode;
    }
    if (!isEmpty){
        currSearch->previousElement->nextElement = NULL;
        free(currSearch);
    }else{
        // If no match is found, delete first pointer and make it point to NULL
        free(*headSearch);
        *headSearch = NULL;
    }
    return !isEmpty;
}

int main (){

    FILE *fp = fopen(FILENAME, "r+");
    if (!fp){
        perror("Error Opening File");
    }

    int lineCount = trueLinecount(fp);
    validateFile(fp);
    dataSet *db = loadFile(fp, lineCount);

    // sortByFlightNumber(&db);
    printTable(db);

    customOrder *search1;
    searchFlightNumber(db, "Asdhsu", &search1);
    printCustomOrder(search1);
    // printf("%s",search1->element->flightNumber);
    fclose(fp);
}