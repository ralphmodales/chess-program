#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include "ai.h"
#include "board.h"
#include "moves.h"

// Piece-Square tables for sophisticated position evaluation
const int pawnTable[64] = {
     0,  0,  0,   0,   0,  0,  0,  0,
    50, 50, 50,  50,  50, 50, 50, 50,
    10, 10, 20,  30,  30, 20, 10, 10,
     5,  5, 10,  25,  25, 10,  5,  5,
     0,  0,  0,  20,  20,  0,  0,  0,
     5, -5,-10,   0,   0,-10, -5,  5,
     5, 10, 10, -20, -20, 10, 10,  5,
     0,  0,  0,   0,   0,  0,  0,  0
};

const int knightTable[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
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
#define PAWN_STRUCTURE_WEIGHT 0.3
#define KING_SAFETY_WEIGHT 0.4
#define MOBILITY_WEIGHT 0.2
#define CENTER_CONTROL_WEIGHT 0.3
#define DEVELOPMENT_WEIGHT 0.2

// Enhanced position evaluation
int evaluatePosition() {
    int score = 0;
    int materialCount = 0;
    int pawnStructureScore = 0;
    int mobilityScore = 0;
    int kingSafetyScore = 0;
    int centerControlScore = 0;
    int developmentScore = 0;
    
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
            
            score += isWhite ? pieceValue : -pieceValue;
            
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

// Get AI move using enhanced search
int getAIMove(int *fromX, int *fromY, int *toX, int *toY) {
    int bestScore = -INFINITY_SCORE;
    int originalPlayer = currentPlayer;
    
    *fromX = -1;
    *fromY = -1;
    *toX = -1;
    *toY = -1;
    
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if(piece == EMPTY || 
               (originalPlayer == 0 && isupper(piece)) ||
               (originalPlayer == 1 && !isupper(piece))) {
                continue;
            }
            
            for(int x = 0; x < SIZE; x++) {
                for(int y = 0; y < SIZE; y++) {
                    if(isValidMove(i, j, x, y)) {
                        char tempDest = board[x][y];
                        board[x][y] = board[i][j];
                        board[i][j] = EMPTY;
                        
                        int score = -pvSearch(MAX_DEPTH - 1, -INFINITY_SCORE, INFINITY_SCORE, false);
                        
                        board[i][j] = board[x][y];
                        board[x][y] = tempDest;
                        
                        if(score > bestScore) {
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
    
    // Return 1 if a valid move was found
    return (*fromX != -1);
}
