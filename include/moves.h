#ifndef MOVES_H
#define MOVES_H

#include "board.h"

// Core move validation
int isValidMove(int x1, int y1, int x2, int y2);
void makeMove(int x1, int y1, int x2, int y2);
void convertNotation(const char *move, int *x1, int *y1, int *x2, int *y2);
void switchTurn();

// Special moves
int isCastlingMove(int x1, int y1, int x2, int y2);
int canCastle(int kingside, int playerColor);
int isEnPassantMove(int x1, int y1, int x2, int y2);
int isPawnPromotion(int x1, int y1, int x2, int y2);

// Game state checks
int isKingInCheck(int playerColor);
int isCheckmate(int playerColor);
int isStalemate(int playerColor);
int isThreefoldRepetition();
int isFiftyMoveDraw();
int hasInsufficientMaterial();

#endif // MOVES_H 
