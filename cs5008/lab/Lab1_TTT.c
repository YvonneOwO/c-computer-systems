#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// ADDED - helper: board full => tie candidate
int board_full(char game_board[3][3])
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			if (game_board[i][j] == ' ')
				return 0;
	return 1;
}

// ADDED - helper: check winner/tie/continue
// Return 'X' or 'O' if someone wins, 'T' if tie, ' ' if game continues
char check_result(char b[3][3])
{
	// rows
	for (int i = 0; i < 3; i++)
	{
		if (b[i][0] != ' ' && b[i][0] == b[i][1] && b[i][1] == b[i][2])
			return b[i][0];
	}
	// cols
	for (int j = 0; j < 3; j++)
	{
		if (b[0][j] != ' ' && b[0][j] == b[1][j] && b[1][j] == b[2][j])
			return b[0][j];
	}
	// diagonals
	if (b[0][0] != ' ' && b[0][0] == b[1][1] && b[1][1] == b[2][2])
		return b[0][0];
	if (b[0][2] != ' ' && b[0][2] == b[1][1] && b[1][1] == b[2][0])
		return b[0][2];

	if (board_full(b)) return 'T';
	return ' ';
}

void print_board(char game_board[3][3])
{
    // TODO - done
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            printf("%c", game_board[i][j]);
            if (j < 2) printf("|");
        }
        printf("\n");
        if (i < 2) printf("-+-+-\n");
    }
}

int main()
{
    printf("Welcome to Tic Tac Toe!\n");

    // Initialize game board with blank
    char game_board[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            game_board[i][j] = ' ';
        }
    }

    char player = 'X';  // ADDED

    // TODO - ADDED
    while (1) {

        print_board(game_board);

        printf("It is %c's turn to move\n", player);
        printf("Your move: ");

        char user_input[8];

        long row, col;

        while (1) {
            if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
                printf("\nNo input. Exiting.\n");
                return 0;
            }

            // Check if the input is valid
            char *str_end;
            row = strtol(user_input, &str_end, 10);    // strtol can only process integers
            if (str_end == user_input)
            {
                printf("You did not input a number!\n");
                printf("Your move: ");
                continue;
                // exit(1);    // Ugly!
            }

            while (*str_end && isspace((unsigned char)*str_end)) str_end++;

            char *col_start = str_end;  // MODIFIED
            col = strtol(col_start, &str_end, 10);
            if (str_end == col_start)
            {
                printf("You did not input a number!\n");
                printf("Your move: ");
                continue;
                // exit(1);    // Ugly!
            }

            // ADDED
            if(row < 0 || row > 2 || col < 0 || col > 2) {
                printf("Out of range. Row/Column must be between 0 and 2. Try again.:)\n");
                printf("Your move: ");
                continue;
            }

            if (game_board[row][col] != ' ') {
                printf("This square is already occupied. Try again. :)\n");
                printf("Your move: ");
                continue;
            }

            // If valid input
            break;
        }
        
        game_board[row][col] = player;

        // ADDED - Check for winner or tie
		char result = check_result(game_board);
		if (result == 'X' || result == 'O')
		{
			print_board(game_board);                 // ADDED
			printf("%c wins!\n", result);            // ADDED
			break;                                   // ADDED
		}
		if (result == 'T')
		{
			print_board(game_board);                 // ADDED
			printf("It's a tie!\n");                 // ADDED
			break;                                   // ADDED
		}

		// ADDED - Change from X's move to O's move
		player = (player == 'X') ? 'O' : 'X';
    }

    return 0;
}

