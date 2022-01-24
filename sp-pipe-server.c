#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "messages.h"

/* Globals */
extern int clientpipe [2];
extern int serverpipe [2];

/* Constructs a board with dimensions specified by boardsize, declared in main.
 * Parameter 1 size of board, 2 determine if to gen random numbers or not.
 * 1 = yes, 0 = no
 * Returns a 2D array board.
 */
int** constructboard(int boardsize, int generatenums)
{
	//Construct board(2d array)
	int* val = calloc(boardsize * boardsize, sizeof(int));
	int** board = malloc(boardsize * sizeof(int*));
	
	//Holds all possible numbers in the board in incrementing order.
	int numbers[boardsize*boardsize];
	//Used to "reduce" numbers array. Used to set the selected number to 0 to prevent duplicates.
	int numberssize = boardsize * boardsize;
	
	srand(time(NULL));	
	//Generate array of numbers of boardsize^2 in incrementing order.
	for(int i = 0; i < numberssize; i++)
	{
		numbers[i] = i;
	}

	for(int i = 0; i < boardsize; i++)
	{
		board[i] = val + i * boardsize; //create boardsize rows of boardsize elements
	}

	//Fills board with numbers 0-(boardsize*boardsize-1) in random order.
	//generatednum is used to store the randomly generated num and is checked
	//with existingnums to ensure no repeats. 1 = already exists
	//Does not run if generatenums = 1
	for(int row = 0; row < boardsize; row++)
	{
		for(int col = 0; col < boardsize; col++)
		{
			int generatednum = rand() % numberssize;
			board[row][col] = numbers[generatednum];
			
			//Set the generated index to the last element in numbers.
			numbers[generatednum] = numbers[numberssize - 1];
			//Set the last element in numbers to 0 and set reduce numberssize to "remove" the chosen number.
			numbers[numberssize - 1] = 0;
			numberssize--;
		}
	}
	return board;
}

/* Move selected tile to empty tile position and move empty tile to selected
 * tile position. Swap tiles.
 * Parameter 1 takes board, parameter 2 and 3 takes selected tile positions,
 * parameter 4 and 5 takes empty tile positions.
 */
int** movetile(int** board, int selrow, int selcol, int clearrow, int clearcol)
{
        int selectedtile = board[selrow][selcol];
        int cleartile = board[clearrow][clearcol];

        board[selrow][selcol] = cleartile;
        board[clearrow][clearcol] = selectedtile;

        return board;
}

/* Checks if move is valid or possible.
 * row and col are the specified tile's
 * row and col.
 * Parameter 1 takes board, parameter 2 takes tile number
 * parameter 3 takes size of board.
 * Return new state of board after moved tiles, return null
 * if tile could not be moved.
 */
int** checkvalidmove(int** board, int tile, int boardsize)
{
	//Check if tile exists
	if(0 < tile && tile <= (boardsize * boardsize - 1))
	{
		//Store position of empty tile to move chosen tile there.
		int newrow, newcol;
		//Store position of chosen tile to make empty tile.
		int oldrow, oldcol;
		//Check if tile is able to move
		bool isvalid = false;
		
		for(int row = 0; row < boardsize; row++)
		{
			for(int col = 0; col < boardsize; col++)
			{
				if(board[row][col] == tile)
				{
					oldrow = row;
					oldcol = col;
					row = boardsize; //break row loop
					break; //break col loop
				}
			}
		}
		
		//Check all adjacent positions for clear tile.
		for(int backorfront = -1; backorfront <= 1; backorfront = backorfront + 2)
                {
                        //Used to look left and right or up
                        //and down of the current tile position.
                        if(oldrow + backorfront < boardsize && oldrow + backorfront >= 0 && board[oldrow + backorfront][oldcol] == 0)
                        {
                        	newrow = oldrow + backorfront;
                                newcol = oldcol;
                                isvalid = true;
                                break;
                        }
                        else if(oldcol + backorfront < boardsize && oldcol + backorfront >= 0 && board[oldrow][oldcol + backorfront] == 0)
                        {
                                newrow = oldrow;
                                newcol = oldcol + backorfront;
                                isvalid = true;
                                break;
                        }
                 }

		//Move tile if it is able to move
		//else tell user tile cannot move
		if(isvalid)
		{
			board = movetile(board, oldrow, oldcol, newrow, newcol);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
	return board;
}

/* Checks if the board tiles are sorted from 1 to board*board-1
 * Returns 1 if sorted(win) or returns 0 if user has not won
 * Takes board as a parameter
 */
int checkforwin(int** board, int boardsize)
{
	int currentnum = 0;
	
	for(int row = 0; row < boardsize; row++)
	{
		for(int col = 0; col < boardsize; col++)
		{
			if(board[row][col] == currentnum + 1 || board[row][col] == 0)
			{
				if(board[row][col] != 0)
					currentnum = board[row][col];
			}
			else
			{
				return 0;
			}
		}
	}
	return 1;
}

//Free board from heap.
void destroyboard(int** board)
{
	free(*board);
	free(board);
}

/* Save the current game the user is on. User specifies filename. 
 * Saves size of board and contents of board in 2 separate files.
 * Parameter 1 takes current board.
 * Parameter 2 takes current boardsize.
 * Parameter 3 takes filename, used to name save files.
 * return 0 if save failed, return 1 if success.
 */
int save(int** board, int boardsize, char* filename)
{
	FILE *file = fopen(filename, "w");
	FILE *sizefile = fopen(strcat(filename, "size"), "w");

	//return fail if boardsize fails to save.
	if(fprintf(sizefile, "%i", boardsize) < 0)
	{
		return 0;
	}

	for(int row = 0; row < boardsize; row++)
	{
		//Returns fail if board failed to completely save.
		if((fwrite(board[row], sizeof(int), boardsize, file) <= boardsize) == 0)
			return 0;
	}
	fclose(file);
	fclose(sizefile);
	return 1;
}

/* Loads the specified save file.
 * Parameter 1 takes current board.
 * Parameter 2 takes pointer of boardsize, meant to hold any changes done to board size.
 * Parameter 3 takes filename to load.
 * return newboard if successful, return NULL if failed.
 */
int** load(int** board, int *boardsize, char* filename)
{
	FILE *file = fopen(filename, "r");
	
	if (file != NULL)
	{
		FILE *sizefile = fopen(strcat(filename, "size"), "r");
		
		//Return fail if sizefile does not exist.
		if(sizefile == NULL)
		{
			return NULL;
		}
        	
		fscanf(sizefile, "%i", boardsize);
        	
		int** newboard = constructboard(*boardsize, 0);
		
		for(int row = 0; row < *boardsize; row++)
		{
			//Return fail if board fails to completely load.
			if((fread(newboard[row], sizeof(int), *boardsize, file) <= *boardsize) == false)
				return NULL;
		}
		destroyboard(board); //clears only board from memory.
		
		fclose(file);
		fclose(sizefile);
		
		return newboard;
	}
	else
	{
		return NULL;
	}
}

/* Handler to check if user won the game and ends the loop if user did win.
 * Takes board and boardsize as parameters to be used for
 * checkforwin
 * return 1 if win, return 0 if not win
 */
int win(int** board, int boardsize)
{
	int result = checkforwin(board,boardsize);
	
	if(result == 1)
		return 1;
	else
		return 0;
}

/* Acts as the main for client. */
int sp_pipe_server()
{	
	close(clientpipe[1]);
	Status status;
	int** board = NULL;
	char* filename = malloc(sizeof(char));
	int boardsize;
	int tile;
	int win; //Stores win status
	Command command;

	//Listens for commands sent from client.
	while(read(clientpipe[0], &command, sizeof(command)) != 0)
	{
		switch(command)
		{
			//Returns the board to client. Sends 1 row at a time.
			case GETBOARD:
				for(int row = 0; row < boardsize; row++)
				{
					write(serverpipe[1], board[row], boardsize * sizeof(int));
				}
				break;
			
			//Returns the size of the board to client.
			case GETSIZE:
				write(serverpipe[1], &boardsize, sizeof(boardsize));
				break;
			
			//Returns if the tile was able to move or not. Sends status
			//SUCCESS, INVALIDMOVE, or ERROR.
			case MOVE:
				if(read(clientpipe[0], &tile, sizeof(tile)) != 0)
				{
					int** changedboard = checkvalidmove(board, tile, boardsize);
	
					if(changedboard != NULL)
					{
						board = changedboard;
	
						status = SUCCESS;
                	                        write(serverpipe[1], &status, sizeof(SUCCESS));
					}
					else
					{
						status = INVALIDMOVE;
						write(serverpipe[1], &status, sizeof(INVALIDMOVE));
					}
				}
				else
				{
					status = ERROR;
					write(serverpipe[1], &status, sizeof(ERROR));
				}
				break;
			
			//Creates a new board with given size from client.
			case NEW:
				if(read(clientpipe[0], &boardsize, sizeof(int)) != 0)
				{
					if(board != NULL)
					{
						destroyboard(board);
					}
					
					board = constructboard(boardsize, 1);

					status = SUCCESS;
					write(serverpipe[1], &status, sizeof(SUCCESS));
				}
				else
				{
					status = ERROR;
					write(serverpipe[1], &status, sizeof(ERROR));
				}
				break;

			//Creates save files for current game progress.
			case SAVE:
				if(read(clientpipe[0], filename, sizeof(char)*255) != 0)
				{
					if(save(board, boardsize, filename) == 1)
					{
						status = SUCCESS;
						write(serverpipe[1], &status, sizeof(SUCCESS));
					}
					else
					{
						status = ERROR;
						write(serverpipe[1], &status, sizeof(ERROR));
					}
        	                }
	                        else
                        	{
					status = ERROR;
        	                        write(serverpipe[1], &status, sizeof(ERROR));
	                        }
				break;

			//Loads save file sent from client.
			case LOAD:
        	                if(read(clientpipe[0], filename, sizeof(char)*255) != 0)
	                        {
					int* bs = &boardsize;
                	                int** newboard = load(board, bs, filename);
					if(newboard != NULL)
					{
						board = newboard;
						status = SUCCESS;
                        	        	write(serverpipe[1], &status, sizeof(SUCCESS));
					}
					else
					{
						status = ERROR;
						write(serverpipe[1], &status, sizeof(ERROR));
					}
                	        }
        	                else
	                        {
					status = ERROR;
                        	        write(serverpipe[1], &status, sizeof(ERROR));
                	        }
        	                break;

			//Checks if user won the board.
			case CHECKWIN:
				win = checkforwin(board, boardsize);
				if(win == 1)
				{
					status = WIN;
                                        write(serverpipe[1], &status, sizeof(WIN));
				}
				else
				{
					status = NOTWIN;
                                        write(serverpipe[1], &status, sizeof(NOTWIN));
				}
				break;

			//Sends error to client if unknown command is given.
			default:
				status = ERROR;
                                write(serverpipe[1], &status, sizeof(ERROR));
				break;
		}
	}
	destroyboard(board);
	free(filename);
}
