#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

int main() {
    char move[6];
    int x1, y1, x2, y2;
    int gameActive = 1;

    initializeBoard();
    printf("Chess Game Started!\n");
    printf("Enter moves in format 'e2 e4' or type 'quit' to exit\n\n");

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
        
        printf("Enter move: ");
        fgets(move, sizeof(move), stdin);
        move[strcspn(move, "\n")] = '\0';
        
        if (strcmp(move, "quit") == 0) {
            printf("Game ended by player.\n");
            break;
        }
        
        // Validate input format
        if (strlen(move) != 5 || move[2] != ' ') {
            printf("Invalid format. Use 'e2 e4' format.\n");
            continue;
        }

        convertNotation(move, &x1, &y1, &x2, &y2);

        if (isValidMove(x1, y1, x2, y2)) {
            makeMove(x1, y1, x2, y2);
            switchTurn();
        } else {
            printf("Invalid move! Try again.\n");
        }
    }

    displayBoard();
    printf("Game Over!\n");

    return 0;
}
