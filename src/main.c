#include <stdio.h>
#include <string.h>  
#include "board.h"
#include "moves.h"   

int main() {
    char move[6];
    int x1, y1, x2, y2;

    initializeBoard();

    while (1) {
        displayBoard();
        printf("Enter your move (e.g., e2 e4): ");
        fgets(move, sizeof(move), stdin);

        // Remove newline character from input
        move[strcspn(move, "\n")] = '\0';

        // Convert input to board indices
        convertNotation(move, &x1, &y1, &x2, &y2);

        // Validate and make the move
        if (isValidMove(x1, y1, x2, y2)) {
            makeMove(x1, y1, x2, y2);
        } else {
            printf("Invalid move! Try again.\n");
        }
    }

    return 0;
}
