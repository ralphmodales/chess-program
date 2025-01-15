#include <stdio.h>
#include "board.h"

char board[SIZE][SIZE];
int currentPlayer = 0;
int canCastleKingside[2];
int canCastleQueenside[2];
int lastPawnDoubleMove[2];
int lastMoveWasDoubleJump;
int fiftyMoveCounter;
int moveHistory[1000][4];
int moveCount;

void initializeBoard() {
    char initialBoard[SIZE][SIZE] = {
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}
    };

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = initialBoard[i][j];
        }
    }
    
    // Initialize castling rights
    canCastleKingside[0] = canCastleKingside[1] = 1;
    canCastleQueenside[0] = canCastleQueenside[1] = 1;
    
    // Initialize other variables
    lastPawnDoubleMove[0] = lastPawnDoubleMove[1] = -1;
    lastMoveWasDoubleJump = 0;
    fiftyMoveCounter = 0;
    moveCount = 0;
}

void displayBoard() {
    printf("\n  a b c d e f g h\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%d ", 8 - i);
        for (int j = 0; j < SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("%d\n", 8 - i);
    }
    printf("  a b c d e f g h\n\n");
}
