#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "board.h"
#include "moves.h"
#include "ai.h"

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void displayGameStatus() {
    if (isKingInCheck(currentPlayer)) {
        printf("CHECK!\n");
    }
    
    printf("%s to move\n", currentPlayer == 0 ? "White" : "Black");
    
    if (canCastleKingside[currentPlayer] || canCastleQueenside[currentPlayer]) {
        printf("Castling available: %s%s\n",
               canCastleKingside[currentPlayer] ? "O-O " : "",
               canCastleQueenside[currentPlayer] ? "O-O-O" : "");
    }
}

void getPlayerMove(char *move) {
    printf("Enter move (e.g., 'e2 e4'): ");
    fgets(move, 6, stdin);
    move[strcspn(move, "\n")] = '\0';
}

int getPlayerColor() {
    char choice;
    while (1) {
        printf("\nChoose your color:\n");
        printf("W - Play as White (lowercase pieces, moves first)\n");
        printf("B - Play as Black (uppercase pieces)\n");
        printf("Your choice (W/B): ");
        scanf(" %c", &choice);
        clearInputBuffer();
        
        choice = toupper(choice);
        if (choice == 'W') {
            printf("\nYou are playing as White (lowercase pieces)\n");
            return 0;  // White is 0 (lowercase)
        }
        if (choice == 'B') {
            printf("\nYou are playing as Black (uppercase pieces)\n");
            return 1;  // Black is 1 (uppercase)
        }
        printf("Invalid choice. Please enter W or B.\n");
    }
}

void printMoveHistory(int moveNum, const char *move, int isAI) {
    printf("Move %d: %s %s\n", moveNum, isAI ? "AI plays" : "You play", move);
}

void formatMove(int fromX, int fromY, int toX, int toY, char *moveStr) {
    sprintf(moveStr, "%c%d %c%d", 
            'a' + fromY, 8 - fromX,
            'a' + toY, 8 - toX);
}

int main() {
    char move[6];
    int gameActive = 1;
    int playerColor;
    int isPlayerTurn;
    int moveNumber = 1;
    char formattedMove[6];

    initializeBoard();
    loadOpenings(); // Load the openings
    printf("\n=== Welcome to Chess with AI ===\n");
    printf("\nBoard notation:\n");
    printf("- Uppercase (RNBQKP) are Black pieces\n");
    printf("- Lowercase (rnbqkp) are White pieces\n");
    printf("- Enter moves in format 'e2 e4'\n");
    printf("- Type 'quit' to end the game\n");
    
    // Get player's color choice
    playerColor = getPlayerColor();
    
    // Game loop
    while (gameActive) {
        displayBoard();
        displayGameStatus();
        
        // Check game ending conditions
        if (isCheckmate(currentPlayer)) {
            printf("\nCheckmate! %s wins!\n", currentPlayer == 0 ? "Black" : "White");
            break;
        }
        
        if (isStalemate(currentPlayer)) {
            printf("\nStalemate! Game is drawn.\n");
            break;
        }
        
        if (isThreefoldRepetition()) {
            printf("\nDraw by threefold repetition!\n");
            break;
        }
        
        if (isFiftyMoveDraw()) {
            printf("\nDraw by fifty-move rule!\n");
            break;
        }
        
        if (hasInsufficientMaterial()) {
            printf("\nDraw by insufficient material!\n");
            break;
        }
        
        isPlayerTurn = (currentPlayer == playerColor);
        
        if (isPlayerTurn) {
            // Player's turn
            while (1) {
                getPlayerMove(move);
                
                if (strcmp(move, "quit") == 0) {
                    printf("\nGame ended by player.\n");
                    gameActive = 0;
                    break;
                }
                
                // Validate input format
                if (strlen(move) != 5 || move[2] != ' ') {
                    printf("Invalid format. Use 'e2 e4' format.\n");
                    continue;
                }

                int x1, y1, x2, y2;
                convertNotation(move, &x1, &y1, &x2, &y2);

                if (isValidMove(x1, y1, x2, y2)) {
                    printMoveHistory(moveNumber, move, 0);
                    makeMove(x1, y1, x2, y2);
                    switchTurn();
                    if (currentPlayer == 1) moveNumber++; // Increment after Black's move
                    break;
                } else {
                    printf("Invalid move! Try again.\n");
                }
            }
        } else {
            // GonAI's turn
            printf("\nGonAI is thinking...\n");
            int fromX, fromY, toX, toY;
            if (getAIMove(&fromX, &fromY, &toX, &toY)) {
                formatMove(fromX, fromY, toX, toY, formattedMove);
                printMoveHistory(moveNumber, formattedMove, 1);
                makeMove(fromX, fromY, toX, toY);
                switchTurn();
                if (currentPlayer == 1) moveNumber++; // Increment after Black's move
            } else {
                printf("AI couldn't find a valid move!\n");
                break;
            }
        }
        
        // Add a small separator between moves
        printf("\n");
    }

    // Game end
    displayBoard();
    printf("\n=== Game Over! ===\n");

    return 0;
}
