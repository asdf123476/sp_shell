#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int checkOpt(char *argv){  
    //실행시의 인자값 확인 
    //  -1 = &
    int opt = 0;
    // 0=없음
    if(argv == NULL){
        return opt;
    }
    for(int i=0; argv[i] != NULL; i++){
        if (argv[i] == '&'){
            opt = -1;
            return opt;
        }
    }
    
    return opt;
}

void matchCMD(int i, char **argv){
    //argv 인자값 확인하고 맞는명령으로 매칭하고 실행되도록 한다.
  
    if(!strcmp(argv[i],"ls")){ls_temp();}
}

void ls_temp(){
    DIR *pdir;
    struct dirent *pde;
    int i = 0;

    if ( (pdir = opendir(".")) < 0 ) {
        perror("디렉토리 열기");
        exit(1);
    }
    while ((pde = readdir(pdir)) != NULL) {
        printf("%20s ", pde->d_name);
        if (++i % 3 == 0)
            printf("\n");
    }
    printf("\n");
    closedir(pdir);
}

void run(int i, int t_opt, char **argv){
    pid_t pid;
    int fd; /* file descriptor */
    char *buf[1024];
    int flags = O_RDWR | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; /* == 0644 */
    memset(buf, 0, 1024);
    pid = fork();
    if (pid == 0){  //child
        //-1 = &
        if(t_opt == -1){
            printf("%s 명령이 백그라운드로 실행됩니다.\n", argv[i]);
            matchCMD(i, argv);
            exit(0);
        }
      
    }
    else if (pid > 0){  //parent - 백그라운드 아닐 때만 기다림
        if(t_opt >= 0){ //백그라운드가 아닐 때
            wait(pid);
	    }
        
    }
    else{
        perror("포크 실패");
    }
}


int main()
{
    char buf[256];
    char *argv[50];
    int narg;
    pid_t pid;
    while (1)
    {
        printf("shell> ");
        gets(buf);
        narg = getargs(buf, argv);
        int t_opt = 0;  
        
        for (int i = 0; i < narg; i++) {  

        if (strcmp(buf, "exit") == 0) // exit 구현
        {
            printf("shell을 종료합니다.\n");
            exit(0);
        }
         // 옵션값으로 명령분리
         int t_opt = checkOpt(argv[i + 1]);    //-1 = &
            if(t_opt == 1){
                i += 2;
            }
            else{
                run(i, t_opt, argv);
            }
            if(t_opt > 1){  
                i += 2;
            }
            
         }
        pid = fork();
        if (pid == 0)
            execvp(argv[0], argv);
        else if (pid > 0)
            wait((int *)0);
        else
            perror("fork failed");
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
