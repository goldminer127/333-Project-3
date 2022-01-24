#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Acts as client main
int sp_pipe_client();

//Acts as server main
int sp_pipe_server();

//Globals
int clientpipe [2];
int serverpipe [2];

int main()
{
	pipe(clientpipe);
	pipe(serverpipe);

	pid_t client = fork();
	if(client == 0) //Client
	{	
		sp_pipe_client();
	}
	else //Server
	{
		sp_pipe_server();
	}

}

