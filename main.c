#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <regex.h>
#include <curses.h>

#define FILENAME "dataset.txt"
#define REGEX_EXPRESSION "^.{2,3}\\s[0-9]*,[A-Z]+,[A-Z]+,[0-9]+,[0-9]{4},(0|[1-9][0-9]*)(\\.[0-9]+)?,[0-9]{1},\n*.*$" // Narrow down acceptable char lengths to avoid overflow

#define EXIT_SEARCH "Press 'q' to exit searching"
#define EXIT_SEARCH_N 27

#define WRONG_FORMAT "Wrong Format! Please try again"
#define WRONG_FORMAT_N 30

#define DATASET_HEADER "Flight number,origin,destination,capacity,departure time,price,stops,\n"

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

// Return true if time is correct, split time in String to two hour and minutes in short
bool validateTime(char time[], short *hour, short *minutes){
    char temp[3];
    strncpy(temp, time, 2), temp[2]='\0';
    *hour = atoi(temp);
    strncpy(temp, time+2, 2), temp[2]='\0';
    *minutes = atoi(temp);

    if (*minutes>= 0 && *minutes <= 59 && *hour>=0 && *hour<=23){
        return true;
    }else{
        return false;
    }
}

// combine hours and minutes to form a string
void timecvtString(char *timeStr, short hour, short minutes){
    sprintf(timeStr, "%02hd%02hd", hour, minutes);
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
    bool err; 
    int i = 0;

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

        if (!err){
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

// Bubble sort, option 1-7 controls what to be sorted
void sortDB(dataSet **head, int option)
{
    dataSet *curr = *head;
    bool swapped = false;
    char str1[5], str2[5];
    int result;
    while (true){
        if (curr->nextNode != NULL){
            switch (option)
            {
            case 1:
                result = strcmp(curr->flightNumber, curr->nextNode->flightNumber);
                break;
            case 2:
                result = strcmp(curr->origin, curr->nextNode->origin);
                break;
            case 3:
                result = strcmp(curr->destination, curr->nextNode->destination);
                break;
            case 4:
                result = curr->capacity - curr->nextNode->capacity;
                break;
            case 5:
                timecvtString(str1, curr->departureHour, curr->departureMinutes);
                timecvtString(str2, curr->nextNode->departureHour, curr->nextNode->departureMinutes);
                result = strcmp(str1, str2);
            case 6:
                result = curr->price - curr->nextNode->price;
                break;
            case 7:
                result = curr->stops - curr->nextNode->stops;
                break;
            default:
                break;
            }
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

// Linear search using strstr function, return number of matches found
// Use optiion to control what to search 1: Flight Number, 2: Origin, 3: Destination
int searchDB(dataSet *head, char input[], customOrder **headSearch, int option){
    customOrder *currSearch = (customOrder*)calloc(1,sizeof(customOrder));
    *headSearch = currSearch;
    dataSet *curr = head;
    char *result;
    bool isEmpty = true;
    int numMatches = 0;
    switch (option)
    {
    case 1:
        while (curr != NULL)
        {
            result = strstr(curr->flightNumber, input);
            // if the input is a substring
            if (result != NULL)
            {
                currSearch->element = curr;
                currSearch->nextElement = (customOrder *)calloc(1, sizeof(customOrder));
                currSearch->nextElement->previousElement = currSearch;
                currSearch = currSearch->nextElement;
                isEmpty = false;
                numMatches++;
            }
            curr = curr->nextNode;
        }
        break;
    case 2:
        while (curr != NULL)
        {
            result = strstr(curr->origin, input);
            // if the input is a substring
            if (result != NULL)
            {
                currSearch->element = curr;
                currSearch->nextElement = (customOrder *)calloc(1, sizeof(customOrder));
                currSearch->nextElement->previousElement = currSearch;
                currSearch = currSearch->nextElement;
                isEmpty = false;
                numMatches++;
            }
            curr = curr->nextNode;
        }
        break;
    case 3:
        while (curr != NULL)
        {
            result = strstr(curr->destination, input);
            // if the input is a substring
            if (result != NULL)
            {
                currSearch->element = curr;
                currSearch->nextElement = (customOrder *)calloc(1, sizeof(customOrder));
                currSearch->nextElement->previousElement = currSearch;
                currSearch = currSearch->nextElement;
                isEmpty = false;
                numMatches++;
            }
            curr = curr->nextNode;
        }
        break;
    default:
        break;
    }
    if (!isEmpty){
        currSearch->previousElement->nextElement = NULL;
        free(currSearch);
    }else{
        // If no match is found, delete first pointer and make it point to NULL
        free(*headSearch);
        *headSearch = NULL;
    }
    return numMatches;
}

void writeFile(dataSet *head, FILE *fp){
    rewind(fp);
    dataSet *curr = head;
    char timeStr[5];

    fprintf(fp, DATASET_HEADER);

    while(curr!= NULL){
        timecvtString(timeStr, curr->departureHour, curr->departureMinutes);
        fprintf(fp, "%s,%s,%s,%hd,%s,%.2f,%hd,\n", curr->flightNumber, curr->origin, curr->destination, 
        curr->capacity, timeStr, curr->price, curr->stops);
        curr = curr->nextNode;
    }
    return;
}

// Input a string and validate it using 
void inputandValidateStr(WINDOW *bottomMenu, char *validatedStr, char *regexExpressrion, int spacing, int maxX, bool initialErr)
{
    regex_t regex;
    bool validated = false;
    bool firstErr = initialErr;
    while (!validated)
    {
        int reti = regcomp(&regex, regexExpressrion, REG_EXTENDED);
        wmove(bottomMenu, 0, spacing);
        wclrtoeol(bottomMenu);
        if(firstErr){
            mvwprintw(bottomMenu, 0, maxX-WRONG_FORMAT_N, WRONG_FORMAT);
            wrefresh(bottomMenu);
        }
        mvwgetnstr(bottomMenu, 0, spacing, validatedStr, 9);
        reti = regexec(&regex, validatedStr, 0, NULL, 0);
        if (reti == 0)
        {
            validated = true;
        }else{
            firstErr = true;
        }
    }
}

// Redundant function, placed for future use
void inputandValidateShort(WINDOW *bottomMenu, short *validatedShort, char *regexExpressrion, int spacing, int maxX)
{
    regex_t regex;
    char validatedStr[10];
    int reti = regcomp(&regex, regexExpressrion, REG_EXTENDED);
    bool validated = false;
    bool firsrErr = false;
    while (!validated)
    {
        int reti = regcomp(&regex, regexExpressrion, REG_EXTENDED);
        wmove(bottomMenu, 0, spacing);
        wclrtoeol(bottomMenu);
        if(firsrErr){
            mvwprintw(bottomMenu, 0, maxX-WRONG_FORMAT_N, WRONG_FORMAT);
            wrefresh(bottomMenu);
        }
        mvwgetnstr(bottomMenu, 0, spacing, validatedStr, 9);
        reti = regexec(&regex, validatedStr, 0, NULL, 0);
        if (reti == 0)
        {
            *validatedShort = atoi(validatedStr);
            validated = true;
        }else{
            firsrErr = true;
        }
    }

}

// valdate time, the function first call inputandValidateStr and proceed to use validateTime to check time format
//  pass the hour and minutes as reference to be modified
void inputValidateTime(WINDOW *bottomMenu, char *validatedStr, char *regexExpressrion,
                       int spacing, int maxX,
                       short *hour, short *minutes)
{
    bool repeat = false;
    do
    {
        inputandValidateStr(bottomMenu, validatedStr, regexExpressrion, spacing, maxX, repeat);
        repeat = true;
    } while ((validateTime(validatedStr, hour, minutes)) == false);
}

// Print the main UI, result is a key indicating which action has been pressed
void cursesPrintMain(dataSet *head, WINDOW *main, WINDOW *bottomMenu,
                     int displayableRows, int spacing, int numElement, int n_choices,
                     int *menuItem, int *index, int *highlitedRow, int *key, char **choices, int n_attributes)
{
    wmove(bottomMenu, 0, 0);
    wclrtoeol(bottomMenu);
    dataSet *curr = head;

    // Scrolling mechanism
    if (*index >= 0)
    {
        for (int k = 0; k < *index; k++)
        {
            curr = curr->nextNode;
        }
    }
    // Print vertically
    for (int i = 0; (i < displayableRows) && (curr != NULL); i++)
    {
        if (*highlitedRow == i)
        {
            wattron(main, A_REVERSE);
        }
        // Print horizontally
        for (int j = 0; (j < n_attributes); j++)
        {
            wmove(main, i, j * spacing);
            wclrtoeol(main);
            switch (j)
            {
            case 0:
                wprintw(main, "%d", (i + *index + 1));
                break;
            case 1:
                wprintw(main, "%s", curr->flightNumber);
                break;
            case 2:
                wprintw(main, "%s", curr->origin);
                break;
            case 3:
                wprintw(main, "%s", curr->destination);
                break;
            case 4:
                wprintw(main, "%d", curr->capacity);
                break;
            case 5:
                char timeStr[5];
                timecvtString(timeStr, curr->departureHour, curr->departureMinutes);
                wprintw(main, "%s", timeStr);
                break;
            case 6:
                wprintw(main, "%.2f", curr->price);
                break;
            case 7:
                wprintw(main, "%hd", curr->stops);
                break;
            default:
                break;
            }
            wrefresh(main);
        }
        wattroff(main, A_REVERSE);
        curr = curr->nextNode;
    }
    // Draw the screen with a specific highlight from 0-4
    for (int i = 0; i < n_choices; i++)
    {
        if (i == *menuItem)
        {
            wattron(bottomMenu, A_REVERSE);
        }
        mvwprintw(bottomMenu, 1, i * spacing, choices[i]);
        wattroff(bottomMenu, A_REVERSE);
    }
    // printw("%d", menuItem);
    // refresh();
    *key = wgetch(bottomMenu);
    switch (*key)
    {
    case KEY_LEFT:
        (*menuItem)--;
        if (*menuItem < 0)
            *menuItem = 0;
        break;
    case KEY_RIGHT:
        (*menuItem)++;
        if (*menuItem > n_choices - 1)
            *menuItem = n_choices - 1;
        break;
    case KEY_UP:
        if (*highlitedRow != 0)
            (*highlitedRow)--;
        else
            (*index)--;
        if (*index < 0)
            *index = 0;
        if (*highlitedRow < 0)
            *highlitedRow = 0;
        break;
    case KEY_DOWN:
        if (*highlitedRow != displayableRows - 1)
            (*highlitedRow)++;
        else
            (*index)++;
        if ((*index > numElement - displayableRows))
            *index = numElement - displayableRows;
        if ((numElement - displayableRows )<= 0){
            *index = 0;
            if (*highlitedRow > numElement - 1)
                *highlitedRow = numElement - 1;
        }
        if (*highlitedRow > displayableRows - 1)
            *highlitedRow = displayableRows - 1;
        break;
    }
}

void cursesPrintSort(dataSet **head, WINDOW *main, WINDOW *bottomMenu, WINDOW *attributeRow,
                     int displayableRows, int numElement, int n_choices, int n_attributes, int attributesSpacing,
                     int *menuItem, int *index, int *highlitedRow, int *key, char **choices, char **attributes)

{
    int sortItem = 1;
    mvwprintw(bottomMenu, 0, 0, "Press left & right to the attribute to be sorted.\tPress 'q' to extt sorting");
    wrefresh(bottomMenu);
    dataSet *curr = *head;
    bool sortAgain = false;

    do
    {
        // Determine if need to sort or not and use sortItem to determine the attribute to be sorted
        if (sortAgain == true)
        {
            sortDB(head, sortItem);
            sortAgain = false;
            curr = *head;
        }
        // Scrolling mechanism
        if (*index > 0)
        {
            for (int k = 0; k < *index; k++)
            {
                curr = curr->nextNode;
            }
        }
        // Print vertically
        for (int i = 0; (i < displayableRows) && (curr != NULL); i++)
        {
            if (*highlitedRow == i)
            {
                wattron(main, A_REVERSE);
            }
            // Print horizontally
            for (int j = 0; (j < n_attributes); j++)
            {
                wmove(main, i, j * attributesSpacing);
                wclrtoeol(main);
                switch (j)
                {
                case 0:
                    wprintw(main, "%d", (i + *index + 1));
                    break;
                case 1:
                    wprintw(main, "%s", curr->flightNumber);
                    break;
                case 2:
                    wprintw(main, "%s", curr->origin);
                    break;
                case 3:
                    wprintw(main, "%s", curr->destination);
                    break;
                case 4:
                    wprintw(main, "%d", curr->capacity);
                    break;
                case 5:
                    char timeStr[5];
                    timecvtString(timeStr, curr->departureHour, curr->departureMinutes);
                    wprintw(main, "%s", timeStr);
                    break;
                case 6:
                    wprintw(main, "%.2f", curr->price);
                    break;
                case 7:
                    wprintw(main, "%hd", curr->stops);
                    break;
                default:
                    break;
                }
                wrefresh(main);
            }
            wattroff(main, A_REVERSE);
            curr = curr->nextNode;
        }
        // Print the top attribute row
        for (int i = 0; i < n_attributes; i++)
        {
            if (i == sortItem)
            {
                wattron(attributeRow, A_REVERSE);
            }
            mvwprintw(attributeRow, 0, i * attributesSpacing, attributes[i]);
            wrefresh(attributeRow);
            wattroff(attributeRow, A_REVERSE);
        }
        // printw("%d", menuItem);
        // refresh();
        *key = wgetch(bottomMenu);
        switch (*key)
        {
        case KEY_LEFT:
            sortItem--;
            if (sortItem < 1)
                sortItem = 1;
            break;
        case KEY_RIGHT:
            sortItem++;
            if (sortItem > n_attributes - 1)
                sortItem = n_attributes - 1;
            break;
        case KEY_UP:
            if (*highlitedRow != 0)
                (*highlitedRow)--;
            else
                (*index)--;
            if (*index < 0)
                *index = 0;
            if (*highlitedRow < 0)
                *highlitedRow = 0;
            break;
        case KEY_DOWN:
            if (*highlitedRow != displayableRows - 1)
                (*highlitedRow)++;
            else
                (*index)++;
            if (*index > numElement - displayableRows)
                *index = numElement - displayableRows;
            if ((numElement - displayableRows) <= 0)
            {
                *index = 0;
                if (*highlitedRow > numElement - 1)
                    *highlitedRow = numElement - 1;
            }
            if (*highlitedRow > displayableRows - 1)
                *highlitedRow = displayableRows - 1;
            break;
        case '\n':
            sortAgain = true;
            *index = 0;
            *highlitedRow = 0;
            break;
        }
        curr = *head;
    } while (*key != 'q' && *key != 'Q');
}

void cursesPrintSearch(dataSet *head, WINDOW *main, WINDOW *bottomMenu, WINDOW *attributeRow,
                     int displayableRows, int numElement, int n_choices, int n_attributes, int attributesSpacing, int maxX,
                     int *menuItem, int *index, int *highlitedRow, int *key, char **choices, char **attributes)

{
    int searchItem = 1, numMatches = 0;
    mvwprintw(bottomMenu, 0, 0, "Press left & right to select attribute to be searched.");
    mvwprintw(bottomMenu, 0, maxX-EXIT_SEARCH_N, EXIT_SEARCH);

    wrefresh(bottomMenu);
    dataSet *curr = head;
    customOrder *search, *currSearch;
    bool promptSearch = false, displaySearch = false;
    char input[10];

    do
    {
        // If search is pressed
        if (promptSearch == true)
        {
            wmove(bottomMenu, 0, 0);
            wclrtoeol(bottomMenu);

            mvwprintw(bottomMenu, 0, 0, "Search by %s:", attributes[searchItem]);
            nocbreak();
            echo();
            curs_set(1);
            mvwgetnstr(bottomMenu, 0, 30, input, 9);
            cbreak();
            noecho();
            curs_set(0);
            // TODO: Use switch case to implement searching on various attributes
            numMatches = 0;
            if (input[0] != '\0' && input[0] != ' ')
            {
                numMatches = searchDB(head, input, &search, searchItem);
            }
            if (numMatches != 0)
            {
                displaySearch = true;
                ;
                wmove(bottomMenu, 0, 0);
                wclrtoeol(bottomMenu);
                mvwprintw(bottomMenu, 0, 0, "%d matches has been found! Select any attribute to search again", numMatches);
                wclear(main);
                wrefresh(main);
                mvwprintw(bottomMenu, 0, maxX - EXIT_SEARCH_N, EXIT_SEARCH);
                wrefresh(bottomMenu);
            }
            else
            {
                displaySearch = false;
                wmove(bottomMenu, 0, 0);
                wclrtoeol(bottomMenu);
                mvwprintw(bottomMenu, 0, 0, "No match has been found! Press any key to continue");
                wclear(main);
                wrefresh(main);
                wgetch(bottomMenu);
                mvwprintw(bottomMenu, 0, 0, "Press left & right to select attribute to be searched.");
                mvwprintw(bottomMenu, 0, maxX - EXIT_SEARCH_N, EXIT_SEARCH);
            }

            promptSearch = false;
            currSearch = search;
        }
        // Display search result or display normal database
        if (displaySearch)
        {
            mvwprintw(bottomMenu, 0, maxX-EXIT_SEARCH_N, EXIT_SEARCH);
            if (*index > 0)
            {
                for (int k = 0; k < *index; k++)
                {
                    currSearch = currSearch->nextElement;
                }
            }
            // Print vertically
            for (int i = 0; (i < displayableRows) && (currSearch != NULL); i++)
            {
                if (*highlitedRow == i)
                {
                    wattron(main, A_REVERSE);
                }
                // Print horizontally
                for (int j = 0; (j < numElement); j++)
                {
                    wmove(main, i, j * attributesSpacing);
                    wclrtoeol(main);
                    switch (j)
                    {
                    case 0:
                        wprintw(main, "%d", (i + *index + 1));
                        break;
                    case 1:
                        wprintw(main, "%s", currSearch->element->flightNumber);
                        break;
                    case 2:
                        wprintw(main, "%s", currSearch->element->origin);
                        break;
                    case 3:
                        wprintw(main, "%s", currSearch->element->destination);
                        break;
                    case 4:
                        wprintw(main, "%d", currSearch->element->capacity);
                        break;
                    case 5:
                        char timeStr[5];
                        timecvtString(timeStr, currSearch->element->departureHour, currSearch->element->departureMinutes);
                        wprintw(main, "%s", timeStr);
                        break;
                    case 6:
                        wprintw(main, "%.2f", currSearch->element->price);
                        break;
                    case 7:
                        wprintw(main, "%hd", currSearch->element->stops);
                        break;
                    default:
                        break;
                    }
                    wrefresh(main);
                }
                wattroff(main, A_REVERSE);
                currSearch = currSearch->nextElement;
            }
            // Print the top attribute row
            for (int i = 0; i < n_attributes; i++)
            {
                if (i == searchItem)
                {
                    wattron(attributeRow, A_REVERSE);
                }
                mvwprintw(attributeRow, 0, i * attributesSpacing, attributes[i]);
                wrefresh(attributeRow);
                wattroff(attributeRow, A_REVERSE);
            }
            // printw("%d", menuItem);
            // refresh();
            *key = wgetch(bottomMenu);
            switch (*key)
            {
            case KEY_LEFT:
                searchItem--;
                if (searchItem < 1)
                    searchItem = 1;
                break;
            case KEY_RIGHT:
                searchItem++;
                if (searchItem > n_attributes - 5)
                    searchItem = n_attributes - 5;
                break;
            case KEY_UP:
                if (*highlitedRow != 0)
                    (*highlitedRow)--;
                else
                    (*index)--;
                if (*index < 0)
                    *index = 0;
                if (*highlitedRow < 0)
                    *highlitedRow = 0;
                break;
            case KEY_DOWN:
                if (*highlitedRow != displayableRows - 1)
                    (*highlitedRow)++;
                else
                    (*index)++;
                if (*index > numMatches - displayableRows)
                    *index = numMatches - displayableRows;
                if (*index < 0)
                    *index = 0;
                if (*highlitedRow > displayableRows - 1)
                    *highlitedRow = displayableRows - 1;
                if (*highlitedRow > numMatches - 1)
                    *highlitedRow = numMatches - 1;
                break;
            case '\n':
                promptSearch = true;
                *index = 0;
                *highlitedRow = 0;
                break;
            }
            currSearch = search;
        }else{
            // Display normal database
            if (*index > 0)
            {
                for (int k = 0; k < *index; k++)
                {
                    curr = curr->nextNode;
                }
            }
            // Print vertically
            for (int i = 0; (i < displayableRows) && (curr != NULL); i++)
            {
                if (*highlitedRow == i)
                {
                    wattron(main, A_REVERSE);
                }
                // Print horizontally
                for (int j = 0; (j < numElement); j++)
                {
                    wmove(main, i, j * attributesSpacing);
                    wclrtoeol(main);
                    switch (j)
                    {
                    case 0:
                        wprintw(main, "%d", (i + *index + 1));
                        break;
                    case 1:
                        wprintw(main, "%s", curr->flightNumber);
                        break;
                    case 2:
                        wprintw(main, "%s", curr->origin);
                        break;
                    case 3:
                        wprintw(main, "%s", curr->destination);
                        break;
                    case 4:
                        wprintw(main, "%d", curr->capacity);
                        break;
                    case 5:
                        char timeStr[5];
                        timecvtString(timeStr, curr->departureHour, curr->departureMinutes);
                        wprintw(main, "%s", timeStr);
                        break;
                    case 6:
                        wprintw(main, "%.2f", curr->price);
                        break;
                    case 7:
                        wprintw(main, "%hd", curr->stops);
                        break;
                    default:
                        break;
                    }
                    wrefresh(main);
                }
                wattroff(main, A_REVERSE);
                curr = curr->nextNode;
            }
            // Print the top attribute row
            for (int i = 0; i < n_attributes; i++)
            {
                if (i == searchItem)
                {
                    wattron(attributeRow, A_REVERSE);
                }
                mvwprintw(attributeRow, 0, i * attributesSpacing, attributes[i]);
                wrefresh(attributeRow);
                wattroff(attributeRow, A_REVERSE);
            }
            // printw("%d", menuItem);
            // refresh();
            *key = wgetch(bottomMenu);
            switch (*key)
            {
            case KEY_LEFT:
                searchItem--;
                if (searchItem < 1)
                    searchItem = 1;
                break;
            case KEY_RIGHT:
                searchItem++;
                if (searchItem > n_attributes - 5)
                    searchItem = n_attributes - 5;
                break;
            case KEY_UP:
                if (*highlitedRow != 0)
                    (*highlitedRow)--;
                else
                    (*index)--;
                if (*index < 0)
                    *index = 0;
                if (*highlitedRow < 0)
                    *highlitedRow = 0;
                break;
            case KEY_DOWN:
                if (*highlitedRow != displayableRows - 1)
                    (*highlitedRow)++;
                else
                    (*index)++;
                if (*index > numElement - displayableRows)
                    *index = numElement - displayableRows;
                if (*highlitedRow > displayableRows - 1)
                    *highlitedRow = displayableRows - 1;
                break;
            case '\n':
                promptSearch = true;
                *index = 0;
                *highlitedRow = 0;
                break;
            }
            curr = head;
        }
    } while (*key != 'q' && *key != 'Q');
}

void cursesAdd(dataSet *head, WINDOW *main, WINDOW *bottomMenu, WINDOW *attributeRow,
                     int displayableRows, int n_choices, int n_attributes, int attributesSpacing, int maxX,
                     int *numElement, int *menuItem, int *index, int *highlitedRow, int *key, char **choices, char **attributes)
{
    dataSet *newEntry = (dataSet*)calloc(1,sizeof(dataSet));
    dataSet *curr = head;
    char temp[10];
    // taking input for 7 attributes
    for (int i = 1; i<= n_attributes -1; i++){
        wmove(bottomMenu, 0, 0);
        wclrtoeol(bottomMenu);
        mvwprintw(bottomMenu, 0, 0, "Add %s:", attributes[i]);
        bool validated = false;
        nocbreak();
        echo();
        curs_set(1);
        switch (i)
        {
        case 1:
            inputandValidateStr(bottomMenu, newEntry->flightNumber, "^.{2,3}\\s[0-9]*$", 20, maxX, false);
            break;
        case 2:
            inputandValidateStr(bottomMenu, newEntry->origin, "^[A-Z]+$", 20, maxX, false);
            break;
        case 3:
            inputandValidateStr(bottomMenu, newEntry->destination, "^[A-Z]+$", 20, maxX, false);
            break;
        case 4:
            inputandValidateStr(bottomMenu, temp,"^[0-9]+$", 20, maxX, false);
            newEntry->capacity = atoi(temp);
            break;
        case 5:
            inputValidateTime(bottomMenu, temp,"^[0-9]{4}$", 20, maxX, &(newEntry->departureHour), &(newEntry->departureMinutes));
            break;
        case 6:
            inputandValidateStr(bottomMenu, temp,"^(0|[1-9][0-9]*)(\\.[0-9]+)?$", 20, maxX, false);
            newEntry->price = atof(temp);
            break;
        case 7:
            inputandValidateStr(bottomMenu, temp, "^[0-9]{1}$", 20, maxX, false);
            newEntry->stops = atoi(temp);
            break;
        default:
            break;
        }
        cbreak();
        noecho();
        curs_set(0);
    }
    cbreak();
    noecho();
    curs_set(0);

    // Cyele to last node from head
    while(curr->nextNode != NULL){
        curr = curr->nextNode;
    }
    curr->nextNode = newEntry;
    newEntry->prevNode = curr;
    newEntry->nextNode = NULL;

    mvwprintw(bottomMenu, 0, 0, "New entry has been added! Press any key to continue");
    wgetch(bottomMenu);
    (*numElement)++;
}

void cursesInsert(dataSet *head, WINDOW *main, WINDOW *bottomMenu, WINDOW *attributeRow,
                     int displayableRows, int n_choices, int n_attributes, int attributesSpacing, int maxX,
                     int *numElement, int *menuItem, int *index, int *highlitedRow, int *key, char **choices, char **attributes)
{
    dataSet *newEntry = (dataSet*)calloc(1,sizeof(dataSet));
    dataSet *curr = head;
    char temp[10];
    //taking input for 7 attributes
    for (int i = 1; i<= n_attributes -1; i++){
        wmove(bottomMenu, 0, 0);
        wclrtoeol(bottomMenu);
        mvwprintw(bottomMenu, 0, 0, "Add %s:", attributes[i]);
        bool validated = false;
        nocbreak();
        echo();
        curs_set(1);
        switch (i)
        {
        case 1:
            inputandValidateStr(bottomMenu, newEntry->flightNumber, "^.{2,3}\\s[0-9]*$", 20, maxX, false);
            break;
        case 2:
            inputandValidateStr(bottomMenu, newEntry->origin, "^[A-Z]+$", 20, maxX, false);
            break;
        case 3:
            inputandValidateStr(bottomMenu, newEntry->destination, "^[A-Z]+$", 20, maxX, false);
            break;
        case 4:
            inputandValidateStr(bottomMenu, temp,"^[0-9]{4}$", 20, maxX, false);
            newEntry->capacity = atoi(temp);
            break;
        case 5:
            inputValidateTime(bottomMenu, temp,"[0-9]{4}", 20, maxX, &(newEntry->departureHour), &(newEntry->departureMinutes));
            break;
        case 6:
            inputandValidateStr(bottomMenu, temp,"(0|[1-9][0-9]*)(\\.[0-9]+)?", 20, maxX, false);
            newEntry->price = atof(temp);
            break;
        case 7:
            inputandValidateStr(bottomMenu, temp, "^[0-9]{1}$", 20, maxX, false);
            newEntry->stops = atoi(temp);
            break;
        default:
            break;
        }
        cbreak();
        noecho();
        curs_set(0);
    }
    cbreak();
    noecho();
    curs_set(0);
    // strcpy(newEntry->flightNumber, "AAA 111");
    // strcpy(newEntry->origin, "AAA");
    // strcpy(newEntry->destination, "BBB");
    // newEntry->departureHour = 12;
    // newEntry->departureMinutes = 12;
    // newEntry->capacity = 100;
    // newEntry->price = 100.00;
    // newEntry->stops = 1;  
    
    // Cyele to the current index
    for (int k = 0; k < (*index + *highlitedRow); k++)
    {
        curr = curr->nextNode;
    }
    if(curr->nextNode == NULL){
        curr->nextNode = newEntry;
        newEntry->prevNode = curr;
        newEntry->nextNode = NULL;
    }else{
        newEntry->nextNode = curr->nextNode;
        curr->nextNode->prevNode = newEntry;
        curr->nextNode = newEntry;
        newEntry->prevNode = curr;
    }

    mvwprintw(bottomMenu, 0, 0, "New entry has been inserted in line %d! Press any key to continue", *index + *highlitedRow +2);
    wgetch(bottomMenu);
    (*numElement)++;
}

void cursesDelete(dataSet **head, WINDOW *main, WINDOW *bottomMenu, WINDOW *attributeRow,
                     int displayableRows, int n_choices, int n_attributes, int attributesSpacing, int maxX,
                     int *numElement, int *menuItem, int *index, int *highlitedRow, int *key, char **choices, char **attributes)
{
    dataSet *curr = *head;

    // Cyele to the current index
    for (int k = 0; k < (*index + *highlitedRow); k++)
    {
        curr = curr->nextNode;
    }

    // if the head is to be deleted
    if (curr->prevNode == NULL){
        *(head) = curr->nextNode;
        (*head)->prevNode = NULL;
        free(curr);
    }else if(curr->nextNode == NULL){
        // if the last element is to be deleted
        curr->prevNode->nextNode = NULL;
        free(curr);
    }else{
        // Normal deletion
        curr->prevNode->nextNode = curr->nextNode;
        curr->nextNode->prevNode = curr->prevNode;
        free(curr);
    }


    mvwprintw(bottomMenu, 0, 0, "Entry has been deleted ! Press any key to continue");
    wgetch(bottomMenu);
    (*numElement)--;
}

void cursesUpdate(dataSet *head, WINDOW *main, WINDOW *bottomMenu, WINDOW *attributeRow,
                     int displayableRows, int n_choices, int n_attributes, int attributesSpacing, int maxX,
                     int *numElement, int *menuItem, int *index, int *highlitedRow, int *key, char **choices, char **attributes)
{
    dataSet *newEntry = (dataSet*)calloc(1,sizeof(dataSet));
    dataSet *curr = head;
    char temp[10];
    // Cyele to the current index
    for (int 
    k = 0; k < (*index + *highlitedRow); k++)
    {
        curr = curr->nextNode;
    }
    //taking input for 7 attributes
    for (int i = 1; i<= n_attributes -1; i++){
        wmove(bottomMenu, 0, 0);
        wclrtoeol(bottomMenu);
        wmove(bottomMenu, 1, 0);
        wclrtoeol(bottomMenu);
        mvwprintw(bottomMenu, 0, 0, "Add %s:", attributes[i]);
        bool validated = false;
        nocbreak();
        echo();
        curs_set(1);
        switch (i)
        {
        case 1:
            mvwprintw(bottomMenu, 1, 0, "Current Value: %s", curr->flightNumber);
            inputandValidateStr(bottomMenu, newEntry->flightNumber, ".{2,3}\\s[0-9]*", 20, maxX, false);
            break;
        case 2:
            mvwprintw(bottomMenu, 1, 0, "Current Value: %s", curr->origin);
            inputandValidateStr(bottomMenu, newEntry->origin, "[A-Z]+", 20, maxX, false);
            break;
        case 3:
            mvwprintw(bottomMenu, 1, 0, "Current Value: %s", curr->destination);
            inputandValidateStr(bottomMenu, newEntry->destination, "[A-Z]+", 20, maxX, false);
            break;
        case 4:
            mvwprintw(bottomMenu, 1, 0, "Current Value: %hd", curr->capacity);
            inputandValidateStr(bottomMenu, temp,"[0-9]+", 20, maxX, false);
            newEntry->capacity = atoi(temp);
            break;
        case 5:
            mvwprintw(bottomMenu, 1, 0, "Current Value: %hd%hd", curr->departureHour, curr->departureMinutes);
            inputValidateTime(bottomMenu, temp,"[0-9]{4}", 20, maxX, &(newEntry->departureHour), &(newEntry->departureMinutes));
            break;
        case 6:
            mvwprintw(bottomMenu, 1, 0, "Current Value: %.2f", curr->price);
            inputandValidateStr(bottomMenu, temp,"(0|[1-9][0-9]*)(\\.[0-9]+)?", 20, maxX, false);
            newEntry->price = atof(temp);
            break;
        case 7:
            mvwprintw(bottomMenu, 1, 0, "Current Value: %hd", curr->stops);
            inputandValidateStr(bottomMenu, temp, "[0-9]{1}", 20, maxX, false);
            newEntry->stops = atoi(temp);
            break;
        default:
            break;
        }
        cbreak();
        noecho();
        curs_set(0);
    }
    cbreak();
    noecho();
    curs_set(0);

    strcpy(curr->flightNumber, newEntry->flightNumber);
    strcpy(curr->origin, newEntry->origin);
    strcpy(curr->destination, newEntry->destination);
    curr->capacity = newEntry->capacity;
    curr->departureHour = newEntry->departureHour;
    curr->departureMinutes = newEntry->departureMinutes;
    curr->price = newEntry->price;
    curr->stops = newEntry->stops;

    mvwprintw(bottomMenu, 0, 0, "New entry has been updated in line %d! Press any key to continue", *index + *highlitedRow +2);
    wgetch(bottomMenu);
    (*numElement)++;
    wmove(bottomMenu, 1, 0);
    wclrtoeol(bottomMenu);
}

int main ()
{
    char filename[100];
    printf("Please enter a file to open:\t");
    fgets(filename, 99, stdin);

    FILE *fp = fopen(FILENAME, "r+");
    if (!fp){
        perror("Error Opening File");
    }



    int lineCount = trueLinecount(fp);
    validateFile(fp);

    dataSet *db = loadFile(fp, lineCount);
    int numElement = lineCount - 1;

    // sortByFlightNumber(&db);

    // writetoCopy(db, fp2);

    // Initialize ncurses
    initscr(); noecho(); cbreak(); start_color(); curs_set(0);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_WHITE, COLOR_BLUE);

    int maxY=0, maxX=0;
    getmaxyx(stdscr, maxY, maxX);
    int displayableRows = maxY - 3;

    if (displayableRows <=   0){
        fprintf(stderr, "Error: Terminal has less than 3 rows, please resisze your terminal");
        exit(EXIT_FAILURE);
    }
    // Create the top attribute row
    WINDOW *attributeRow = newwin(1, maxX, 0, 0);
    wbkgd(attributeRow, COLOR_PAIR(3));

    char *attributes[] = {
        "No",
        "Flight Number",
        "Origin",
        "Destination",
        "Capacity",
        "Departure Time",
        "Price",
        "Stops"
    };

    // Dynamicaly calculate the attribute row spacing
    int n_attributes = sizeof(attributes)/sizeof(attributes[0]);
    int attributesSpacing = maxX/n_attributes;

    for (int i = 0; i < n_attributes; i++){
        mvwprintw(attributeRow, 0, i * attributesSpacing, attributes[i]);
    }
    wrefresh(attributeRow);

    // Create the window for the bottom menu bar
    WINDOW *bottomMenu = newwin(2, maxX, (maxY-2), 0);
    wbkgd(bottomMenu, COLOR_PAIR(2));
    keypad(bottomMenu, TRUE);

    char *choices[] = { 
            "Search", 
            "Sort", 
            "Add", 
            "Insert", 
            "Delete", 
            "Update",
            "Save",
            "Quit"
    };
    
    // Dynamically calculate amout of menu and spacing required
    int n_choices = sizeof(choices)/sizeof(choices[0]);
    int spacing = maxX/n_choices;
    int key, menuItem = 0, highlitedRow = 0, index = 0;

    WINDOW *main = newwin(displayableRows, maxX, 1, 0);
    wbkgd(main, COLOR_PAIR(1));
    wrefresh(main);

    while (1)
    {
        do
        {
            // Main display UI
            cursesPrintMain(db, main, bottomMenu, 
            displayableRows, attributesSpacing, numElement, n_choices, 
            &menuItem, &index, &highlitedRow, &key, choices, n_attributes);
 
        } while (key != '\n');

        // menuItem correspond to each of the functionalites present in the bottom menu
        if (menuItem == 0)
        {
            menuItem = index = highlitedRow = key = 0;
            cursesPrintSearch(db, main, bottomMenu, attributeRow, 
            displayableRows, numElement, n_choices, n_attributes, attributesSpacing, maxX,
            &menuItem, &index, &highlitedRow, &key, choices, attributes);

        }else if (menuItem == 1){
            menuItem = index = highlitedRow = key = 0;
            cursesPrintSort(&db, main, bottomMenu, attributeRow, 
            displayableRows, numElement, n_choices, n_attributes, attributesSpacing,
            &menuItem, &index, &highlitedRow, &key, choices, attributes);
        }
        else if (menuItem == 2){
            menuItem = index = highlitedRow = key = 0;
            cursesAdd(db, main, bottomMenu, attributeRow, 
            displayableRows,n_choices, n_attributes, attributesSpacing, maxX,
            &numElement, &menuItem, &index, &highlitedRow, &key, choices, attributes);
        }else if (menuItem == 3){
            cursesInsert(db, main, bottomMenu, attributeRow, 
            displayableRows,n_choices, n_attributes, attributesSpacing, maxX,
            &numElement, &menuItem, &index, &highlitedRow, &key, choices, attributes);
        }else if (menuItem == 4){
            cursesDelete(&db, main, bottomMenu, attributeRow, 
            displayableRows,n_choices, n_attributes, attributesSpacing, maxX,
            &numElement, &menuItem, &index, &highlitedRow, &key, choices, attributes);
        }else if (menuItem == 5){
            cursesUpdate(db, main, bottomMenu, attributeRow, 
            displayableRows,n_choices, n_attributes, attributesSpacing, maxX,
            &numElement, &menuItem, &index, &highlitedRow, &key, choices, attributes);
        }else if (menuItem == 6){
            mvwprintw(bottomMenu, 0, 0, "Do you want to save? (Y/N)?");
            char choice = wgetch(bottomMenu);
            if (choice == 'Y' || choice == 'y'){
                rewind(fp);
                writeFile(db, fp);
                wmove(bottomMenu, 0, 0);
                wclrtoeol(bottomMenu);
                mvwprintw(bottomMenu, 0, 0, "File has been saved! Press any key to continue");
                wgetch(bottomMenu);
            }
        }else if (menuItem == 7){
            break;
        }
    }
    echo();
    fclose(fp);
    endwin();
}