#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define MAX(a, b) (a > b ? a : b)
#define DELAY  10000

/* Create Struct Point Player and Difficulty */
typedef struct Point {
    uint16_t x;
    uint16_t y;
} Point;

typedef struct Difficulty {
    uint8_t diff;
    uint8_t lim;
} Difficulty;

typedef struct Player {
    Point p;
    uint16_t win;
} Player;

/* Init Point Players, Map and Dot */
Point m = {0, 0}, c = {0, 0};
Player p1 = {0, 1, 0}, p2 = {0, 1, 0};

/* Timing and Limit Bot */
Difficulty timing[3] = {{12, 5}, {9, 10}, {6, 15}};

/* Init Run While */
bool run = true;

/* Window Menu */
uint8_t menu()
{
    WINDOW *win;
    uint16_t ch, yMax, xMax, yMaxW, xMaxW;
    int8_t i = 0;
    char list[3][5] = {"Easy", "Midd", "Hard"};
    char item[5];
        
    initscr();
    getmaxyx(stdscr, yMax, xMax); 
    win = newwin(5, xMax/4, yMax/2-(5/2), xMax/2-(xMax/4)/2); 
    getmaxyx(win, yMaxW, xMaxW);
    box(win, 0, 0);            
    mvwprintw(win, 0, xMaxW/2-2, "%s", "Menu"); 
    
    for (i; i < 3; i++) {
        if (i == 0)
            wattron(win, A_STANDOUT); 
        else
            wattroff(win, A_STANDOUT);
        sprintf(item, "%-4s", list[i]);
        mvwprintw(win, i + 1, 2, "* %s", item);
    }
    
    wrefresh(win);
    noecho();        
    keypad(win, TRUE); 
    curs_set(0);     
    
    i = 0;

    while ((ch = wgetch(win)) != 10) {
        sprintf(item, "%-4s", list[i]);
        mvwprintw(win, i + 1, 2, "* %s", item);
        
        switch (ch) {
            case KEY_UP:
                i--;
                i = (i < 0) ? 2 : i;
                break;
            case KEY_DOWN:
                i++;
                i = (i > 2) ? 0 : i;
                break;
        }
        
        wattron(win, A_STANDOUT);
        sprintf(item, "%-4s", list[i]);
        mvwprintw(win, i + 1, 2, "/ %s", item);
        wattroff(win, A_STANDOUT);
    }
    
    delwin(win);
    endwin();

    return i;    
}

/* Player Controler */
void key_event(void)
{
    struct timespec length = {0, 0};
    uint16_t i, c;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    switch(c = wgetch(stdscr)) {
        case KEY_UP: case 'k': case 'K':
            p1.p.y--;
            break;

        case KEY_DOWN: case 'j': case 'J':
            p1.p.y++;
            break;

        case KEY_LEFT: case 'h': case 'H':
            p1.p.x--;
            break;

        case KEY_RIGHT: case 'l': case 'L':
            p1.p.x++;
            break;

        case 'q': case 'Q':
            run = false;
            break;
        
        default:
            pselect(1, &rfds, NULL, NULL, &length, NULL);
    }
    return;
}

void new_point(void)
{
    while (1) {
        c.x = rand() % m.x-1;
        c.y = rand() % m.y-1;
        if ((c.x > 0 && c.x < m.x-1) && (c.y > 1 && c.y < m.y-1))
            break;
    }
}

void end_game(Point m, bool win)
{
    clear();
    if (win) {
        mvprintw(m.y/2,(m.x-7)/2, "YOU WIN");
    } else {
        mvprintw(m.y/2,(m.x-8)/2, "YOU LOSE");
    }
    refresh();
    sleep(2);
    run = false;
}

int main()
{
    srand(time(0));
    uint32_t score = 0, i = 0, choice = 0;

    /* Init Ncurses */
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, true);

    /* New Window */
    WINDOW *win = newwin(0, 0, 0, 0);

    /* Border Window */
    // uint8_t left, right, top, bottom, tlc, trc, blc, brc;
    // left = right = '|';
    // top = bottom = '-';
    // tlc = brc = trc = blc = '+';

    /* Choice Generate Bot */
    getmaxyx(stdscr, m.y, m.x);
    p2.p.x = m.x-1;
    
    /* First Generation New Point */
    new_point();

    /* Timing */
    choice = menu();

    while(run) {
        key_event();
        getmaxyx(win, m.y, m.x);
        // wborder(win, left, right, top, bottom, tlc, trc, blc, brc);
        box(win, 0, 0);
        wprintw(win, "SCORE - MY: %d - BOT: %d ", p1.win, p2.win);
        mvwprintw(win, m.y-1, 0, "%dx%d", m.x, m.y);
  
        /* Displaying Players and Dot */
        mvwaddch(win, p1.p.y, p1.p.x, 'o');
        mvwaddch(win, p2.p.y, p2.p.x, 'm');
        mvwaddch(win, c.y, c.x, 'x');

        /* Algorithm Bot Control */
        if (i >= timing[choice].diff) {
            if (abs(p2.p.x-c.x) > abs(p2.p.y-c.y)) {
                if (p2.p.x < c.x)
                    p2.p.x++;
                else if (p2.p.x > c.x)
                    p2.p.x--;
            } else {
                if (p2.p.y < c.y)
                    p2.p.y++;
                else if (p2.p.y > c.y)
                    p2.p.y--;
            }
            i = 0;
        } else {
            i++;
        }

        /* Field Boundaries */
        if (p1.p.x < 1)
            p1.p.x++;
        else if (p1.p.x >= m.x-1)
            p1.p.x--;
        
        if (p1.p.y < 2)
            p1.p.y++;
        else if (p1.p.y >= m.y-1)
            p1.p.y--;

        /* Generation New Point */
        if (p1.p.x == c.x && p1.p.y == c.y) {
           p1.win++;
           new_point();
        }
        if (p2.p.x == c.x && p2.p.y == c.y) {
            p2.win++;
            new_point();
        }
        if (c.x >= m.x-1 || c.y >= m.y-1) {
            new_point();
        }

        /* Break Program */
        if (p1.win >= timing[choice].lim)
            end_game(m, 1);
        if (p2.win >= timing[choice].lim)
           end_game(m, 0);

        usleep(DELAY);
        wrefresh(win);
        werase(win);
    }
    delwin(win);
    endwin();
}