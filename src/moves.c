#include <stdio.h>
#include <string.h>
#include "moves.h"
#include "board.h"

// Convert notation to moves
void convertNotation(const char *move, int *x1, int *y1, int *x2, int *y2) {
    *x1 = 8 - (move[1] - '0'); 
    *y1 = move[0] - 'a';       
    *x2 = 8 - (move[3] - '0');
    *y2 = move[2] - 'a';
}

// Check if a move is a valid (basic for now...)
int isValidMove(int x1, int y1, int x2, int y2) {
    // Check if the source and destination is valid 
    if (x1 < 0 || x1 >= SIZE || y1 < 0 || y1 >= SIZE ||
        x2 < 0 || x2 >= SIZE || y2 < 0 || y2 >= SIZE) {
        return 0;
    }
    // Check if the source is not empty
    if (board[x1][y1] == EMPTY) {
        return 0;
    }
    // Check if the destination is empty or contains an opponent's piece
    if (board[x2][y2] != EMPTY && board[x2][y2] == board[x1][y1]) {
        return 0; // Cannot capture your own piece
    }
    return 1;
}

void makeMove(int x1, int y1, int x2, int y2) {
    board[x2][y2] = board[x1][y1];
    board[x1][y1] = EMPTY;
}
