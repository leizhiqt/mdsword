#include <stdio.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>
#include <sys/wait.h>  

int pipe_default[2];    
  
int main()  
{  
    pid_t pid;  
    char buffer[32];  
  
    memset(buffer, 0, 32);  
    if(pipe(pipe_default) < 0)  
    {  
        printf("Failed to create pipe!\n");  
        return 0;  
    }  
  
    if(0 == (pid = fork()))  
    {  
        close(pipe_default[1]);  
        sleep(5);  
        if(read(pipe_default[0], buffer, 32) > 0)  
        {  
            printf("Receive data from server, %s!\n", buffer);  
        }  
        close(pipe_default[0]);  
    }  
    else  
    {  
        close(pipe_default[0]);  
        if(-1 != write(pipe_default[1], "hello", strlen("hello")))  
        {  
            printf("Send data to client, hello!\n");  
        }  
        close(pipe_default[1]);  
        waitpid(pid,NULL,0);  
    }  
  
    return 1;  
} 
