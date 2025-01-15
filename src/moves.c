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

void findKingPosition(int playerColor, int *kingX, int *kingY) {
    char kingPiece = playerColor == 0 ? 'k' : 'K';
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == kingPiece) {
                *kingX = i;
                *kingY = j;
                return;
            }
        }
    }
}

int isSquareUnderAttack(int x, int y, int attackingColor) {
    // Check attacks from all opponent's pieces
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if (piece == EMPTY) {
                continue;
            }
            
            // For white king (lowercase k), check attacks from black pieces (uppercase)
            // For black king (uppercase K), check attacks from white pieces (lowercase)
            if ((attackingColor == 0 && !isupper(piece)) || 
                (attackingColor == 1 && isupper(piece))) {
                continue;
            }

            // Store current position
            char destPiece = board[x][y];
            board[x][y] = EMPTY;
                
            // Check move validity without recursion
            int isValid = 0;
            char pieceType = toupper(piece);
            switch (pieceType) {
                case 'P': isValid = isPawnMoveValid(i, j, x, y); break;
                case 'N': isValid = isKnightMoveValid(i, j, x, y); break;
                case 'R': isValid = isRookMoveValid(i, j, x, y); break;
                case 'B': isValid = isBishopMoveValid(i, j, x, y); break;
                case 'Q': isValid = isQueenMoveValid(i, j, x, y); break;
                case 'K': isValid = isKingMoveValid(i, j, x, y); break;
            }
                
            // Restore position
            board[x][y] = destPiece;
                
            if (isValid) {
                return 1;
            }
        }
    }
    return 0;
}

int isKingInCheck(int playerColor) {
    int kingX, kingY;
    findKingPosition(playerColor, &kingX, &kingY);
    return isSquareUnderAttack(kingX, kingY, playerColor);
}

int willMoveResultInCheck(int x1, int y1, int x2, int y2, int playerColor) {
    char tempDest = board[x2][y2];
    char tempSrc = board[x1][y1];
    board[x2][y2] = tempSrc;
    board[x1][y1] = EMPTY;
    
    int inCheck = isKingInCheck(playerColor);
    
    board[x1][y1] = tempSrc;
    board[x2][y2] = tempDest;
    
    return inCheck;
}

int isSquareAttackedAfterMove(int fromX, int fromY, int toX, int toY, int x, int y, int playerColor) {
    char tempSrc = board[fromX][fromY];
    char tempDst = board[toX][toY];
    
    // Make temporary move
    board[toX][toY] = tempSrc;
    board[fromX][fromY] = EMPTY;
    
    int isAttacked = isSquareUnderAttack(x, y, playerColor);
    
    // Restore board
    board[fromX][fromY] = tempSrc;
    board[toX][toY] = tempDst;
    
    return isAttacked;
}

int isMovePossible(int x1, int y1, int x2, int y2) {
    // Check if destination square would be under attack after move
    if (toupper(board[x1][y1]) == 'K') {
        if (isSquareAttackedAfterMove(x1, y1, x2, y2, x2, y2, currentPlayer)) {
            return 0;
        }
    } else {
        int kingX, kingY;
        findKingPosition(currentPlayer, &kingX, &kingY);
        if (isSquareAttackedAfterMove(x1, y1, x2, y2, kingX, kingY, currentPlayer)) {
            return 0;
        }
    }
    return 1;
}

int hasLegalMoves(int playerColor) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if (piece == EMPTY) continue;
            
            // Check if piece belongs to current player
            if ((playerColor == 0 && isupper(piece)) || 
                (playerColor == 1 && !isupper(piece))) {
                continue;
            }
            
            // Try all possible destinations
            for (int x = 0; x < SIZE; x++) {
                for (int y = 0; y < SIZE; y++) {
                    // Skip if destination contains own piece
                    if (board[x][y] != EMPTY && 
                        (isupper(board[i][j]) == isupper(board[x][y]))) {
                        continue;
                    }
                    
                    char pieceType = toupper(piece);
                    int moveValid = 0;
                    
                    switch (pieceType) {
                        case 'P': moveValid = isPawnMoveValid(i, j, x, y); break;
                        case 'N': moveValid = isKnightMoveValid(i, j, x, y); break;
                        case 'R': moveValid = isRookMoveValid(i, j, x, y); break;
                        case 'B': moveValid = isBishopMoveValid(i, j, x, y); break;
                        case 'Q': moveValid = isQueenMoveValid(i, j, x, y); break;
                        case 'K': moveValid = isKingMoveValid(i, j, x, y); break;
                    }
                    
                    if (moveValid && isMovePossible(i, j, x, y)) {
                        return 1;  // Found at least one legal move
                    }
                }
            }
        }
    }
    return 0;  // No legal moves found
}

int isCheckmate(int playerColor) {
    if (!isKingInCheck(playerColor)) {
        return 0;
    }
    return !hasLegalMoves(playerColor);
}

int isStalemate(int playerColor) {
    return !isKingInCheck(playerColor) && !hasLegalMoves(playerColor);
}


int isValidMove(int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 >= SIZE || y1 < 0 || y1 >= SIZE ||
        x2 < 0 || x2 >= SIZE || y2 < 0 || y2 >= SIZE) {
        printf("Out of bounds move\n");
        return 0;
    }

    if (board[x1][y1] == EMPTY) {
        printf("No piece at starting position\n");
        return 0;
    }

    if (currentPlayer == 0) { // White's turn
        if (isupper(board[x1][y1])) {
            printf("White must move lowercase pieces\n");
            return 0;
        }
    } else { // Black's turn
        if (!isupper(board[x1][y1])) {
            printf("Black must move uppercase pieces\n");
            return 0;
        }
    }

    if (board[x2][y2] != EMPTY && 
        (isupper(board[x1][y1]) == isupper(board[x2][y2]))) {
        printf("Cannot capture your own piece\n");
        return 0;
    }

    char piece = toupper(board[x1][y1]);
    int moveValid = 0;
    
    switch (piece) {
        case 'P': moveValid = isPawnMoveValid(x1, y1, x2, y2); break;
        case 'N': moveValid = isKnightMoveValid(x1, y1, x2, y2); break;
        case 'R': moveValid = isRookMoveValid(x1, y1, x2, y2); break;
        case 'B': moveValid = isBishopMoveValid(x1, y1, x2, y2); break;
        case 'Q': moveValid = isQueenMoveValid(x1, y1, x2, y2); break;
        case 'K': moveValid = isKingMoveValid(x1, y1, x2, y2); break;
        default: return 0;
    }
    
    if (!moveValid) {
        printf("Invalid move for this piece\n");
        return 0;
    }
    
    if (willMoveResultInCheck(x1, y1, x2, y2, currentPlayer)) {
        printf("This move would leave your king in check!\n");
        return 0;
    }
    
    return 1;
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

void switchTurn() {
    currentPlayer = 1 - currentPlayer;
}

