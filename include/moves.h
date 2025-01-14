#ifndef MOVES_H
#define MOVES_H

#include "board.h"

// Convert notation to moves
void convertNotation(const char *move, int *x1, int *y1, int *x2, int *y2);

// Check if a move is a valid (basic for now...)
int isValidMove(int x1, int y1, int x2, int y2);

// Function to make a move
void makeMove(int x1, int y1, int x2, int y2);

#endif // MOVES_H 
