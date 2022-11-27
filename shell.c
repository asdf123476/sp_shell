#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> // sigaction
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

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
//실행시의 인자값 확인
int optionCheck(char *argv)
{
    int opt = 0; // 0 : 없음

    if (argv == NULL)
    {
        return opt;
    }

    for (int i = 0; argv[i] != NULL; i++)
    {
        if (argv[i] == '&')
        {
            opt = -1; // -1 : &
            return opt;
        }
        if (argv[i] == '|')
        {
            opt = 1; // 1 : pipe
            return opt;
        }
        if (argv[i] == '<')
        {
            opt = 2; // 2 : <
            return opt;
        }
        if (argv[i] == '>')
        {
            opt = 3; // 3 : >
            return opt;
        }
    }

    return opt;
}

void matching(int i, char **argv)
{
    // argv 인자값 확인하고 맞는명령으로 매칭하고 실행되도록 한다.

    if (!strcmp(argv[i], "ls"))
    {
        make_ls();
    }
    if (!strcmp(argv[i], "pwd"))
    {
        make_pwd();
    }
}
void make_pwd()
{
    char path[1024];
    getcwd(path, 1024); // 현재 작업 경로를 얻음
    printf("%s\n", path);
}

void make_ls()
{
    DIR *dirptr;            //현재 디렉토리를 저장해 줄 DIR 포인터 변수 선언
    struct dirent *direntp; //파일의 정보를 저장해 줄 dirent 포인터 변수 선언
    int i = 0;

    dirptr = opendir("."); //성공하면 DIR을, 실패하면 NULL을 반환

    if (dirptr == NULL)
    {
        fprintf(stderr, "Fail to open %s", "."); // open에 실패하면 에러를 출력
    }
    else
    {
        //다음 dirent를 받아 옴
        direntp = readdir(dirptr);
        while (direntp != NULL)
        {
            printf("%10s", direntp->d_name); // dirent에서 이름 데려오기
            if (++i % 3 == 0)
                printf("\n");
            direntp = readdir(dirptr);
        }
        closedir(dirptr); //완료 후 DIR을 close 해준다.
    }
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
        //-1 = &, 1 = pipe, 2 = <, 3 = >
        if (t_opt == -1)
        {
            printf("%s 명령이 백그라운드로 실행됩니다.\n", argv[i]);
            matching(i, argv);
            exit(0);
        }
        else if (t_opt == 0)
        {
            matching(i, argv);
            exit(0);
        }
        else if (t_opt == 2)
        {
            if ((fd = open(argv[i + 2], flags, mode)) == -1)
            {
                perror("open"); /* errno에 대응하는 메시지 출력됨*/
                exit(1);
            }
            if (dup2(fd, STDIN_FILENO) == -1)
            {
                exit(1);
            }
            if (close(fd) == -1)
            {
                perror("close"); /* errno에 대응하는 메시지 출력됨*/
                exit(1);
            }
            //
            matching(i, argv);
            exit(0);
        }
        else if (t_opt == 3)
        {
            if ((fd = open(argv[i + 2], flags, mode)) == -1)
            {
                perror("open"); /* errno에 대응하는 메시지 출력됨*/
                exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                exit(1);
            }
            if (close(fd) == -1)
            {
                perror("close"); /* errno에 대응하는 메시지 출력됨*/
                exit(1);
            }
            matching(i, argv);
            exit(0);
        }
    }
    else if (pid > 0)
    { // parent - 백그라운드 아닐 때만 기다림
        if (t_opt >= 0)
        { //백그라운드가 아닐 때
            wait(pid);
        }
        if (!strcmp(argv[i], "cd"))
        {
            if (argv[i + 1] == NULL)
            {
                fprintf(stderr, "error\n");
            }
            else
            {
                //
            }
        }
    }
    else
    {
        perror("포크 실패");
    }
}

void run_pipe(int i, char **argv)
{
    char buf[1024];
    int p[2];
    int pid;

    /* open pipe */
    if (pipe(p) == -1)
    {
        perror("pipe call failed");
        exit(1);
    }

    pid = fork();

    if (pid == 0)
    {
        close(p[0]);
        if (dup2(p[1], STDOUT_FILENO) == -1)
        {
            exit(1);
        }
        close(p[1]);
        matching(i, argv);
        exit(0);
    }
    else if (pid > 0)
    {
        wait(pid);
        char *arg[1024];
        close(p[1]);
        sprintf(buf, "%d", p[0]);
        arg[0] = argv[i + 2];
        arg[1] = buf;
        matching(0, arg);
    }
    else
        perror("포크 실패");
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

    printf("쉘을 시작합니다.\n");

    while (1)
    {
        char path[1024];
        getcwd(path, 1024); // 현재 작업 경로를 얻음
        printf("%s", path);

        printf("> ");

        gets(buf);
        narg = getargs(buf, argv);

        int t_opt = 0;

        for (int i = 0; i < narg; i++)
        {

            if (strcmp(buf, "exit") == 0)
            {
                printf("쉘을 종료합니다.\n");
                exit(0);
            }
            // 옵션값으로 명령분리
            int t_opt = optionCheck(argv[i + 1]); //-1 = &, 1 = pipe, 2 = <, 3 = >
            if (t_opt == 1)                       // 1 : pipe
            {
                run_pipe(i, argv);
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
