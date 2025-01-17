#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "ai.h"
#include "board.h"
#include "moves.h"

// Piece-Square tables for sophisticated position evaluation
const int pawnTable[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    70,  70,  70,  80,  80,  70,  70,  70,  // Increased central pawn advancement value
    30,  30,  40,  50,  50,  40,  30,  30,  // Better rewards for controlling center
    10,  10,  20,  35,  35,  20,  10,  10,  
    5,   5,   10,  25,  25,  10,  5,   5,
    0,   0,   0,   5,   5,   0,   0,   0,
    0,   0,   0,  -5,  -5,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

const int knightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -30,   5,  15,  15,  15,  15,   5, -30,  // Slightly reduced central square values
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   0,   5,  10,  10,   5,   0, -30,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

const int bishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,  // Increased diagonal movement values
    -10,   0,  10,  15,  15,  10,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

const int kingTableMiddle[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

const int kingTableEnd[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

// Evaluation weights
#define PAWN_STRUCTURE_WEIGHT 0.4   
#define KING_SAFETY_WEIGHT 0.3      
#define MOBILITY_WEIGHT 0.3         
#define CENTER_CONTROL_WEIGHT 0.5   
#define DEVELOPMENT_WEIGHT 0.4

// Opening book
#define MAX_OPENING_MOVES 1000
#define MAX_MOVE_SEQUENCE 10

typedef struct {
    char moves[MAX_MOVE_SEQUENCE][5]; // Each move is 4 characters (e.g., "e2e4") + null terminator
    int moveCount;                    // Number of moves in this sequence
} MoveSequence;

MoveSequence openingBook[MAX_OPENING_MOVES];
int openingBookSize = 0;

char lastMoves[MAX_MOVE_SEQUENCE][5]; // Stores the last moves
int lastMoveCount = 0;

void loadOpenings() {
    FILE *file = fopen("openings.txt", "r");
    if (!file) {
        perror("Failed to open opening book");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if (openingBookSize >= MAX_OPENING_MOVES) break;

        line[strcspn(line, "\n")] = '\0'; // Remove newline character

        MoveSequence sequence;
        sequence.moveCount = 0;

        char *token = strtok(line, " ");
        while (token != NULL && sequence.moveCount < MAX_MOVE_SEQUENCE) {
            strncpy(sequence.moves[sequence.moveCount], token, 5);
            sequence.moveCount++;
            token = strtok(NULL, " ");
        }

        openingBook[openingBookSize] = sequence;
        openingBookSize++;
    }

    fclose(file);
}

void recordMove(int fromX, int fromY, int toX, int toY) {
    if (lastMoveCount >= MAX_MOVE_SEQUENCE) {
        // Shift moves to make room for the new move
        for (int i = 1; i < MAX_MOVE_SEQUENCE; i++) {
            strncpy(lastMoves[i - 1], lastMoves[i], 5);
        }
        lastMoveCount--;
    }

    // Convert board coordinates to algebraic notation
    // Use snprintf to prevent buffer overflow and ensure null termination
    char move[25];  // Increased buffer size to 25 bytes
    snprintf(move, sizeof(move), "%c%d%c%d", 
             'a' + fromY, 8 - fromX,
             'a' + toY, 8 - toX);
    
    // Use strncpy to safely copy the move
    strncpy(lastMoves[lastMoveCount], move, 5);
    lastMoves[lastMoveCount][4] = '\0';  // Ensure null termination
    lastMoveCount++;
}

int getMoveCount() {
    return lastMoveCount;
}

int getOpeningMove(int *fromX, int *fromY, int *toX, int *toY) {
    // Return 0 if we're out of book moves
    if (lastMoveCount >= MAX_MOVE_SEQUENCE) return 0;
    
    // Find all matching sequences up to current position
    int matchingSequences[MAX_OPENING_MOVES];
    int matchCount = 0;
    
    for (int i = 0; i < openingBookSize; i++) {
        bool matches = true;
        // Check if this sequence matches our game so far
        for (int j = 0; j < lastMoveCount; j++) {
            if (j >= openingBook[i].moveCount || 
                strcmp(lastMoves[j], openingBook[i].moves[j]) != 0) {
                matches = false;
                break;
            }
        }
        
        // If we found a match and it has more moves
        if (matches && openingBook[i].moveCount > lastMoveCount) {
            matchingSequences[matchCount++] = i;
        }
    }
    
    // If we found matching sequences, randomly choose one
    if (matchCount > 0) {
        int chosen = rand() % matchCount;
        int seqIndex = matchingSequences[chosen];
        char *nextMove = openingBook[seqIndex].moves[lastMoveCount];
        
        // Convert algebraic notation back to board coordinates
        *fromY = nextMove[0] - 'a';
        *fromX = '8' - nextMove[1];
        *toY = nextMove[2] - 'a';
        *toX = '8' - nextMove[3];
        
        // Verify move is legal before returning
        if (isValidMove(*fromX, *fromY, *toX, *toY)) {
            return 1;
        }
    }
    
    return 0;
}

int getEarlyGameDevelopmentBonus(char piece, int x, int y, int moveCount) {
    if (moveCount > 10) return 0;  // Only apply in first 10 moves
    
    int bonus = 0;
    switch(toupper(piece)) {
        case 'P':
            // Bonus for central pawns moving forward
            if ((y == 3 || y == 4) && (x == 3 || x == 4)) {
                bonus += 25;
            }
            break;
        case 'N':
        case 'B':
            // Development bonus for minor pieces
            if (x != 0 && x != 7) {  // If piece has moved from back rank
                bonus += 20;
            }
            break;
        case 'Q':
            // Small penalty for early queen development
            if (x != 0 && x != 7) {
                bonus -= 10;
            }
            break;
    }
    return bonus;
}

// Enhanced position evaluation
int evaluatePosition() {
    int score = 0;
    int materialCount = 0;
    int pawnStructureScore = 0;
    int mobilityScore = 0;
    int kingSafetyScore = 0;
    int centerControlScore = 0;
    int developmentScore = 0;
    
    // Get move count for development evaluation
    int moveCount = getMoveCount();  
    
    // Count material and get basic positioning
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if(piece == EMPTY) continue;
            
            int square = i * 8 + j;
            int pieceValue = 0;
            int isWhite = isupper(piece);
            
            // Material and piece-square tables evaluation
            switch(toupper(piece)) {
                case 'P':
                    pieceValue = PAWN_VALUE;
                    pieceValue += isWhite ? pawnTable[square] : -pawnTable[63-square];
                    materialCount += PAWN_VALUE;
                    break;
                case 'N':
                    pieceValue = KNIGHT_VALUE;
                    pieceValue += isWhite ? knightTable[square] : -knightTable[63-square];
                    materialCount += KNIGHT_VALUE;
                    break;
                case 'B':
                    pieceValue = BISHOP_VALUE;
                    pieceValue += isWhite ? bishopTable[square] : -bishopTable[63-square];
                    materialCount += BISHOP_VALUE;
                    break;
                case 'R':
                    pieceValue = ROOK_VALUE;
                    materialCount += ROOK_VALUE;
                    break;
                case 'Q':
                    pieceValue = QUEEN_VALUE;
                    materialCount += QUEEN_VALUE;
                    break;
                case 'K':
                    // Use different king tables for middle/endgame
                    if(materialCount < 3000) {
                        pieceValue += isWhite ? kingTableEnd[square] : -kingTableEnd[63-square];
                    } else {
                        pieceValue += isWhite ? kingTableMiddle[square] : -kingTableMiddle[63-square];
                    }
                    break;
            }
            
            developmentScore += getEarlyGameDevelopmentBonus(piece, i, j, moveCount);
            score += isWhite ? pieceValue : -pieceValue; 

            if (moveCount <= 10) {  // Early game focus on center
              int centralPawns = 0;
              for (int i = 2; i <= 5; i++) {
                for (int j = 2; j <= 5; j++) {
                  if (board[i][j] == 'P') centralPawns++;
                  if (board[i][j] == 'p') centralPawns--;
                }
            }
            centerControlScore += centralPawns * 15;
          }
            // Pawn structure evaluation
            if(toupper(piece) == 'P') {
                // Doubled pawns penalty
                for(int k = 0; k < SIZE; k++) {
                    if(k != i && board[k][j] == piece) {
                        pawnStructureScore -= 20;
                    }
                }
                
                // Isolated pawns penalty
                bool isolated = true;
                for(int adj = j-1; adj <= j+1; adj += 2) {
                    if(adj >= 0 && adj < SIZE) {
                        for(int k = 0; k < SIZE; k++) {
                            if(board[k][adj] == piece) {
                                isolated = false;
                                break;
                            }
                        }
                    }
                }
                if(isolated) pawnStructureScore -= 30;
            }
        }
    }
    
    // Mobility evaluation with piece-specific weights
    int originalPlayer = currentPlayer;
    currentPlayer = 1; // White
    int whiteMoves = 0;
    int blackMoves = 0;
    
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if(piece == EMPTY) continue;
            
            for(int x = 0; x < SIZE; x++) {
                for(int y = 0; y < SIZE; y++) {
                    if(isValidMove(i, j, x, y)) {
                        int moveValue = 1;
                        switch(toupper(piece)) {
                            case 'Q': moveValue = 4; break;
                            case 'R': moveValue = 3; break;
                            case 'B':
                            case 'N': moveValue = 2; break;
                        }
                        if(isupper(piece)) whiteMoves += moveValue;
                        else blackMoves += moveValue;
                    }
                }
            }
        }
    }
    
    mobilityScore = (whiteMoves - blackMoves) * 10;
    
    // Center control evaluation
    int centerSquares[4][2] = {{3,3}, {3,4}, {4,3}, {4,4}};
    for(int i = 0; i < 4; i++) {
        char piece = board[centerSquares[i][0]][centerSquares[i][1]];
        if(piece != EMPTY) {
            centerControlScore += isupper(piece) ? 30 : -30;
        }
        // Also consider attacks on center
        for(int x = 0; x < SIZE; x++) {
            for(int y = 0; y < SIZE; y++) {
                char attacker = board[x][y];
                if(attacker != EMPTY && isValidMove(x, y, centerSquares[i][0], centerSquares[i][1])) {
                    centerControlScore += isupper(attacker) ? 15 : -15;
                }
            }
        }
    }
    
    currentPlayer = originalPlayer;
    
    // Combine all evaluation components
    return score + 
           (int)(pawnStructureScore * PAWN_STRUCTURE_WEIGHT) +
           (int)(mobilityScore * MOBILITY_WEIGHT) +
           (int)(kingSafetyScore * KING_SAFETY_WEIGHT) +
           (int)(centerControlScore * CENTER_CONTROL_WEIGHT) +
           (int)(developmentScore * DEVELOPMENT_WEIGHT);
}

// Principal Variation Search with Quiescence
int quiescence(int alpha, int beta, int depth) {
    int standPat = evaluatePosition();
    
    if(standPat >= beta) return beta;
    if(alpha < standPat) alpha = standPat;
    if(depth <= -3) return alpha; // Limit quiescence depth
    
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if(piece == EMPTY || 
              (currentPlayer == 0 && !isupper(piece)) ||
              (currentPlayer == 1 && isupper(piece))) continue;
            
            for(int x = 0; x < SIZE; x++) {
                for(int y = 0; y < SIZE; y++) {
                    // Only consider captures in quiescence search
                    if(isValidMove(i, j, x, y) && board[x][y] != EMPTY) {
                        char tempDest = board[x][y];
                        board[x][y] = board[i][j];
                        board[i][j] = EMPTY;
                        
                        currentPlayer = !currentPlayer;
                        int score = -quiescence(-beta, -alpha, depth - 1);
                        currentPlayer = !currentPlayer;
                        
                        board[i][j] = board[x][y];
                        board[x][y] = tempDest;
                        
                        if(score >= beta) return beta;
                        if(score > alpha) alpha = score;
                    }
                }
            }
        }
    }
    return alpha;
}

// Principal Variation Search
int pvSearch(int depth, int alpha, int beta, int maximizing) {
    if(depth <= 0) return quiescence(alpha, beta, 0);
    
    int oldAlpha = alpha;
    int score;
    bool foundPV = false;
    
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if(piece == EMPTY || 
              (maximizing && !isupper(piece)) ||
              (!maximizing && isupper(piece))) continue;
            
            for(int x = 0; x < SIZE; x++) {
                for(int y = 0; y < SIZE; y++) {
                    if(isValidMove(i, j, x, y)) {
                        char tempDest = board[x][y];
                        board[x][y] = board[i][j];
                        board[i][j] = EMPTY;
                        
                        if(!foundPV) {
                            score = -pvSearch(depth - 1, -beta, -alpha, !maximizing);
                        } else {
                            score = -pvSearch(depth - 1, -alpha - 1, -alpha, !maximizing);
                            if(score > alpha && score < beta) {
                                score = -pvSearch(depth - 1, -beta, -alpha, !maximizing);
                            }
                        }
                        
                        board[i][j] = board[x][y];
                        board[x][y] = tempDest;
                        
                        if(score >= beta) return beta;
                        if(score > alpha) {
                            alpha = score;
                            foundPV = true;
                        }
                    }
                }
            }
        }
    }
    
    if(alpha == oldAlpha) {
        // No moves were found
        if(isKingInCheck(currentPlayer)) {
            return -INFINITY_SCORE + (MAX_DEPTH - depth); // Prefer later checkmate
        }
        return 0; // Stalemate
    }
    
    return alpha;
}

// Get AI move
int getAIMove(int *fromX, int *fromY, int *toX, int *toY) {
    static int openingPhase = 1;

    if (openingPhase) {
        if (getOpeningMove(fromX, fromY, toX, toY)) {
            if (isValidMove(*fromX, *fromY, *toX, *toY)) {
                return 1;
            } else {
                openingPhase = 0; // Fall back to regular search if the move is invalid
            }
        } else {
            openingPhase = 0; // No more opening moves
        }
    }

    // Fall back to regular search if no opening move is found
    int bestScore = -INFINITY_SCORE;
    int originalPlayer = currentPlayer;

    *fromX = -1;
    *fromY = -1;
    *toX = -1;
    *toY = -1;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if (piece == EMPTY || 
                (originalPlayer == 0 && isupper(piece)) ||
                (originalPlayer == 1 && !isupper(piece))) {
                continue;
            }

            for (int x = 0; x < SIZE; x++) {
                for (int y = 0; y < SIZE; y++) {
                    if (isValidMove(i, j, x, y)) {
                        char tempDest = board[x][y];
                        board[x][y] = board[i][j];
                        board[i][j] = EMPTY;

                        int score = -pvSearch(MAX_DEPTH - 1, -INFINITY_SCORE, INFINITY_SCORE, false);

                        board[i][j] = board[x][y];
                        board[x][y] = tempDest;

                        if (score > bestScore) {
                            bestScore = score;
                            *fromX = i;
                            *fromY = j;
                            *toX = x;
                            *toY = y;
                        }
                    }
                }
            }
        }
    }

    return (*fromX != -1);
}
