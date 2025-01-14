#include <stdio.h>
#include <string.h>
#include "board.h"
#include "moves.h"

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Clear the input buffer
}

int main() {
    char move[6];
    int x1, y1, x2, y2;

    initializeBoard();

    while (1) {
        displayBoard();
        printf("%s's turn. Enter your move (e.g., e2 e4): ", currentPlayer == 0 ? "White" : "Black");
        fgets(move, sizeof(move), stdin); // i think this one causes the duplication of input like if i input a move for white, the input will use again for black

        // Remove newline character from input
        move[strcspn(move, "\n")] = '\0';

        // Clear the input buffer to avoid bugs
        clearInputBuffer();

        // Convert input to board indices
        convertNotation(move, &x1, &y1, &x2, &y2);

        // Validate and make the move
        if (isValidMove(x1, y1, x2, y2)) {
            makeMove(x1, y1, x2, y2);
            switchTurn(); // Switch turns after a valid move
        } else {
            printf("Invalid move! Try again.\n");
        }
    }

    return 0;
}
