#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100

/* help functions */

typedef struct HistoryNode{
    char command[BUFFER_SIZE];
    struct HistoryNode* next;
}HistoryNode;


int check_if_background(char* command)  /* if last chat is & run in background */
{
    int len= strlen(command);
    if(command[len-1]=='&')
    {
        command[len-1]='\0';   /* remove & from the command */
        return 1;
    }
    return 0;
}

void get_args(char* command, char* args[])  /* using strtok to sort args into array */
{
    char* tokens=strtok(command, " ");
    int i=0;

    while(tokens!=NULL)
    {
        args[i++]=tokens;
        tokens=strtok(NULL, " ");
    }
    args[i]=NULL;
}

void add_to_history(HistoryNode** history, char* command)
{
    HistoryNode* newNode = (HistoryNode*)malloc(sizeof(HistoryNode));
    strncpy(newNode->command, command, BUFFER_SIZE - 1);
    newNode->command[BUFFER_SIZE-1]='\0';
    newNode->next=*history;
    *history=newNode;
}

void print_history(HistoryNode* history,int total_commands) 
{
    while (history != NULL) {
        printf("%d %s\n", total_commands, history->command);
        total_commands--;
        history = history->next;
    }
}

void free_history(HistoryNode* history)
{
    HistoryNode* current = history;
    HistoryNode* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}

/* end of help functions */

int main(void)
{
    close(2);
    dup(1);
    char command[BUFFER_SIZE];

    int total_commands=0;
    int is_background=0;
    char* args[BUFFER_SIZE];
    HistoryNode* history=NULL;

    while (1)
    {
        fprintf(stdout, "my-shell> ");
        memset(command, 0, BUFFER_SIZE);
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0';

        if(strncmp(command, "exit", 4) == 0)
        {
            break;
        }

    /* my code starts here */

        total_commands++;
        add_to_history(&history,command);
        is_background= check_if_background(command);     
        pid_t pid=fork();

        if(pid<0)  /* fork failed */
        {
            perror("error");
            continue;
        }

        if(pid>0)  /* parent */
        {
            if(!is_background) waitpid(pid, NULL, 0); /* wait for the child */
        }
        else   /* child */
        {
            if(strncmp(command, "history", 7) == 0)
            {   
                print_history(history,total_commands);
                exit(0);
            }
            else{
                get_args(command,args);
                
                execvp(args[0], args);
                perror("error");
                exit(1);
            }
        }
    }
    free_history(history);
    return 0;
}
