#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#define READ 0
#define WRITE 1

int main(int argc, char *argv[])
{
    while (1)
    {

        int child, pid, status;
        char str[1024];

        int amper = 0, redirect = 0, is_pipe = 0;

        // read stdin
        printf("[prompt] ");
        fgets(str, sizeof(str), stdin);
        str[strlen(str) - 1] = '\0';

        // exit or logout
        if (strcmp(str, "exit") == 0)
            break;
        if (strcmp(str, "logout") == 0)
            break;

        // 백그라운드 검사
        if (strchr(str, '&') != NULL)
            amper = 1;

        // 입출력 재지정 검사 ( >:1 <:2 <>:3 )
        if (strchr(str, '>') != NULL)
            redirect = 1;
        if (strchr(str, '<') != NULL)
            redirect = 2;
        if (strchr(str, '<') != NULL && strchr(str, '>') != NULL)
            redirect = 3;

        if (strchr(str, '|') != NULL)
            is_pipe = 1;

        pid = fork();
        if (pid == 0) // 자식 프로세스
        { 

            // 표준 입출력 재지정 명령어
            if (redirect != 0)
            {
                if (redirect == 1) // 표준 출력 재지정
                {
                    char *command1, *command2;
                    int fd;

                    command1 = strtok(str, "> ");
                    command2 = strtok(NULL, "> ");
                    fd = open(command2, O_CREAT | O_TRUNC | O_WRONLY, 0600);
                    dup2(fd, 1); // 파일을 표준출력에 복제
                    close(fd);
                    execlp(command1, command1, NULL);
                    fprintf(stderr, "%s:실행 불가\n", command1);
                    break;
                }
                else if (redirect == 2) // 표준 입력 재지정
                {
                    char *command1, *command2;
                    int fd;

                    command1 = strtok(str, "< ");
                    command2 = strtok(NULL, "< ");
                    fd = open(command2, O_RDONLY);
                    dup2(fd, 0); // 파일을 표준입력에 복제
                    close(fd);
                    execlp(command1, command1, NULL);
                    fprintf(stderr, "%s:실행 불가\n", command1);
                    break;
                }
                else if (redirect == 3) // 표준 입력 재지정 + 표준 출력 재지정
                {
                    char *command1, *command2, *command3;
                    int fd[2];
                    command1 = strtok(str, "< ");
                    command2 = strtok(NULL, "< ");
                    command3 = strtok(NULL, "> ");
                    fd[0] = open(command2, O_RDONLY);
                    dup2(fd[0], 0); // 파일을 표준입력에 복제
                    close(fd[0]);

                    fd[1] = open(command3, O_CREAT | O_TRUNC | O_WRONLY, 0600);
                    dup2(fd[1], 1); // 파일을 표준출력에 복제
                    close(fd[1]);
                    execlp(command1, command1, NULL);
                    fprintf(stderr, "%s:실행 불가\n", command1);
                    break;
                }
            }

            // pipe 명령어
            else if (is_pipe == 1)
            {
                char *command1, *command2;
                int fd[2];

                command1 = strtok(str, "| ");
                command2 = strtok(NULL, "| ");

                pipe(fd);
                if (fork() == 0)
                {
                    close(fd[READ]);
                    dup2(fd[WRITE], 1); // 쓰기용 파이프를 표준출력에 복제
                    close(fd[WRITE]);
                    execlp(command1, command1, NULL);
                    perror("pipe");
                    break;
                }
                else
                {
                    close(fd[WRITE]);
                    dup2(fd[READ], 0); // 읽기용 파이프를 표준입력에 복제
                    close(fd[READ]);
                    execlp(command2, command2, NULL);
                    perror("pipe");
                    break;
                }
            }

            else // 일반 명령어
            {
                char *result_array[10];
                char *result;
                int i = 0;

                result = strtok(str, " ");

                // str을 execvp를 사용하기 위해 배열로 바꾸는 작업
                while (result != NULL && strcmp(result, "&") != 0)
                {
                    result_array[i] = result;
                    result = strtok(NULL, " ");
                    i++;
                }
                result_array[i] = '\0';

                execvp(result_array[0], &result_array[0]);
                fprintf(stderr, "%s:실행 불가\n", str);
                break;
            }
        }

        // 부모 프로세스 : 백그라운드 작업이 아닐 경우 wait
        else if (amper == 0)
            child = wait(&status);
    }
}
