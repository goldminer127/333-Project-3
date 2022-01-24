#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "messages.h"

/* functions */
int** retrieveboard(int);
int checkwin();
void printboard();
void new(int);
int move();
void savecommand();
void loadcommand();
int sp_pipe_client();

/* Globals */
extern int clientpipe [2];
extern int serverpipe [2];

/* Retrieves the board from server.
 * Takes boardsize as a parameter to read
 * the board sent from server.
 */
int** retrieveboard(int boardsize)
{
        Command command = GETBOARD;
        write(clientpipe[1], &command, sizeof(GETBOARD));
	
	//Create a blank board to read the incoming board into.
	int* val = calloc(boardsize * boardsize, sizeof(int));
        int** board = malloc(boardsize * sizeof(int*));
 
        for(int i = 0; i < boardsize; i++)
        {
                board[i] = val + i * boardsize; //create boardsize rows of boardsize elements
        }
	
        for(int row = 0; row < boardsize; row++)
        {
                read(serverpipe[0], board[row], boardsize * sizeof(int));
        }

        return board;
}

/* Checks if the user won the board. Asks the user if they want
 * to play another game if they have won the board. If yes
 * return 1 to continue to a new game. Else reutrn 0 to
 * end the program.
 * Returns 1 as continue or 0 as end program.
 */
int checkwin()
{
        Command command = CHECKWIN;
        write(clientpipe[1], &command, sizeof(CHECKWIN));

        Status status;
        read(serverpipe[0], &status, sizeof(int));

        if(status == WIN)
        {
                char* input = malloc(sizeof(char));

                printf("Congradulations, you won! Would you like to play again?\n");
                printf("Enter \"yes\" to play again, enter anything else to exit.\n");

                scanf("%s", input);
		free(input);
		//Only returns 0 if user wants to stop playing after they won.
                if(strcmp("yes", input) == 0)
                {
                        new(0);
                        return 1;
                }
                else
		{
                        return 0;
		}
        }
        else
	{
                return 1;
	}
}


/* Allows the user to see the state of the board by
 * printing it to console. Only prints on request.
 */
void printboard()
{
	Command command = GETSIZE;
	write(clientpipe[1], &command, sizeof(GETSIZE));
	
	//Read boardsize incoming from server
	int boardsize;
	read(serverpipe[0], &boardsize, sizeof(int));
	
	//Temporary board used to print out board from server.
	int** board = retrieveboard(boardsize);

        //Make top board boarder.
        for(int top = 0; top < boardsize * 3; top++)
        {
                printf("-");
        }
        printf("\n");

        //Print out tile numbers.
        for(int row = 0; row < boardsize; row++)
        {
                for(int col = 0; col < boardsize; col++)
                {
                        //0 represents empty tile. Print out whitespace if tile number is 0.
                        if(board[row][col] != 0)
                        {
                                printf("%3d", board[row][col]);
                        }
                        else
                        {
                                printf("%3s", " ");
                        }
                }
                printf("\n");
        }

        //Print out bottom border.
        for(int bottom = 0; bottom < boardsize * 3; bottom++)
        {
                printf("-");
        }
        printf("\n");
	
	//Free temporary board from memory.
	free(*board);
	free(board);
}

/* Tells the server to move the specified tile in the board.
 * Runs the checkwin function. If checkwin returns 1, return 0
 * to signal quitting the game. Else return 1 to continue the game.
 */
int move()
{
	Command command = MOVE;
	write(clientpipe[1], &command, sizeof(MOVE));
	
	//Get the tile the user wants to move.
	int input;

	printf("What tile do you want to move?\n");
	scanf("%i", &input);

	write(clientpipe[1], &input, sizeof(input));
	
	//Stores result of move from server, result shows if move was valid.
	Status success;
        read(serverpipe[0], &success, sizeof(int));
	
	if(success == INVALIDMOVE)
        {
                printf("Invalid move. Could not move tile %i\n", input);
        }
        else if(success == ERROR)
        {
                printf("Could not move tile %i due to an error.\n", input);
        }
	else
	{
		printf("Successfully moved tile %i\n", input);

		printboard();

		//Check if user wins the current board.
		int win = checkwin();

		if(win == 0)
		{
			//Signal to end the game.
			return 0;
		}
	}

	return 1;
}

/* Tells the server to generate a new board with the specified dimensions.
 * Takes an int as a parameter. If generatedefault is 1, generate a 4 x 4
 * board as a default board. Else ask user to define board dimensions.
 */
void new(int generatedefault)
{
	Command command = NEW;
	write(clientpipe[1], &command, sizeof(NEW));
	
	int input;

	if(generatedefault == 0)
	{
		printf("Enter a number to specify new board size.\n");
		scanf("%i", &input);
		write(clientpipe[1], &input, sizeof(int));
	}
	else
	{
		//Default board size = 4
		input = 4;
		write(clientpipe[1], &input, sizeof(int));
	}
	
	Status success;
        read(serverpipe[0], &success, sizeof(int));

        if(success == SUCCESS)
                printf("\nSuccessfully created a new %i x %i board.\n\n", input, input);
        else
                printf("\nFailed to create new game.\n\n");
}

/* Tells the server to save the current game with the specified
 * file name.
 */
void savecommand()
{
	Command command = SAVE;
	write(clientpipe[1], &command, sizeof(SAVE));

        char* input = malloc(sizeof(char));

        printf("Enter the name of the save file.\n");
        scanf("%s", input);
	
        write(clientpipe[1], input, sizeof(input));
	
	Status success;
        read(serverpipe[0], &success, sizeof(int));

        if(success == SUCCESS)
                printf("Successfully saved %s\n", input);
        else
                printf("Failed to save game");

	free(input);
}

/* Tells the server to load the specified save file.
 * If save file could not be loaded then user continues
 * with current board.
 */
void loadcommand()
{
	Command command = LOAD;
	write(clientpipe[1], &command, sizeof(LOAD));

        char* input = malloc(sizeof(char));

        printf("Enter the name of the save file to load.\n");
        scanf("%s", input);

        write(clientpipe[1], input, sizeof(input));

	Status success;
        read(serverpipe[0], &success, sizeof(int));

        if(success == SUCCESS)
        {
                printf("Successfully loaded game %s\n", input);
        }
        else
        {
                printf("Failed to load %s\n", input);
        }

	free(input);
}

/* Acts as the main for client. */
int sp_pipe_client()
{
	//Close uneccessary server write pipe.
	close(serverpipe[1]);

	//Create default 4x4 board.
	new(1);
	
	//Take input from user for commands.
	char* input = malloc(sizeof(char));

	//Continue game until either user inputs quit or they win the board
	//and do not want to continue a new game.
	while(1)
	{
		printf("Menu: print, move, new, quit, save, load\n");
		
		//Holds user input to decide what choice to execute
                scanf("%s", input);
                
		printf("\n");

		if(strcmp("print", input) == 0)
		{
                        printboard();
                }
		else if(strcmp("move",input) == 0)
		{
			int status = move();
	
			//Stop the game if move return 0.
			if(status == 0)
			{
				break;
			}
		}
		else if(strcmp("new",input) == 0)
		{
			new(0);
		}
		else if(strcmp("quit",input) == 0)
		{
			printf("Quitting game...\n");
			break; //stops the loop
		}
		else if(strcmp("save",input) == 0)
		{
			savecommand();
		}
		else if(strcmp("load",input) == 0)
		{
			loadcommand();
		}
		else
		{
			printf("Invalid input.\n");
			printf("%s\n\n\n", input);
		}
	}

	free(input);
}
