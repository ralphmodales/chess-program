#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "moves.h"
#include "board.h"

int isPathClear(int x1, int y1, int x2, int y2) {
    int dx = (x2 > x1) ? 1 : (x2 < x1) ? -1 : 0;
    int dy = (y2 > y1) ? 1 : (y2 < y1) ? -1 : 0;
    
    int x = x1 + dx;
    int y = y1 + dy;
    
    while (x != x2 || y != y2) {
        if (board[x][y] != EMPTY) {
            return 0;
        }
        x += dx;
        y += dy;
    }
    return 1;
}

int isPawnMoveValid(int x1, int y1, int x2, int y2) {
    int direction = (isupper(board[x1][y1])) ? 1 : -1;
    
    if (y1 == y2) {  // Moving forward
        if (x2 == x1 + direction && board[x2][y2] == EMPTY) {
            return 1;
        }
        // Initial double move
        if ((x1 == 1 && direction == 1) || (x1 == 6 && direction == -1)) {
            if (x2 == x1 + 2 * direction && 
                board[x2][y2] == EMPTY && 
                board[x1 + direction][y2] == EMPTY) {
                return 1;
            }
        }
    } else if (abs(y2 - y1) == 1 && x2 == x1 + direction) {  // Capture
        if (board[x2][y2] != EMPTY) {
            // Check if piece is enemy (uppercase vs lowercase)
            return (isupper(board[x1][y1]) != isupper(board[x2][y2]));
        }
    }
    return 0;
}

int isKnightMoveValid(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    return (dx == 2 && dy == 1) || (dx == 1 && dy == 2);
}

int isRookMoveValid(int x1, int y1, int x2, int y2) {
    if (x1 != x2 && y1 != y2) return 0;
    return isPathClear(x1, y1, x2, y2);
}

int isBishopMoveValid(int x1, int y1, int x2, int y2) {
    if (abs(x2 - x1) != abs(y2 - y1)) return 0;
    return isPathClear(x1, y1, x2, y2);
}

int isQueenMoveValid(int x1, int y1, int x2, int y2) {
    if ((x1 != x2 && y1 != y2) && (abs(x2 - x1) != abs(y2 - y1))) return 0;
    return isPathClear(x1, y1, x2, y2);
}

int isKingMoveValid(int x1, int y1, int x2, int y2) {
    return abs(x2 - x1) <= 1 && abs(y2 - y1) <= 1;
}

int isValidMove(int x1, int y1, int x2, int y2) {
    // Check if the source and destination are within bounds
    if (x1 < 0 || x1 >= SIZE || y1 < 0 || y1 >= SIZE ||
        x2 < 0 || x2 >= SIZE || y2 < 0 || y2 >= SIZE) {
        return 0;
    }

    // Check if the source is not empty
    if (board[x1][y1] == EMPTY) {
        return 0;
    }

    // Check if the destination contains a friendly piece
    if (board[x2][y2] != EMPTY && 
        (isupper(board[x1][y1]) == isupper(board[x2][y2]))) {
        return 0;
    }

    // Piece-specific validation
    char piece = toupper(board[x1][y1]);
    switch (piece) {
        case 'P': return isPawnMoveValid(x1, y1, x2, y2);
        case 'N': return isKnightMoveValid(x1, y1, x2, y2);
        case 'R': return isRookMoveValid(x1, y1, x2, y2);
        case 'B': return isBishopMoveValid(x1, y1, x2, y2);
        case 'Q': return isQueenMoveValid(x1, y1, x2, y2);
        case 'K': return isKingMoveValid(x1, y1, x2, y2);
        default: return 0;
    }
}

void convertNotation(const char *move, int *x1, int *y1, int *x2, int *y2) {
    *y1 = tolower(move[0]) - 'a';
    *x1 = 8 - (move[1] - '0');
    *y2 = tolower(move[3]) - 'a';
    *x2 = 8 - (move[4] - '0');
}

void makeMove(int x1, int y1, int x2, int y2) {
    board[x2][y2] = board[x1][y1];
    board[x1][y1] = EMPTY;
}

