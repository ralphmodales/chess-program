#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "board.h"
#include "moves.h"

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
        printf("Choose your color (W for White, B for Black): ");
        scanf(" %c", &choice);
        clearInputBuffer();  // Clear any remaining input
        
        choice = toupper(choice);
        if (choice == 'W') return 1;  // White
        if (choice == 'B') return 0;  // Black
        printf("Invalid choice. Please enter W or B.\n");
    }
}

int main() {
    char move[6];
    int x1, y1, x2, y2;
    int gameActive = 1;
    int playerColor; 
    int isPlayerTurn;

    initializeBoard();
    playerColor = getPlayerColor();
    printf("You are playing as %s\n", playerColor ? "White" : "Black");

    while (gameActive) {
        displayBoard();
        displayGameStatus();
        
        // Check game ending conditions
        if (isCheckmate(currentPlayer)) {
            printf("Checkmate! %s wins!\n", 
                   currentPlayer == 0 ? "Black" : "White");
            break;
        }
        
        if (isStalemate(currentPlayer)) {
            printf("Stalemate! Game is drawn.\n");
            break;
        }
        
        if (isThreefoldRepetition()) {
            printf("Draw by threefold repetition!\n");
            break;
        }
        
        if (isFiftyMoveDraw()) {
            printf("Draw by fifty-move rule!\n");
            break;
        }
        
        if (hasInsufficientMaterial()) {
            printf("Draw by insufficient material!\n");
            break;
        }
        
        if (isPlayerTurn) {
            // Player's turn
            while (1) {
                getPlayerMove(move);
                
                if (strcmp(move, "quit") == 0) {
                    printf("Game ended by player.\n");
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
                    makeMove(x1, y1, x2, y2);
                    switchTurn();
                    break;
                } else {
                    printf("Invalid move! Try again.\n");
                }
            }
        } else {
            // AI's moves functionality will be add later
            printf("AI is thinking...\n"); 
        }
    }

    displayBoard();
    printf("Game Over!\n");

    return 0;
}
