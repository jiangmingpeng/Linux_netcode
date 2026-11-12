#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

// //系统调用实现的思路
// int daemon (int nochdir, int noclose)
// {
//   int fd;

//   switch (__fork()) {
//     case -1:
//       return (-1);
//     case 0:
//       break;
//     default:
//       _exit(0);
//     }

//   if (__setsid() == -1)
//     return (-1);

//   if (!nochdir)
//     (void)__chdir("/");

//   if (!noclose && (fd = __open(_PATH_DEVNULL, O_RDWR, 0)) != -1) {
//     (void)__dup2(fd, STDIN_FILENO);
//     (void)__dup2(fd, STDOUT_FILENO);
//     (void)__dup2(fd, STDERR_FILENO);
//     if (fd > 2)
//       (void)__close (fd);
//   }
//   return (0);
// }

//1.是否重定向 2.是否更改目录
void Daemon(int isdup,int isdir)
{
    // 忽略中断信号 和 管道破裂信号 因为不能被用户直接中断 也不能因为网络断开链接就直接杀死进程
    signal(SIGINT,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);

    // 新建会话不能是组长
    if (fork() > 0)
        exit(0); // 父进程


    // 创建新的会话
    pid_t pid = setsid();
    if(pid<0)
    {
        std::cerr<<"setsid fail erro"<<std::endl;
        exit(pid);
    }

    // 更改守护进程的工作路径
    if(isdir)
        chdir("/");

    // 进行重定向 1不重定向直接关 0重定向到null黑洞里
    if(isdup)
    {
        int fd = open("/dev/null",O_RDWR);//把所有的垃圾都往null里面丢
        dup2(fd,0);
        dup2(fd,1);
        dup2(fd,2);
        close(fd);
    }
    else
    {
        close(0);
        close(1);
        close(2);
    }
}

