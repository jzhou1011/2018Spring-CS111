#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>

/* Use this variable to remember original terminal attributes. */

struct termios saved_attributes;
int debug_flag = 0;
int shell_flag = 0;
pid_t child_pid = -1;
int temp_errno;
int to_child_pipe[2];
int from_child_pipe[2];
ssize_t num_read;


void reset_input_mode (void)
{
  //fprintf(stderr, "Resetting input mode.\r\n" );
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode (void)
{
  struct termios tattr;

  /* Save the terminal attributes so we can restore them later. */
  tcgetattr(STDIN_FILENO, &saved_attributes);
  atexit(reset_input_mode);

  /* Set the funny terminal modes. */
  tcgetattr(STDIN_FILENO, &tattr);
  tattr.c_iflag = ISTRIP;
  tattr.c_oflag = 0;
  tattr.c_lflag = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

void message(void){
  int wstatus;

  if(waitpid(child_pid, &wstatus, 0)<0){
    temp_errno = errno;
    fprintf(stderr, "Error with waitpid due to %s.\r\n",strerror(temp_errno));
    exit(1);
  }
  fprintf(stderr, "SHELL EXIT STATUS=%d SIGNAL=%d.\r\n",WEXITSTATUS(wstatus),WTERMSIG(wstatus));
}

void sighandler(int signum) {
   int a = signum;
   //just in case it prints warning
   a += a;
   char msg[] = "Signal SIGPIPE received.\r\n";
   if (write(2,msg,sizeof(msg))<0){
    temp_errno = errno;
    fprintf(stderr, "Error with write due to %s.\r\n",strerror(temp_errno));
    exit(1);
   }
   if (kill(child_pid, SIGPIPE) == -1){
      temp_errno = errno;
      fprintf(stderr, "Unable to kill the child process due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }
   message();
   reset_input_mode();
   _exit(1);
}

void fork_process(void){
  if (pipe(to_child_pipe) ==-1){
    temp_errno = errno;
    fprintf(stderr, "pipe() failed due to %s.\r\n",strerror(temp_errno));
    exit(1);
  }
  if (pipe(from_child_pipe) == -1){
    temp_errno = errno;
    fprintf(stderr, "pipe() failed due to %s.\r\n",strerror(temp_errno));
    exit(1);
  }
  child_pid = fork();
//error
  if (child_pid < 0){
    temp_errno = errno;
    fprintf(stderr, "fork() failed due to %s.\r\n",strerror(temp_errno));
    exit(1);
  }
//parent process
  if (child_pid > 0){
    if ((close(to_child_pipe[0])) < 0){
      temp_errno = errno;
      fprintf(stderr, "close() failed due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }

    if ((close(from_child_pipe[1])) < 0){
      temp_errno = errno;
      fprintf(stderr, "close() failed due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }

    int BUF_SIZE = 10000;

    struct pollfd fds[2];
    int ret;

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = from_child_pipe[0];
    fds[1].events = POLLIN;

    /*if (!ret){
      fprintf(stderr, "Timeout.\n");
      exit(-1);
    }
    */
    if (signal(SIGPIPE, sighandler) == SIG_ERR){
      fprintf(stderr, "Unable to register the signal handler due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }
    while(1){
      ret = poll(fds, 2, 0);
      if (ret == -1){
        temp_errno = errno;
        fprintf(stderr, "Error with polling due to %s.\r\n",strerror(temp_errno));
        exit(1);
      }
      //read from stdin
      if (fds[0].revents & POLLIN){
        char buf[BUF_SIZE+1];
        num_read = read(STDIN_FILENO,&buf,BUF_SIZE);
        if (num_read == -1){
          temp_errno = errno;
          fprintf(stderr,"1.Unable to read from standard input due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
        for(int i = 0; i<num_read; i++){
          if (buf[i] == '\004'){
            char *stop = "^D";
            if (write(STDOUT_FILENO,stop,2) == -1){
              temp_errno = errno;
              fprintf(stderr,"2.Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
            if(debug_flag)
              fprintf(stderr, "Printing %s to stdout.\r\n", stop);
            if ((close(to_child_pipe[1])) < 0){
              temp_errno = errno;
              fprintf(stderr, "Closed() failed due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
            exit(0);
          }
          else if (buf[i] == '\003'){
            char *interupt = "^C";
            if (write(STDOUT_FILENO,interupt,2) == -1){
              temp_errno = errno;
              fprintf(stderr,"3.Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
            if(debug_flag)
              fprintf(stderr, "Printing %s to stdout.\r\n", interupt);
            kill(child_pid,SIGINT);
          }
          else if (buf[i]=='\r' || buf[i]=='\n'){
            char *rn = "\r\n";
            char n = '\n';
            if (write(STDOUT_FILENO,rn,2) == -1){
              temp_errno = errno;
              fprintf(stderr,"4.Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
            if(debug_flag)
              fprintf(stderr, "Printing %s to stdout.\r\n", rn);
            if (write(to_child_pipe[1],&n,1) == -1){
              temp_errno = errno;
              fprintf(stderr,"5.Unable to write to child process due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
            if(debug_flag)
              fprintf(stderr, "Printing %c to shell.\r\n", n);
          }
          else{
            char cur = buf[i];
            if (write(STDOUT_FILENO,&cur,1) == -1){
              temp_errno = errno;
              fprintf(stderr,"6.Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
            if(debug_flag)
              fprintf(stderr, "Printing %c to stdout.\r\n", buf[i]);
            if (write(to_child_pipe[1],&cur,1) == -1){
              temp_errno = errno;
              fprintf(stderr,"7.Unable to write to child process due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
            if(debug_flag)
              fprintf(stderr, "Printing %c to shell.\r\n", buf[i]);
          }
        }
      }
      if (fds[0].revents & POLLHUP){
        if ((close(to_child_pipe[1])) < 0){
          temp_errno = errno;
          fprintf(stderr, "Closed() failed due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
        fds[0].fd *= -1;
        exit(0);
      }

      if (fds[0].revents & POLLERR){
        if ((close(to_child_pipe[1])) < 0){
          temp_errno = errno;
          fprintf(stderr, "Closed() failed due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
        fds[0].fd *= -1;
        exit(0);
      }

      //read from child
      if (fds[1].revents & POLLIN){
        char buf[BUF_SIZE+1];
        num_read = read(from_child_pipe[0],&buf,BUF_SIZE);
        if (num_read == -1){
          temp_errno = errno;
          fprintf(stderr,"8.Unable to read from child process due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
        for(int i = 0; i<num_read; i++){
          if (buf[i] == '\004'){
            if (close(from_child_pipe[0]) < 0){
              temp_errno = errno;
              fprintf(stderr, "Closed() failed due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
            fds[1].fd *= -1;
            exit(0);
          }
          else if (buf[i]=='\n'){
            char *rn = "\r\n";
            if (write(STDOUT_FILENO,rn,2) == -1){
              temp_errno = errno;
              fprintf(stderr,"9.Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
          }
          else{
            char cur = buf[i];
            if (write(STDOUT_FILENO,&cur,1) == -1){
              temp_errno = errno;
              fprintf(stderr,"10.Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
              exit(1);
            }
          }
        }
      }
      if (fds[1].revents & POLLERR){
        exit(0);
      }

      if (fds[1].revents & POLLHUP){
        exit(0);
      }
    }
  }
//child process
  else if (child_pid == 0){

    if (close(from_child_pipe[0]) < 0){
      temp_errno = errno;
      fprintf(stderr, "Closed() failed due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }

    if (close(to_child_pipe[1]) < 0){
      temp_errno = errno;
      fprintf(stderr, "Closed() failed due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }

    if (dup2(to_child_pipe[0],STDIN_FILENO) < 0){
      temp_errno = errno;
      fprintf(stderr, "dup2() failed due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }
    
    if (dup2(from_child_pipe[1],STDERR_FILENO)< 0){
      temp_errno = errno;
      fprintf(stderr, "dup2() failed due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }

    if (dup2(from_child_pipe[1],STDOUT_FILENO) < 0){
      temp_errno = errno;
      fprintf(stderr, "dup2() failed due to %s.\r\n",strerror(temp_errno));
      exit(1);
    }

    char *execvp_argv[2];
    char execvp_filename[] = "/bin/bash";
    execvp_argv[0] = execvp_filename;
    execvp_argv[1] = NULL;
    if (execvp(execvp_filename, execvp_argv) == -1){
      fprintf(stderr, "execvp() failed.\r\n");
      exit(1);
    }
  }
  else {
    temp_errno = errno;
    fprintf(stderr, "fork() failed due to %s.\r\n", strerror(temp_errno));
    exit(1);
  }
}

void read_write (int ifd, int ofd)
{
  if(debug_flag){
    fprintf(stderr,"read_write() called.\r\n");
  }
  int temp_errno;

  while(1){
    char buf[2048];
    num_read = read(ifd,&buf,2048);
    if (num_read == -1){
        temp_errno = errno;
        fprintf(stderr,"Unable to read from standard input due to %s.\r\n",strerror(temp_errno));
        exit(1);
    }
    for(int i = 0; i<num_read; i++){
      if (buf[i] == '\004'){
        char stop[2]="^D";
        if (write(ofd,&stop,2) == -1){
          temp_errno = errno;
          fprintf(stderr,"Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
        exit(0);
      }
      else if (buf[i]=='\r' || buf[i]=='\n'){
        char *rn="\r\n";
        if (write(ofd,rn,2) == -1){
          temp_errno = errno;
          fprintf(stderr,"Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
      }
      else{
        if (write(ofd,&buf+i,1) == -1){
          temp_errno = errno;
          fprintf(stderr,"Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
      }
    }
  }
}

void last_read(void){
  char *rn = "\r\n";
  char buf[2048];
  while((num_read=read(from_child_pipe[0],&buf,2048))> 0){
    for(int i = 0; i < num_read; i++){
      if (buf[i]=='\r' || buf[i]=='\n'){
        if (write(STDOUT_FILENO,rn,2) == -1){
          temp_errno = errno;
          fprintf(stderr,"11.Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
      }
      else{
        char cur = buf[i];
        if (write(STDOUT_FILENO,&cur,1) == -1){
          temp_errno = errno;
          fprintf(stderr,"12.Unable to write to standard output due to %s.\r\n",strerror(temp_errno));
          exit(1);
        }
      }
    }
  }
}

int main(int argc, char **argv){
  set_input_mode();

  static struct option long_options[] =
  {
    {"debug",     no_argument, 0, 'a'},
    {"shell",     no_argument, 0, 'b'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "ab",
                       long_options, &option_index)) != -1){
    switch (c)
    {
      case 'a':
        debug_flag = 1;
        break;
      case 'b':
        shell_flag = 1;
        break;
      default:
        fprintf(stderr, "Usage: lab0 [--shell] [--debug].\r\n");
        exit(1);
    }
  }

  if (shell_flag){
    atexit(message);
    atexit(last_read);
    fork_process();
  }

  else{
    read_write(0,1);
  }

  exit(0);
}

