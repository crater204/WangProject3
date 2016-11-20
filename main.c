#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "main.h"


int main(void)
{
    size_t nMaxCmdLen = 256;
	char *cmd = (char *)malloc(nMaxCmdLen);
    assert(cmd != NULL);

    // print a welcome message
    write(STDOUT_FILENO, blankline, strlen(blankline));
    write(STDOUT_FILENO, welcome, strlen(welcome));
    write(STDOUT_FILENO, blankline, strlen(blankline));
    write(STDOUT_FILENO, blankline, strlen(blankline));

	do
	{
        changePrompt();
        // print "minix: " prompt
		write(STDOUT_FILENO, prompt, strlen(prompt));

        // get the command from user
        memset(cmd, 0, nMaxCmdLen);
        getline(&cmd, &nMaxCmdLen, stdin);
        cmd[strcspn(cmd, "\n")] = '\0';

        // break the command
        char *cmdToken = strtok(cmd, tokendel);
        if (cmdToken != NULL)
        {
            write(STDOUT_FILENO, blankline, strlen(blankline));

            // proces the comman
            if (strcmp(cmdToken, "quit") == 0)
            {
                char *param = strtok(NULL, tokendel);
                if (verifyNoAnyParameter(param))
                {
                    quit();
                    break;
                }
            }
            else if (strcmp(cmdToken, "cd") == 0)
            {
                char *pathName = strtok(NULL, tokendel);
                cd(pathName);
            }
            else if (strcmp(cmdToken, "path") == 0)
            {
                char *option = strtok(NULL, tokendel);
                if (option == NULL)
                {
                    path();
                }
                else
                {
                    char *pathName = strtok(NULL, tokendel);
                    if (strcmp(option, "+") == 0)
                    {
                        pathPlus(pathName);
                    }
                    else if (strcmp(option, "-") == 0)
                    {
                        pathMinus(pathName);
                    }
                    else
                    {
                        write(STDOUT_FILENO, e_invalidparameter, strlen(e_invalidparameter));
                    }
                }
            }
            else
            {
                lineIntepreter(cmd);
            }
        }

        write(STDOUT_FILENO, blankline, strlen(blankline));
        write(STDOUT_FILENO, blankline, strlen(blankline));
    }while (1);

    free(cmd);

    return 0;
}
