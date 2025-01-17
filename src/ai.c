#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "ai.h"
#include "board.h"
#include "moves.h"

// Move history to track repetition
#define MOVE_HISTORY_SIZE 5
char moveHistory[MOVE_HISTORY_SIZE][5];
int moveHistoryCount = 0;

// Penalty for repeating moves
#define REPETITION_PENALTY 50

// Piece-square tables and other constants remain unchanged
const int pawnTable[64] = { /* ... */ };
const int knightTable[64] = { /* ... */ };
const int bishopTable[64] = { /* ... */ };
const int kingTableMiddle[64] = { /* ... */ };
const int kingTableEnd[64] = { /* ... */ };

#define CENTER_CONTROL_WEIGHT 0.6
#define DEVELOPMENT_WEIGHT 0.5
#define KING_SAFETY_WEIGHT 0.4
#define PAWN_STRUCTURE_WEIGHT 0.5
#define PIECE_COORDINATION_WEIGHT 0.4
#define MOBILITY_WEIGHT 0.4
#define CONNECTED_ROOKS_BONUS 30

#define MAX_OPENING_MOVES 1000
#define MAX_MOVE_SEQUENCE 10

typedef struct {
    char moves[MAX_MOVE_SEQUENCE][5];
    int moveCount;
} MoveSequence;

MoveSequence openingBook[MAX_OPENING_MOVES];
int openingBookSize = 0;

char lastMoves[MAX_MOVE_SEQUENCE][5];
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

        line[strcspn(line, "\n")] = '\0';

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
    // Update move history
    if (moveHistoryCount >= MOVE_HISTORY_SIZE) {
        for (int i = 1; i < MOVE_HISTORY_SIZE; i++) {
            strncpy(moveHistory[i - 1], moveHistory[i], 5);
        }
        moveHistoryCount--;
    }

    char move[25];
    snprintf(move, sizeof(move), "%c%d%c%d", 
             'a' + fromY, 8 - fromX,
             'a' + toY, 8 - toX);
    
    strncpy(moveHistory[moveHistoryCount], move, 5);
    moveHistory[moveHistoryCount][4] = '\0';
    moveHistoryCount++;

    // Update last moves for opening book
    if (lastMoveCount >= MAX_MOVE_SEQUENCE) {
        for (int i = 1; i < MAX_MOVE_SEQUENCE; i++) {
            strncpy(lastMoves[i - 1], lastMoves[i], 5);
        }
        lastMoveCount--;
    }

    strncpy(lastMoves[lastMoveCount], move, 5);
    lastMoves[lastMoveCount][4] = '\0';
    lastMoveCount++;
}

int getMoveCount() {
    return lastMoveCount;
}

int getOpeningMove(int *fromX, int *fromY, int *toX, int *toY) {
    if (lastMoveCount >= MAX_MOVE_SEQUENCE) return 0;
    
    int matchingSequences[MAX_OPENING_MOVES];
    int matchCount = 0;
    
    for (int i = 0; i < openingBookSize; i++) {
        bool matches = true;
        for (int j = 0; j < lastMoveCount; j++) {
            if (j >= openingBook[i].moveCount || 
                strcmp(lastMoves[j], openingBook[i].moves[j]) != 0) {
                matches = false;
                break;
            }
        }
        
        if (matches && openingBook[i].moveCount > lastMoveCount) {
            matchingSequences[matchCount++] = i;
        }
    }
    
    if (matchCount > 0) {
        int chosen = rand() % matchCount;
        int seqIndex = matchingSequences[chosen];
        char *nextMove = openingBook[seqIndex].moves[lastMoveCount];
        
        *fromY = nextMove[0] - 'a';
        *fromX = '8' - nextMove[1];
        *toY = nextMove[2] - 'a';
        *toX = '8' - nextMove[3];
        
        if (isValidMove(*fromX, *fromY, *toX, *toY)) {
            return 1;
        }
    }
    
    return 0;
}

int evaluateKingSafety() {
    int whiteKingSafety = 0;
    int blackKingSafety = 0;
    
    int whiteKingX = -1, whiteKingY = -1;
    int blackKingX = -1, blackKingY = -1;
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == 'K') { whiteKingX = i; whiteKingY = j; }
            if (board[i][j] == 'k') { blackKingX = i; blackKingY = j; }
        }
    }
    
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            int wx = whiteKingX + x;
            int wy = whiteKingY + y;
            int bx = blackKingX + x;
            int by = blackKingY + y;
            
            if (wx >= 0 && wx < 8 && wy >= 0 && wy < 8) {
                if (board[wx][wy] == 'P') whiteKingSafety += 10;
                if (islower(board[wx][wy])) whiteKingSafety -= 20;
            }
            
            if (bx >= 0 && bx < 8 && by >= 0 && by < 8) {
                if (board[bx][by] == 'p') blackKingSafety += 10;
                if (isupper(board[bx][by])) blackKingSafety -= 20;
            }
        }
    }
    
    return whiteKingSafety - blackKingSafety;
}

int evaluatePieceCoordination() {
    int whiteCoordination = 0;
    int blackCoordination = 0;
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if (piece == EMPTY) continue;
            
            for (int x = 0; x < SIZE; x++) {
                for (int y = 0; y < SIZE; y++) {
                    if (isValidMove(i, j, x, y)) {
                        char targetPiece = board[x][y];
                        if (targetPiece != EMPTY) {
                            if (isupper(piece) == isupper(targetPiece)) {
                                if (isupper(piece)) whiteCoordination += 5;
                                else blackCoordination += 5;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return whiteCoordination - blackCoordination;
}

int evaluatePosition() {
    int score = 0;
    int whiteDevelopedPieces = 0, blackDevelopedPieces = 0;
    int centerControl = 0;
    
    // Check for move repetition
    if (moveHistoryCount >= 2) {
        for (int i = 0; i < moveHistoryCount - 1; i++) {
            if (strcmp(moveHistory[i], moveHistory[moveHistoryCount - 1]) == 0) {
                score -= REPETITION_PENALTY;
                break;
            }
        }
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            
            if ((piece == 'N' || piece == 'B') && i != 7) whiteDevelopedPieces++;
            if ((piece == 'n' || piece == 'b') && i != 0) blackDevelopedPieces++;
            
            if ((i == 3 || i == 4) && (j == 3 || j == 4)) {
                if (isupper(piece)) centerControl++;
                else if (islower(piece)) centerControl--;
            }
        }
    }
    
    int moveCount = getMoveCount();
    if (moveCount < 10) {
        score += (whiteDevelopedPieces - blackDevelopedPieces) * 20;
        
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (board[i][j] == 'Q' && i != 7) score -= 30;
                if (board[i][j] == 'q' && i != 0) score += 30;
            }
        }
    }
    
    int whiteKingSafety = evaluateKingSafety();
    score += whiteKingSafety * KING_SAFETY_WEIGHT;
    
    int coordination = evaluatePieceCoordination();
    score += coordination * PIECE_COORDINATION_WEIGHT;
    
    score += evaluateConnectedRooks();
    
    score += evaluatePawnStructure() * PAWN_STRUCTURE_WEIGHT;
    
    score += centerControl * CENTER_CONTROL_WEIGHT * 10;
    
    return score;
}

int evaluateConnectedRooks() {
    int score = 0;
    bool whiteRooksConnected = false;
    bool blackRooksConnected = false;
    
    for (int i = 0; i < SIZE; i++) {
        int whiteRookCount = 0;
        int blackRookCount = 0;
        bool blockedPieces = false;
        
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == 'R') whiteRookCount++;
            else if (board[i][j] == 'r') blackRookCount++;
            else if (board[i][j] != '.') blockedPieces = true;
        }
        
        if (whiteRookCount == 2 && !blockedPieces) whiteRooksConnected = true;
        if (blackRookCount == 2 && !blockedPieces) blackRooksConnected = true;
    }
    
    if (whiteRooksConnected) score += CONNECTED_ROOKS_BONUS;
    if (blackRooksConnected) score -= CONNECTED_ROOKS_BONUS;
    
    return score;
}

int evaluatePawnStructure() {
    int score = 0;
    
    for (int j = 0; j < SIZE; j++) {
        bool whitePawnOnFile = false;
        bool blackPawnOnFile = false;
        
        for (int i = 0; i < SIZE; i++) {
            if (board[i][j] == 'P') whitePawnOnFile = true;
            if (board[i][j] == 'p') blackPawnOnFile = true;
            
            if (whitePawnOnFile && board[i][j] == 'P') score -= 15;
            if (blackPawnOnFile && board[i][j] == 'p') score += 15;
            
            if ((j == 0 || board[i][j-1] != 'P') && 
                (j == 7 || board[i][j+1] != 'P')) score -= 20;
            if ((j == 0 || board[i][j-1] != 'p') && 
                (j == 7 || board[i][j+1] != 'p')) score += 20;
        }
    }
    
    return score;
}

int quiescence(int alpha, int beta, int depth) {
    int standPat = evaluatePosition();
    
    if(standPat >= beta) return beta;
    if(alpha < standPat) alpha = standPat;
    if(depth <= -3) return alpha;
    
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if(piece == EMPTY || 
              (currentPlayer == 0 && !isupper(piece)) ||
              (currentPlayer == 1 && isupper(piece))) continue;
            
            for(int x = 0; x < SIZE; x++) {
                for(int y = 0; y < SIZE; y++) {
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
        if(isKingInCheck(currentPlayer)) {
            return -INFINITY_SCORE + (MAX_DEPTH - depth);
        }
        return 0;
    }
    
    return alpha;
}

int getAIMove(int *fromX, int *fromY, int *toX, int *toY) {
    static int openingPhase = 1;

    if (openingPhase) {
        if (getOpeningMove(fromX, fromY, toX, toY)) {
            if (isValidMove(*fromX, *fromY, *toX, *toY)) {
                return 1;
            } else {
                openingPhase = 0;
            }
        } else {
            openingPhase = 0;
        }
    }

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
