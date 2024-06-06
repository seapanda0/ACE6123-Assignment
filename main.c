#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <regex.h>
#include <curses.h>

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

void cursesPrintMain(dataSet *head, WINDOW *main, int displayableRows, int spacing, int highlitedRow, int index, int numElement)
{
    dataSet *curr = head;
    // if (index>numElement-displayableRows){
    //     index = numElement - displayableRows;
    // }
    // Scrolling mechanism
    if (index >= 0){
        for (int k = 0 ; k <index; k++){
            curr = curr->nextNode;
        }
    }
    // Print vertically
    for (int i = 0; (i < displayableRows) && (curr != NULL); i++)
    {
        if (highlitedRow == i){
            wattron(main, A_REVERSE);
        }
        // Print horizontally
        for (int j = 0; (j < numElement); j++)
        {
            wmove(main, i, j * spacing);
            wclrtoeol(main);
            switch (j)
            {
            case 0:
                wprintw(main, "%d", (i + index + 1));
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
        curr = curr-> nextNode;
        
    }
}

int main (){

    FILE *fp = fopen(FILENAME, "r+");
    if (!fp){
        perror("Error Opening File");
    }

    int lineCount = trueLinecount(fp);
    validateFile(fp);
    dataSet *db = loadFile(fp, lineCount);
    int numElement = lineCount - 1;

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
            "insert", 
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

    do{
        cursesPrintMain(db, main, displayableRows, attributesSpacing, highlitedRow, index, numElement);
        // Draw the screen with a specific highlight from 0-4
        for (int i = 0; i < n_choices; i++){
            if (i == menuItem){
                wattron(bottomMenu, A_REVERSE);
            }
            mvwprintw(bottomMenu, 1, i * spacing, choices[i]);
            wattroff(bottomMenu, A_REVERSE);
        }
        // printw("%d", menuItem);
        // refresh();
        key = wgetch(bottomMenu);
        switch (key){
        case KEY_LEFT:
            menuItem--;
            if (menuItem < 0)
                menuItem = 0;
            break;
        case KEY_RIGHT:
            menuItem++;
            if (menuItem > n_choices - 1)
                menuItem = n_choices - 1;
            break;
        case KEY_UP:
            if (highlitedRow != 0){
                highlitedRow--;
            }else{
                index--;
            };
            if (index<0) index = 0;
            if (highlitedRow<0) highlitedRow = 0;
            break;
        case KEY_DOWN:
            if(highlitedRow != displayableRows -1){
                highlitedRow++;
            }else{
                index++;
            }
            if (index>numElement-displayableRows) index = numElement-displayableRows;
            if (highlitedRow>displayableRows - 1) highlitedRow = displayableRows -1;
            break;
        }
    } while (key != '\n');

    // sortByFlightNumber(&db);
    printTable(db);

    customOrder *search1;
    searchFlightNumber(db, "Asdhsu", &search1);
    printCustomOrder(search1);
    // printf("%s",search1->element->flightNumber);
    fclose(fp);
    endwin();
}