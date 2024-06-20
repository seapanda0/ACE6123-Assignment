# Flights data manager in C w/ ncurses

External libraries used:

- [ncurses](https://invisible-island.net/ncurses/man/ncurses.3x.html)
- regex (Prepackaged into POSIX systems)

## Compiling

- Debian:

  ``` bash
  git clone https://github.com/seapanda0/ACE6123-Assignment
    
  sudo apt install libncurses5-dev

  cd ACE6123-Assignment/

  gcc -o main main.c -lncurses
  ```

## Running

  After compiling, type `./main` in your terminal to start the program.

## Using the program

At start, the program will prompt you for the database text file. You need to enter a comma-separated value file in `.txt` extension. The file needs to be in your current working directory. Refer to [`dataset.txt`](dataset.txt) above for sample dataset.

Use left/right arrow keys to navigate through the menu options, Up/down arrow keys or mouse scroll wheel to navigate through the data list.

## Features

- File validation using regular expressions
- Search by flight number, origin and destination
- Sort by all attributes
- Add entry at the botton of the dataset
- Insert entry at a specific line
- Delete a specific entry
- Update a selected entry
- Input validation using regular expressions
- Save file

## Screenshots

![Running][ss-1]

![Searching][ss-3]

![GIF of program's sort function][ss-2]

![Add][ss-4]

![Insert][ss-6]

![Delete][ss-5]

![Update][ss-7]

[ss-1]:https://i.imgur.com/ktOytH7.gif "INTRO"

[ss-2]: https://i.imgur.com/bjGXtOa.gif "SORT"

[ss-3]:https://i.imgur.com/MPa7AG3.gif "SEARCH"

[ss-4]:https://i.imgur.com/Bu7u58R.gif "ADD"

[ss-5]:https://i.imgur.com/SahBp2H.gif "DELETE"

[ss-6]:https://i.imgur.com/lFu0Ay0.gif "INSERT"

[ss-7]:https://i.imgur.com/0G5uNmF.gif "UPDATE"
