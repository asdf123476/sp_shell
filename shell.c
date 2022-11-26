#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h> // sigaction

struct sigaction act_ctrl_z;
struct sigaction act_ctrl_c;
struct sigaction act_ctrl_ㅣ;

void ctrl_c(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nctrl - c 을 입력받았습니다.\n");
    printf("SIGINT 발생\n");
    exit(0);
}
void ctrl_z(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nctrl - z 을 입력받았습니다.\n");
    printf("SIGSTOP 발생. 일시정지!!\n");
    raise(SIGSTOP);
}
void ctrl_ㅣ(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nctrl - | 을 입력받았습니다.\n");
    printf("SIGQUIT 발생\n");
    exit(0);
}

int checkOpt(char *argv)
{
    //실행시의 인자값 확인
    //  -1 = &
    int opt = 0;
    // 0=없음
    if (argv == NULL)
    {
        return opt;
    }
    for (int i = 0; argv[i] != NULL; i++)
    {
        if (argv[i] == '&')
        {
            opt = -1;
            return opt;
        }
    }

    return opt;
}

void matchCMD(int i, char **argv)
{
    // argv 인자값 확인하고 맞는명령으로 매칭하고 실행되도록 한다.

    if (!strcmp(argv[i], "ls"))
    {
        ls_temp();
    }
}

void ls_temp()
{
    DIR *pdir;
    struct dirent *pde;
    int i = 0;

    if ((pdir = opendir(".")) < 0)
    {
        perror("디렉토리 열기");
        exit(1);
    }
    while ((pde = readdir(pdir)) != NULL)
    {
        printf("%20s ", pde->d_name);
        if (++i % 3 == 0)
            printf("\n");
    }
    printf("\n");
    closedir(pdir);
}

void run(int i, int t_opt, char **argv)
{
    pid_t pid;
    int fd; /* file descriptor */
    char *buf[1024];
    int flags = O_RDWR | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; /* == 0644 */
    memset(buf, 0, 1024);
    pid = fork();
    if (pid == 0)
    { // child
        //-1 = &
        if (t_opt == -1)
        {
            printf("%s 명령이 백그라운드로 실행됩니다.\n", argv[i]);
            matchCMD(i, argv);
            exit(0);
        }
    }
    else if (pid > 0)
    { // parent - 백그라운드 아닐 때만 기다림
        if (t_opt >= 0)
        { //백그라운드가 아닐 때
            wait(pid);
        }
    }
    else
    {
        perror("포크 실패");
    }
}

int main()
{
    char buf[256];
    char *argv[50];
    int narg;
    pid_t pid;

    act_ctrl_c.sa_handler = ctrl_c;   // ctrl - c (SIGINT) 프로세스 강제 종료
    act_ctrl_z.sa_handler = ctrl_z;   // ctrl - z (SIGTSTP) 프로세스 일시 정지
    act_ctrl_ㅣ.sa_handler = ctrl_ㅣ; // ctrl - \ (SIGQUIT) 프로세스가 종료되고 코어가 덤프

    sigaction(SIGINT, &act_ctrl_c, NULL);
    sigaction(SIGTSTP, &act_ctrl_z, NULL);
    sigaction(SIGQUIT, &act_ctrl_ㅣ, NULL);

    while (1)
    {
        char path[1024];
        getcwd(path, 1024); // 현재 작업 경로를 얻음
        printf("%s ", path);

        printf("위치의 shell> ");

        gets(buf);
        narg = getargs(buf, argv);
        int t_opt = 0;

        if (strcmp(buf, "exit") == 0) // exit 구현
        {
            printf("shell을 종료합니다.\n");
            exit(0);
        }

        for (int i = 0; i < narg; i++)
        {

            // 옵션값으로 명령분리
            int t_opt = checkOpt(argv[i + 1]); //-1 = &
            if (t_opt == 1)
            {
                i += 2;
            }
            else
            {
                run(i, t_opt, argv);
            }
            if (t_opt > 1)
            {
                i += 2;
            }
        }
    }
}
int getargs(char *cmd, char **argv)
{
    int narg = 0;
    while (*cmd)
    {
        if (*cmd == ' ' || *cmd == '\t')
            *cmd++ = '\0';
        else
        {
            argv[narg++] = cmd++;
            while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
                cmd++;
        }
    }
    argv[narg] = NULL;
    return narg;
}
