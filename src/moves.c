#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "moves.h"
#include "board.h"

// Helper function to check if path is clear between two squares
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

// Individual piece move validation
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
    } else if (abs(y2 - y1) == 1 && x2 == x1 + direction) {  // Capture or en passant
        if (board[x2][y2] != EMPTY) {  // Regular capture
            return (isupper(board[x1][y1]) != isupper(board[x2][y2]));
        } else if (isEnPassantMove(x1, y1, x2, y2)) {  // En passant
            return 1;
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
    if (isCastlingMove(x1, y1, x2, y2)) {
        return canCastle(y2 > y1, currentPlayer);
    }
    return abs(x2 - x1) <= 1 && abs(y2 - y1) <= 1;
}

// Game state checking functions
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
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if (piece == EMPTY) continue;
            
            if ((attackingColor == 0 && !isupper(piece)) || 
                (attackingColor == 1 && isupper(piece))) {
                continue;
            }

            char pieceType = toupper(piece);
            switch (pieceType) {
                case 'P':
                    if (isPawnMoveValid(i, j, x, y)) return 1;
                    break;
                case 'N':
                    if (isKnightMoveValid(i, j, x, y)) return 1;
                    break;
                case 'R':
                    if (isRookMoveValid(i, j, x, y)) return 1;
                    break;
                case 'B':
                    if (isBishopMoveValid(i, j, x, y)) return 1;
                    break;
                case 'Q':
                    if (isQueenMoveValid(i, j, x, y)) return 1;
                    break;
                case 'K':
                    if (abs(x - i) <= 1 && abs(y - j) <= 1) return 1;
                    break;
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

// Special moves
int isCastlingMove(int x1, int y1, int x2, int y2) {
    char piece = board[x1][y1];
    if (toupper(piece) != 'K') return 0;
    return abs(y2 - y1) == 2 && x1 == x2;
}

int canCastle(int kingside, int playerColor) {
    int rank = playerColor == 0 ? 7 : 0;
    char king = playerColor == 0 ? 'k' : 'K';
    
    // Check if castling rights are still available
    if (kingside && !canCastleKingside[playerColor]) return 0;
    if (!kingside && !canCastleQueenside[playerColor]) return 0;
    
    // Check if king and rook are in correct positions
    if (board[rank][4] != king) return 0;
    if (kingside && board[rank][7] != (playerColor == 0 ? 'r' : 'R')) return 0;
    if (!kingside && board[rank][0] != (playerColor == 0 ? 'r' : 'R')) return 0;
    
    // Check if path is clear and not under attack
    int start = kingside ? 5 : 1;
    int end = kingside ? 6 : 3;
    for (int y = start; y <= end; y++) {
        if (board[rank][y] != EMPTY) return 0;
        if (isSquareUnderAttack(rank, y, playerColor)) return 0;
    }
    
    // Check if king is in check
    if (isKingInCheck(playerColor)) return 0;
    
    return 1;
}

void performCastling(int x1, int y1, int x2, int y2) {
    int isKingside = y2 > y1;
    int rank = x1;
    
    // Move king
    board[x2][y2] = board[x1][y1];
    board[x1][y1] = EMPTY;
    
    // Move rook
    if (isKingside) {
        board[rank][5] = board[rank][7];
        board[rank][7] = EMPTY;
    } else {
        board[rank][3] = board[rank][0];
        board[rank][0] = EMPTY;
    }
}

int isEnPassantMove(int x1, int y1, int x2, int y2) {
    char piece = board[x1][y1];
    if (toupper(piece) != 'P') return 0;
    
    int direction = (isupper(piece)) ? 1 : -1;
    if (x2 != x1 + direction) return 0;
    
    // Check if it's a diagonal move to an empty square
    if (abs(y2 - y1) != 1 || board[x2][y2] != EMPTY) return 0;
    
    // Check if there's an enemy pawn in the correct position that just moved
    char enemyPawn = (direction == 1) ? 'p' : 'P';
    return board[x1][y2] == enemyPawn && 
           lastMoveWasDoubleJump && 
           lastPawnDoubleMove[1-currentPlayer] == y2;
}

void performEnPassant(int x1, int y1, int x2, int y2) {
    // Move the pawn
    board[x2][y2] = board[x1][y1];
    board[x1][y1] = EMPTY;
    
    // Remove the captured pawn
    board[x1][y2] = EMPTY;
}

int isPawnPromotion(int x1, int y1, int x2, int y2) {
    (void)y1;  
    (void)y2;
    char piece = board[x1][y1];
    if (toupper(piece) != 'P') return 0;
    return (x2 == 0 && !isupper(piece)) || (x2 == 7 && isupper(piece));
}

void promotePawn(int x2, int y2) {
    char validPieces[] = "QRBN";
    char piece;
    
    while (1) {
        printf("Choose promotion piece (Q/R/B/N): ");
        scanf(" %c", &piece);
        piece = toupper(piece);
        
        if (strchr(validPieces, piece) != NULL) {
            break;
        }
        printf("Invalid piece. Please choose Q (Queen), R (Rook), B (Bishop), or N (Knight)\n");
    }
    
    // Convert to correct case based on player
    if (currentPlayer == 0) {
        piece = tolower(piece);
    }
    
    board[x2][y2] = piece;
}

// Move validation and execution
int isValidMove(int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 >= SIZE || y1 < 0 || y1 >= SIZE ||
        x2 < 0 || x2 >= SIZE || y2 < 0 || y2 >= SIZE) {
        return 0;
    }

    if (board[x1][y1] == EMPTY) return 0;

    // Check if moving correct color piece
    if (currentPlayer == 0 && isupper(board[x1][y1])) return 0;
    if (currentPlayer == 1 && !isupper(board[x1][y1])) return 0;

    // Check if capturing own piece
    if (board[x2][y2] != EMPTY && 
        (isupper(board[x1][y1]) == isupper(board[x2][y2]))) {
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
    }
    
    if (!moveValid) return 0;

    // Test if move would result in check
    char tempDest = board[x2][y2];
    char tempSrc = board[x1][y1];
    board[x2][y2] = tempSrc;
    board[x1][y1] = EMPTY;
    
    int inCheck = isKingInCheck(currentPlayer);
    
    board[x1][y1] = tempSrc;
    board[x2][y2] = tempDest;
    
    return !inCheck;
}

void makeMove(int x1, int y1, int x2, int y2) {
    // Update fifty move counter
    if (toupper(board[x1][y1]) == 'P' || board[x2][y2] != EMPTY) {
        fiftyMoveCounter = 0;
    } else {
        fiftyMoveCounter++;
    }
    
    // Store move in history
    moveHistory[moveCount][0] = x1;
    moveHistory[moveCount][1] = y1;
    moveHistory[moveCount][2] = x2;
    moveHistory[moveCount][3] = y2;
    moveCount++;
    
    // Handle special moves
    if (isCastlingMove(x1, y1, x2, y2)) {
        performCastling(x1, y1, x2, y2);
    } else if (isEnPassantMove(x1, y1, x2, y2)) {
        performEnPassant(x1, y1, x2, y2);
    } else {
        // Regular move
        board[x2][y2] = board[x1][y1];
        board[x1][y1] = EMPTY;
    }
    
    // Handle pawn promotion
    if (isPawnPromotion(x1, y1, x2, y2)) {
        promotePawn(x2, y2);
    }
    
    // Update castling rights
    if (toupper(board[x2][y2]) == 'K') {
        canCastleKingside[currentPlayer] = 0;
        canCastleQueenside[currentPlayer] = 0;
    } else if (toupper(board[x2][y2]) == 'R') {
        if (y1 == 0) canCastleQueenside[currentPlayer] = 0;
        if (y1 == 7) canCastleKingside[currentPlayer] = 0;
    }
    
    // Update en passant information
    lastMoveWasDoubleJump = 0;
    if (toupper(board[x2][y2]) == 'P' && abs(x2 - x1) == 2) {
        lastPawnDoubleMove[currentPlayer] = y2;
        lastMoveWasDoubleJump = 1;
    }
}

void convertNotation(const char *move, int *x1, int *y1, int *x2, int *y2) {
    *y1 = tolower(move[0]) - 'a';
    *x1 = 8 - (move[1] - '0');
    *y2 = tolower(move[3]) - 'a';
    *x2 = 8 - (move[4] - '0');
}

int isThreefoldRepetition() {
    if (moveCount < 8) return 0;  // Need at least 8 moves for a repetition
    
    int repetitions = 1;  // Count current position
    for (int i = 0; i < moveCount - 7; i += 2) {  // Check every full move
        if (moveHistory[i][0] == moveHistory[moveCount-1][0] &&
            moveHistory[i][1] == moveHistory[moveCount-1][1] &&
            moveHistory[i][2] == moveHistory[moveCount-1][2] &&
            moveHistory[i][3] == moveHistory[moveCount-1][3]) {
            repetitions++;
            if (repetitions >= 3) return 1;
        }
    }
    return 0;
}

int isFiftyMoveDraw() {
    return fiftyMoveCounter >= 100;  // 50 moves by each player = 100 half-moves
}

int hasInsufficientMaterial() {
    int pieces[2][6] = {0};  // [color][piece type]
    int totalPieces = 0;
    
    // Count all pieces
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char piece = board[i][j];
            if (piece == EMPTY) continue;
            
            int color = isupper(piece) ? 1 : 0;
            switch(toupper(piece)) {
                case 'P': pieces[color][0]++; break;
                case 'N': pieces[color][1]++; break;
                case 'B': pieces[color][2]++; break;
                case 'R': pieces[color][3]++; break;
                case 'Q': pieces[color][4]++; break;
                case 'K': pieces[color][5]++; break;
            }
            totalPieces++;
        }
    }
    
    // King vs King
    if (totalPieces == 2) return 1;
    
    // King + minor piece vs King
    if (totalPieces == 3 && 
        (pieces[0][1] + pieces[0][2] + pieces[1][1] + pieces[1][2] == 1)) {
        return 1;
    }
    
    // King + Bishop vs King + Bishop (same colored bishops)
    if (totalPieces == 4 && 
        pieces[0][2] + pieces[1][2] == 2 && 
        pieces[0][0] + pieces[0][1] + pieces[0][3] + pieces[0][4] +
        pieces[1][0] + pieces[1][1] + pieces[1][3] + pieces[1][4] == 0) {
        // TODO: Check if bishops are on same colored squares
        return 1;
    }
    
    return 0;
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
                    if (isValidMove(i, j, x, y)) {
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
    if (isKingInCheck(playerColor)) {
        return 0;
    }
    return !hasLegalMoves(playerColor);
}

void switchTurn() {
    currentPlayer = 1 - currentPlayer;
}

