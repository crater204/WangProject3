#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/minix_fs.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include "main.h"
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>

/************************************
 *
 * Global variables
 *
 ************************************/

char welcome[] = "Welcome to Minix File System simple terminal";
char blankline[] = "\n";
char tokendel[] = " \t\n";
char e_invalidparameter[] = "--- ---ERROR: Invalid parameter!--- ---";
char e_cannotfork[] = "--- ---ERROR: Fork failed!--- ---";
char e_cannotexecute[] = "--- ---ERROR: Execv failed!--- ---";
char e_cannotwait[] = "--- ---ERROR: Waitpid failed!--- ---";
char e_illegalpathname[] = "--- ---ERROR: Illegal pathname!--- ---";
char e_cannotfind[] = "--- ---ERROR: Directory doesn't exist!--- ---";
char e_cannotopen[] = "--- ---ERROR: Opendir failed!--- ---";
char e_cannotremove[] = "--- ---ERROR: Path is empty! Cannot remove a pathname!--- ---";
char e_cannotpwd[] = "--- ---ERROR: Getcwd failed!--- ---";
char e_cannotchdir[] = "--- ---ERROR: Chdir failed!--- ---";

char *dirpath[20];
char *prompt;

/************************************
 *
 * Helper methods
 *
 ************************************/
int runCommand(char *, char **, bool);

bool verifyNoAnyParameter(char *pParam)
{
    bool hasNoParameter = true;
    if (pParam != NULL && strlen(pParam))
    {
        printf("--- ---This command should not take any parameter!--- ---");
        fflush(stdout);
        hasNoParameter = false;
    }

    return hasNoParameter;
}

void buildcwd(char **argv)
{
    char *tok;
    tok = *argv;
    int ii;
    int status = 0;
    size_t cwdsize = 256;

    for (ii = 0; ii < strlen(argv[0]); ii++)
    {
        if (*(tok + ii) == '/')
        {
            status = 1;
            break;
        }
    }

    char *dirname = (char *)malloc(cwdsize);
    memset(dirname, 0, cwdsize);

    if (status == 1)
    {
        if (*tok == '/')
        {
            runCommand(tok, argv, false);
        }
        else if (*tok == '.')
        {
            if (getcwd(dirname, cwdsize) == NULL)
            {
                write(STDOUT_FILENO, e_cannotpwd, strlen(e_cannotpwd));
            }
            else
            {
                char *new_tok = (char *)malloc(strlen(tok) - 1);
                memset(new_tok, 0, strlen(tok) - 1);
                strcpy(new_tok, &tok[1]);

                char *new_str = (char *)malloc(strlen(dirname) + strlen(new_tok) + 1);
                memset(new_str, 0, strlen(dirname) + strlen(new_tok) + 1);
                strcat(new_str, dirname);
                strcat(new_str, new_tok);

                runCommand(new_str, argv, false);

                free(new_tok);
                free(new_str);
            }
        }
        else if ((*tok == '.') && (*(tok + 1) == '.'))
        {
            if (getcwd(dirname, cwdsize) == NULL)
            {
                write(STDOUT_FILENO, e_cannotpwd, strlen(e_cannotpwd));
            }
            else
            {
                if (strcmp(dirname, "/") == 0)
                {
                    char *new_tok = (char *)malloc(strlen(tok) - 2);
                    memset(new_tok, 0, strlen(tok) - 2);
                    strcpy(new_tok, &tok[2]);
                    runCommand(new_tok, argv, false);
                    free(new_tok);
                }
                else
                {
                    char *last = strrchr(dirname, '/');
                    char *new_dirname = (char *)malloc(strlen(dirname) - strlen(last) + 1);
                    memset(new_dirname, 0, strlen(dirname) - strlen(last) + 1);
                    strncpy(new_dirname, dirname, strlen(dirname) - strlen(last));

                    char *new_tok = (char *)malloc(strlen(tok) - 2);
                    memset(new_tok, 0, strlen(tok) - 2);
                    strcpy(new_tok, &tok[2]);

                    char *new_str = (char *)malloc(strlen(new_dirname) + strlen(new_tok) + 1);
                    memset(new_str, 0, strlen(new_dirname) + strlen(new_tok) + 1);
                    strcat(new_str, new_dirname);
                    strcat(new_str, new_tok);

                    runCommand(new_str, argv, false);
                    free(new_dirname);
                    free(new_tok);
                    free(new_str);
                }
            }
        }
        else
        {
            if (getcwd(dirname, cwdsize) == NULL)
            {
                write(STDOUT_FILENO, e_cannotpwd, strlen(e_cannotpwd));
            }
            else
            {
                char *new_str = (char *)malloc(strlen(dirname) + strlen(tok) + 2);
                memset(new_str, 0, strlen(dirname) + strlen(tok) + 2);
                strcat(new_str, dirname);
                strcat(new_str, "/");
                strcat(new_str, tok);

                runCommand(new_str, argv, false);
                free(new_str);
            }
        }
    }
    else
    {
        if (getcwd(dirname, cwdsize) == NULL)
        {
            write(STDOUT_FILENO, e_cannotpwd, strlen(e_cannotpwd));
        }
        else
        {
            char *new_str = (char *)malloc(strlen(dirname) + strlen(tok) + 2);
            memset(new_str, 0, strlen(dirname) + strlen(tok) + 2);
            strcat(new_str, dirname);
            strcat(new_str, "/");
            strcat(new_str, tok);

            runCommand(new_str, argv, true);
            free(new_str);
        }
    }

    free(dirname);
}

int runCommand(char *pathname, char **argv, bool pathSearch)
{
    pid_t pid;
    int status;
    int jj;

    pid = fork();
    if (pid == -1)
    {
        write(STDOUT_FILENO, e_cannotfork, strlen(e_cannotfork));
        return (-1);
    }
    else if (pid == 0)
    {
        if (pathSearch)
        {
            execv(pathname, argv);
            printf("Cannot find command from current directory! Search from maintained path!\n");
            fflush(stdout);

            for (jj = 0; jj < 20; jj++)
            {
                if (dirpath[jj] == NULL)
                    break;

                char *new_dirpath = (char *)malloc(strlen(dirpath[jj]) + strlen(argv[0]) + 2);
                memset(new_dirpath, 0, strlen(dirpath[jj]) + strlen(argv[0]) + 2);
                strcat(new_dirpath, dirpath[jj]);
                strcat(new_dirpath, "/");
                strcat(new_dirpath, argv[0]);

                execv(new_dirpath, argv);
                printf("Cannot find command at index %d of miantained path!\n", jj);
                fflush(stdout);
                free(new_dirpath);
            }

            printf("Failed to execute command: %i\n", errno);
            fflush(stdout);
            //write(STDOUT_FILENO, e_cannotexecute, strlen(e_cannotexecute));
        }
        else
        {
            execv(pathname, argv);
            printf("Cannot find command from current directory! Will not search from maintained path!\n");
            fflush(stdout);
        }

        exit(1);
    }
    else
    {
        if (waitpid(pid, &status, 0) == -1)
        {
            write(STDOUT_FILENO, e_cannotwait, strlen(e_cannotwait));
            return (-1);
        }
        else
        {
            return (status);
        }
    }
}

/************************************
 *
 * Minix commands
 *
 ************************************/

void changePrompt();
void cleanUp();

void quit(void)
{
    cleanUp();
    printf("Bye, bye!\n\n");
    fflush(stdout);
}

void cleanUp(void)
{
    int ii;

    for (ii = 0; ii < 20; ii++)
    {
        if (dirpath[ii] == NULL)
            break;
        free(dirpath[ii]);
        dirpath[ii] = NULL;
    }

    free(prompt);
    prompt = NULL;
}

void cd(char *pathName)
{
    if (chdir(pathName) == -1)
    {
        write(STDOUT_FILENO, e_cannotchdir, strlen(e_cannotchdir));
    }
    else
    {
        changePrompt();
    }
}

void changePrompt(void)
{
    size_t cwdsize = 256;

    prompt = (char *)malloc(256);
    memset(prompt, 0, 256);

    char *dirname = (char *)malloc(cwdsize);
    memset(dirname, 0, cwdsize);
    if (getcwd(dirname, cwdsize) == NULL)
    {
        write(STDOUT_FILENO, e_cannotpwd, strlen(e_cannotpwd));
    }
    else
    {
        struct passwd *pw = getpwuid(getuid());
        char *homedir = pw->pw_dir;

        char *ret = strstr(dirname, homedir);
        if (ret != NULL && ret == dirname)
        {
            char *new_dirname = (char *)malloc(strlen(dirname) - strlen(homedir) + 3);
            memset(new_dirname, 0, strlen(dirname) - strlen(homedir) + 3);
            strcat(new_dirname, "~");
            strcat(new_dirname, &dirname[strlen(homedir)]);
            strcat(new_dirname, "$");
            strcpy(prompt, new_dirname);
            free(new_dirname);
        }
        else
        {
            strcat(dirname, "$");
            strcpy(prompt, dirname);
        }
    }

    free(dirname);
}

void path(void)
{
    int i;

    char *alt1 = "/bin";
    dirpath[0] = (char *)malloc(strlen(alt1) + 1);
    memset(dirpath[0], 0, strlen(alt1) + 1);
    strcpy(dirpath[0], alt1);

    char *alt2 = "/usr/bin";
    dirpath[1] = (char *)malloc(strlen(alt2) + 1);
    memset(dirpath[1], 0, strlen(alt2) + 1);
    strcpy(dirpath[1], alt2);

    for (i = 0; i < 20; i++)
    {
        if (dirpath[i] != NULL)
        {
            printf("%s", dirpath[i]);
            if (dirpath[i + 1] != NULL)
            {
                printf(":");
            }
        }
        else
        {
            break;
        }
    }

    fflush(stdout);
}

void pathPlus(char *pathName)
{
    int ii, jj;
    for (ii = 0; ii <= strlen(pathName) - 1; ii++)
    {
        if (*(pathName + ii) == '\0')
        {
            write(STDOUT_FILENO, e_illegalpathname, strlen(e_illegalpathname));
            break;
        }
    }

    DIR *dp = opendir(pathName);
    if (dp)
    {
        closedir(dp);
    }
    else if (ENOENT == errno)
    {
        write(STDOUT_FILENO, e_cannotfind, strlen(e_cannotfind));
    }
    else
    {
        write(STDOUT_FILENO, e_cannotopen, strlen(e_cannotopen));
    }

    for (jj = 2; jj < 20; jj++)
    {
        if (dirpath[jj] == NULL)
        {
            dirpath[jj] = (char *)malloc(strlen(pathName) + 1);
            memset(dirpath[jj], 0, strlen(pathName) + 1);
            strcpy(dirpath[jj], pathName);
            break;
        }
    }
}

void pathMinus(char *pathName)
{
    int ii;
    int status = 0;

    if (dirpath == NULL)
    {
        write(STDOUT_FILENO, e_cannotremove, strlen(e_cannotremove));
        return;
    }

    for (ii = 0; ii < 19; ii++)
    {
        if (strcmp(dirpath[ii], pathName) == 0)
        {
            status = 1;
            free(dirpath[ii]);
            dirpath[ii] = NULL;
            int jj;
            for (jj = ii; jj < 19; jj++)
            {
                dirpath[jj] = dirpath[jj + 1];
            }
            dirpath[jj + 1] = NULL;
            break;
        }
    }

    if (status == 0)
    {
        if (strcmp(dirpath[19], pathName) == 0)
        {
            free(dirpath[19]);
            dirpath[19] = NULL;
        }
        else
        {
            write(STDOUT_FILENO, e_cannotfind, strlen(e_cannotfind));
        }
    }
}

void lineIntepreter(char *commandLine)
{
    char *argv[20]; // TODO
    int ii = 0;

    int narg = 1;
    char *arg;

    for (ii = 0; ii < 20; ++ii)
    {
        argv[ii] = NULL;
    }

    argv[0] = (char *)malloc(strlen(commandLine) + 1);
    memset(argv[0], 0, strlen(commandLine) + 1);
    strcpy(argv[0], commandLine);
    while ((arg = strtok(NULL, tokendel)) != NULL)
    {
        argv[narg] = (char *)malloc(strlen(arg) + 1);
        memset(argv[narg], 0, strlen(arg) + 1);
        strcpy(argv[narg], arg);
        narg++;
    }
    argv[narg] = NULL;
    buildcwd(argv);
}
