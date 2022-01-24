/* Commands
 * Used by client to communicate with server.
 */
typedef enum Command {START, GETBOARD, GETSIZE, MOVE, NEW, SAVE, LOAD, CHECKWIN} Command;

/* Move Status
 * Used server to give status on client commands.
 */
typedef enum Status {SUCCESS, ERROR, INVALIDMOVE, WIN, NOTWIN} Status;
