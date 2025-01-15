#ifndef AI_H
#define AI_H

#define MAX_DEPTH 4  // Adjust based on desired strength/speed | 800 - 1000 elo as of now
#define INFINITY_SCORE 1000000

// Piece values for processing of AI
#define PAWN_VALUE 100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE 500
#define QUEEN_VALUE 900

int getAIMove(int *fromX, int *fromY, int *toX, int *toY);
int evaluatePosition(void);
int minimax(int depth, int alpha, int beta, int maximizing);
void loadOpenings(void);

#endif
