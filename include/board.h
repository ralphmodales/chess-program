#ifndef BOARD_H
#define BOARD_H

#define SIZE 8
#define EMPTY '.'

extern char board[SIZE][SIZE];
extern int currentPlayer; // 0 for white and 1 for black

void initializeBoard();
void displayBoard();

#endif // !BOARD_H

