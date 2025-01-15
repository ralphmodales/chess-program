#include <stdio.h>
#include <string.h>
#include "board.h"
#include "moves.h"

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    char move[6];
    int x1, y1, x2, y2;

    initializeBoard();

    while (1) {
        displayBoard();
        
        // Check if the current player is in check
        if (isKingInCheck(currentPlayer)) {
            printf("Your king is in check!\n");
        }
        
        printf("%s's turn. Enter your move (e.g., e2 e4): ", 
               currentPlayer == 0 ? "White" : "Black");
               
        fgets(move, sizeof(move), stdin);
        move[strcspn(move, "\n")] = '\0';
        clearInputBuffer();

        convertNotation(move, &x1, &y1, &x2, &y2);

        if (isValidMove(x1, y1, x2, y2)) {
            makeMove(x1, y1, x2, y2);
            
            // Check if the opponent is now in check
            if (isKingInCheck(1 - currentPlayer)) {
                printf("Check!\n");
            }
            
            switchTurn();
        } else {
            printf("Invalid move! Try again.\n");
        }
    }

    return 0;
}

