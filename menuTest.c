#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <curses.h>
#include <signal.h>

int main() {

    initscr(); noecho(); cbreak(); start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);

    int maxY=0, maxX=0;
    getmaxyx(stdscr, maxY, maxX);

    // Create the window for the bottom menu bar
    WINDOW *bottomMenu = newwin(1, maxX, (maxY-1), 0);
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

    int n_choices = sizeof(choices)/sizeof(choices[0]);
    int spacing = maxX/n_choices;
    int key, menuItem = 0;
    wbkgd(bottomMenu, COLOR_PAIR(2));

    do{
        // Draw the screen with a specific highlight from 0-4
        for (int i = 0; i < n_choices; i++){
            if (i == menuItem){
                wattron(bottomMenu, COLOR_PAIR(1));
            }
            mvwprintw(bottomMenu, 0, i * spacing, choices[i]);
            wattroff(bottomMenu, COLOR_PAIR(1));
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
        default:
            break;
        }

    } while (key != '\n');

    // wgetch(bottomMenu);
    // wattron(bottomMenu,COLOR_PAIR(1));
    // // waddstr(bottomMenu, "Choice 1");
    // wgetch(bottomMenu);

    int choice = 0;
    int highlight = 0;

    endwin();

    return 0;
}