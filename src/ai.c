#include <stdio.h>
#include <ctype.h>
#include "ai.h"
#include "board.h"
#include "moves.h"

// TODO: Improve the AI

// Piece-Square tables for position evaluation
const int pawnTable[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

int evaluatePosition() {
    int score = 0;
    
    // Material counting
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if(piece == EMPTY) continue;
            
            int pieceValue = 0;
            switch(toupper(piece)) {
                case 'P': pieceValue = PAWN_VALUE; break;
                case 'N': pieceValue = KNIGHT_VALUE; break;
                case 'B': pieceValue = BISHOP_VALUE; break;
                case 'R': pieceValue = ROOK_VALUE; break;
                case 'Q': pieceValue = QUEEN_VALUE; break;
                case 'K': pieceValue = 0; break; // King's value is implicit
            }
            
            // Add position bonus for pawns
            if(toupper(piece) == 'P') {
                int tableIndex = i * 8 + j;
                pieceValue += pawnTable[tableIndex];
            }
            
            // Add or subtract based on piece color
            if(isupper(piece)) {
                score += pieceValue;
            } else {
                score -= pieceValue;
            }
        }
    }
    
    // Mobility evaluation (count possible moves)
    int originalPlayer = currentPlayer;
    currentPlayer = 1; // White
    int whiteMoves = 0;
    int blackMoves = 0;
    
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            for(int x = 0; x < SIZE; x++) {
                for(int y = 0; y < SIZE; y++) {
                    if(isValidMove(i, j, x, y)) whiteMoves++;
                }
            }
        }
    }
    
    currentPlayer = 0; // Black
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            for(int x = 0; x < SIZE; x++) {
                for(int y = 0; y < SIZE; y++) {
                    if(isValidMove(i, j, x, y)) blackMoves++;
                }
            }
        }
    }
    
    currentPlayer = originalPlayer;
    score += (whiteMoves - blackMoves) * 10;
    
    return score;
}

// Minimax algorithm with alpha-beta pruning
int minimax(int depth, int alpha, int beta, int maximizing) {
    if(depth == 0) {
        return evaluatePosition();
    }
    
    if(maximizing) {
        int maxEval = -INFINITY_SCORE;
        for(int i = 0; i < SIZE; i++) {
            for(int j = 0; j < SIZE; j++) {
                char piece = board[i][j];
                if(piece == EMPTY || !isupper(piece)) continue;
                
                for(int x = 0; x < SIZE; x++) {
                    for(int y = 0; y < SIZE; y++) {
                        if(isValidMove(i, j, x, y)) {
                            // Make move
                            char tempDest = board[x][y];
                            board[x][y] = board[i][j];
                            board[i][j] = EMPTY;
                            
                            int eval = minimax(depth - 1, alpha, beta, 0);
                            
                            // Undo move
                            board[i][j] = board[x][y];
                            board[x][y] = tempDest;
                            
                            maxEval = (eval > maxEval) ? eval : maxEval;
                            alpha = (alpha > eval) ? alpha : eval;
                            if(beta <= alpha) break;
                        }
                    }
                }
            }
        }
        return maxEval;
    } else {
        int minEval = INFINITY_SCORE;
        for(int i = 0; i < SIZE; i++) {
            for(int j = 0; j < SIZE; j++) {
                char piece = board[i][j];
                if(piece == EMPTY || isupper(piece)) continue;
                
                for(int x = 0; x < SIZE; x++) {
                    for(int y = 0; y < SIZE; y++) {
                        if(isValidMove(i, j, x, y)) {
                            // Make move
                            char tempDest = board[x][y];
                            board[x][y] = board[i][j];
                            board[i][j] = EMPTY;
                            
                            int eval = minimax(depth - 1, alpha, beta, 1);
                            
                            // Undo move
                            board[i][j] = board[x][y];
                            board[x][y] = tempDest;
                            
                            minEval = (eval < minEval) ? eval : minEval;
                            beta = (beta < eval) ? beta : eval;
                            if(beta <= alpha) break;
                        }
                    }
                }
            }
        }
        return minEval;
    }
}

// Function to get the AI's move
int getAIMove(int *fromX, int *fromY, int *toX, int *toY) {
    int bestScore = -INFINITY_SCORE;
    int originalPlayer = currentPlayer;
    
    // Initialize best move to invalid values
    *fromX = -1;
    *fromY = -1;
    *toX = -1;
    *toY = -1;
    
    // Search for best move
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            // Skip empty squares and opponent's pieces
            if(piece == EMPTY || 
               (originalPlayer == 0 && isupper(piece)) ||
               (originalPlayer == 1 && !isupper(piece))) {
                continue;
            }
            
            for(int x = 0; x < SIZE; x++) {
                for(int y = 0; y < SIZE; y++) {
                    if(isValidMove(i, j, x, y)) {
                        // Make move
                        char tempDest = board[x][y];
                        board[x][y] = board[i][j];
                        board[i][j] = EMPTY;
                        
                        int score = minimax(MAX_DEPTH - 1, -INFINITY_SCORE, INFINITY_SCORE, 0);
                        
                        // Undo move
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

