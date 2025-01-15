#ifndef BOARD_H
#define BOARD_H

#define SIZE 8
#define EMPTY '.'

extern char board[SIZE][SIZE];
extern int currentPlayer;  
extern int canCastleKingside[2];
extern int canCastleQueenside[2];
extern int lastPawnDoubleMove[2];
extern int lastMoveWasDoubleJump;
extern int fiftyMoveCounter;
extern int moveHistory[1000][4];
extern int moveCount;

void initializeBoard();
void displayBoard();

#endif // !BOARD_H
