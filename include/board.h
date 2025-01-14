#ifndef BOARD_H
#define BOARD_H

#define SIZE 8
#define EMPTY '.'

extern char board[SIZE][SIZE];

void initializeBoard();
void displayBoard();

#endif // !BOARD_H

