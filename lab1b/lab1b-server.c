
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <getopt.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include "zlib.h"

/* Use this variable to remember original terminal attributes. */

struct termios saved_attributes;
int compress_flag = 0;
int port_flag = 0;
int debug_flag = 0;
int log_flag = 0;
int port_num;

pid_t child_pid = -1;
int temp_errno;
int to_child_pipe[2];
int from_child_pipe[2];
ssize_t num_read;
ssize_t num_to_write;
ssize_t num_write;
int newsockfd;
z_stream shell_to_client;
z_stream server_to_shell;

void error_handle(char *func_nm, int err_num){
  fprintf(stderr, "Error with %s due to %s.\n",func_nm,strerror(err_num));
  exit(1);
}

void close_stream(void){
  inflateEnd(&server_to_shell);
  deflateEnd(&shell_to_client);
}

ssize_t my_decompress(int num_read, unsigned char *read_buf, unsigned char *write_buf){
  memset(write_buf,0,2048);
  server_to_shell.avail_in = num_read;
  server_to_shell.next_in = read_buf;
  server_to_shell.avail_out = 2048;
  server_to_shell.next_out = write_buf;

  do{
    if (inflate(&server_to_shell, Z_SYNC_FLUSH) == Z_STREAM_ERROR){
      temp_errno = errno;
      error_handle("inflate()", temp_errno);
    }
  }while(server_to_shell.avail_in > 0);
  num_to_write = 2048 - server_to_shell.avail_out;
  return num_to_write;
}

ssize_t my_compress(int num_read, unsigned char *read_buf, unsigned char *write_buf){
  memset(write_buf,0,2048);
  shell_to_client.avail_in = num_read;
  shell_to_client.next_in = read_buf;
  shell_to_client.avail_out = 2048;
  shell_to_client.next_out = write_buf;

  do{
    if (deflate(&shell_to_client, Z_SYNC_FLUSH) == Z_STREAM_ERROR){
      temp_errno = errno;
      error_handle("dnflate()", temp_errno);
    }
  }while(server_to_shell.avail_in > 0);

  num_to_write = 2048 - shell_to_client.avail_out;
  return num_to_write;
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
   unsigned char msg[] = "Signal SIGPIPE received by server.\n";
   if (write(STDERR_FILENO,msg,sizeof(msg))<0){
    temp_errno = errno;
    error_handle("write()",temp_errno);
   }
   if (kill(child_pid, SIGPIPE) == -1){
      temp_errno = errno;
      error_handle("kill()",temp_errno);
    }
   message();
   _exit(1);
}

void last_read(void){
  char *rn = "\r\n";
  unsigned char read_buf[2048];
  unsigned char write_buf[2048];
  while((num_read=read(from_child_pipe[0],&read_buf,2048))> 0){
    if (compress_flag){
    //compression required
      unsigned char temp_buf[2048];
      memset(temp_buf,0,2048);
      ssize_t temp_size;
      for(int i = 0; i<num_read; i++){
        if (read_buf[i]=='\n'){
          temp_buf[temp_size] = '\r';
          temp_buf[temp_size+1] = '\n';
          temp_size += 2;
        }
        else{
          temp_buf[temp_size] = read_buf[i];
          temp_size++;
        }
      }
      num_to_write = my_compress(temp_size,temp_buf,write_buf);
      if (write(newsockfd,write_buf,num_to_write) < 0){
        temp_errno = errno;
        error_handle("write() to client", temp_errno);
      }
    }
    else{
      for(int i = 0; i < num_read; i++){

        if (write_buf[i]=='\r' || write_buf[i]=='\n'){
          if (write(newsockfd,rn,2) == -1){
            temp_errno = errno;
            error_handle("write()", temp_errno);
          }
        }
        else{
          unsigned char cur = write_buf[i];
          if (write(newsockfd,&cur,1) == -1){
            temp_errno = errno;
            error_handle("write()", temp_errno);
          }
        }
      }
    }
  }
}

void close_socket(void){
  if ((close(newsockfd)) < 0){
    temp_errno = errno;
    error_handle("close() socket", temp_errno);
  }
}
//returns num_after_compress


int main(int argc, char **argv){
//parsing options
  static struct option long_options[] =
  {
    {"compress",  no_argument, 0,       'c'},
    {"port",      required_argument, 0, 'p'},
    {"debug",     no_argument, 0,       'd'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "cp:d",
                       long_options, &option_index)) != -1){
    switch (c)
    {
      case 'c':
        compress_flag = 1;
        break;
      case 'p':
        port_flag = 1;
        port_num = atoi(optarg);
        break;
      case 'd':
        debug_flag = 1;
        break;
      default:
        fprintf(stderr, "Usage: lab1b-server --port=port_no [--compress] [--debug].\n");
        exit(1);
    }
  }

//check mandatory port number
  if (!port_flag){
    fprintf(stderr, "Usage: lab1b-server --port=port_no [--compress] [--debug].\n");
    fprintf(stderr, "Port number is a mandatory option.");
    exit(1);
  }

//initialize the z_streams
  shell_to_client.zalloc = Z_NULL;
  shell_to_client.zfree = Z_NULL;
  shell_to_client.opaque = Z_NULL;

  if (deflateInit(&shell_to_client, Z_DEFAULT_COMPRESSION) != Z_OK){
    temp_errno = errno;
    error_handle("deflateInit()", temp_errno);
  }

  server_to_shell.zalloc = Z_NULL;
  server_to_shell.zfree = Z_NULL;
  server_to_shell.opaque = Z_NULL;

  if (inflateInit(&server_to_shell) != Z_OK){
    temp_errno = errno;
    error_handle("inflateInit()", temp_errno);
  }

  atexit(close_stream);

//socket communication initialization
  int sockfd;
  struct sockaddr_in serv_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    temp_errno = errno;
    error_handle("socket()",temp_errno);
  }
  memset((unsigned char *) &serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port_num);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    temp_errno = errno;
    error_handle("bind()",temp_errno);
  }
  listen(sockfd,5);
  newsockfd = accept(sockfd, (struct sockaddr *) NULL, NULL);
  if (newsockfd < 0){
    temp_errno = errno;
    error_handle("accept()",temp_errno);
  }
//client accepted
  atexit(close_socket);
//pipe and fork
  if (pipe(to_child_pipe) ==-1){
    temp_errno = errno;
    error_handle("pipe()",temp_errno);
  }
  if (pipe(from_child_pipe) == -1){
    temp_errno = errno;
    error_handle("pipe()",temp_errno);
  }
  atexit(message);
  atexit(last_read);
  child_pid = fork();
  if (child_pid < 0){
    temp_errno = errno;
    error_handle("fork()",temp_errno);
  }
//parent
  else if (child_pid > 0){
    if ((close(to_child_pipe[0])) < 0){
      temp_errno = errno;
      error_handle("close()", temp_errno);
    }
    if ((close(from_child_pipe[1])) < 0){
      temp_errno = errno;
      error_handle("close()", temp_errno);
    }

    struct pollfd fds[2];
    int ret;

    fds[0].fd = newsockfd;
    fds[0].events = POLLIN;
    fds[1].fd = from_child_pipe[0];
    fds[1].events = POLLIN;

    if (signal(SIGPIPE, sighandler) == SIG_ERR){
      fprintf(stderr, "Unable to register the signal handler due to %s.\n",strerror(temp_errno));
      exit(1);
    }

    unsigned char read_buf[2048];
    unsigned char write_buf[2048];
    while(1){
      ret = poll(fds, 2, 0);
      if (ret == -1){
        temp_errno = errno;
        error_handle("poll()",temp_errno);
      }
      //read from socket
      if (fds[0].revents & POLLIN){
        memset(read_buf,0,2048);
        memset(write_buf,0,2048);
        num_read = read(newsockfd,read_buf,2048);
        if (debug_flag){
          fprintf(stderr, "Debug info: msg received from socket: " );
          write(STDERR_FILENO,read_buf,num_read);
          fprintf(stderr, "\r\n" );
        }
        if (num_read < 0){
          temp_errno = errno;
          if ((close(to_child_pipe[1])) < 0){
            int temp_errno2 = errno;
            error_handle("close()", temp_errno2);
          }
          error_handle("read() from socket", temp_errno);
        }
        if (compress_flag){
          num_to_write = my_decompress(num_read,read_buf,write_buf);
          if (debug_flag){
            fprintf(stderr, "Debug info: %ld bytes received after decompressing: ",num_to_write);
            write(STDERR_FILENO,write_buf,num_to_write);
            fprintf(stderr, "\r\n" );
          }
        }
        else{
          memcpy(write_buf,read_buf,num_read);
          num_to_write = num_read;
        }
        
        for (int i = 0; i < num_to_write; i++){
          if (write_buf[i] == '\004'){
            if(debug_flag)
              fprintf(stderr, "^D received, closing the write end of to_child_pipe");
            if ((close(to_child_pipe[1])) < 0){
              temp_errno = errno;
              error_handle("close()", temp_errno);
            }
            exit(0);
          }
          else if (write_buf[i] == '\003'){
            if(debug_flag)
              fprintf(stderr, "^C received, sending SIGINT to shell.");
            if (kill(child_pid,SIGINT) < 0){
              temp_errno = errno;
              error_handle("kill()", temp_errno);
            }
          }
          else if (write_buf[i]=='\r' || write_buf[i]=='\n'){
            unsigned char n = '\n';
            if (write(to_child_pipe[1],&n,1) == -1){
              temp_errno = errno;
              if ((close(to_child_pipe[1])) < 0){
                int temp_errno2 = errno;
                error_handle("close()", temp_errno2);
              }
              error_handle("write()",temp_errno);
            }
            if(debug_flag)
              fprintf(stderr, "Printing newline to shell.\n");
          }
          else{
            unsigned char cur = write_buf[i];
            if (write(to_child_pipe[1],&cur,1) == -1){
              temp_errno = errno;
              if ((close(to_child_pipe[1])) < 0){
                int temp_errno2 = errno;
                error_handle("close()", temp_errno2);
              }
              error_handle("write()",temp_errno);
            }
            if(debug_flag)
              fprintf(stderr, "Printing %c to shell.\n", write_buf[i]);
          }
        }
      }
      if (fds[0].revents & POLLHUP){
        if ((close(to_child_pipe[1])) < 0){
          temp_errno = errno;
          error_handle("close()",temp_errno);
        }
        fds[0].fd *= -1;
        exit(0);
      }
      if (fds[0].revents & POLLERR){
        if ((close(to_child_pipe[1])) < 0){
          temp_errno = errno;
          fprintf(stderr, "Closed() failed due to %s.\n",strerror(temp_errno));
          exit(1);
        }
        fds[0].fd *= -1;
        exit(0);
      }

      //read from child
      if (fds[1].revents & POLLIN){
        memset(read_buf,0,2048);
        num_read = read(from_child_pipe[0],&read_buf,2048);
        if (num_read == -1){
          temp_errno = errno;
          if ((close(from_child_pipe[0])) < 0){
            int temp_errno2 = errno;
            error_handle("close()", temp_errno2);
          }
          error_handle("read() from shell", temp_errno);
        }
        memset(write_buf,0,2048);
        if (compress_flag){
        //compression required
          unsigned char temp_buf[4096] ={};
          ssize_t temp_size = 0;
          for(int i = 0; i<num_read; i++){
            if (read_buf[i] == '\004'){
              if (close(from_child_pipe[0]) < 0){
                temp_errno = errno;
                error_handle("close()", temp_errno);
              }
              fds[1].fd *= -1;
              //compress
              num_to_write = my_compress(temp_size, temp_buf, write_buf);
              if (write(newsockfd,write_buf,num_to_write) < 0){
                temp_errno = errno;
                error_handle("write() to client", temp_errno);
              }
              exit(0);
            }
            else if (read_buf[i]=='\n'){
              temp_buf[temp_size] = '\r';
              temp_buf[temp_size+1] = '\n';
              temp_size += 2;
            }
            else{
              temp_buf[temp_size] = read_buf[i];
              temp_size++;
            }
          }
          num_to_write = my_compress(temp_size, temp_buf, write_buf);
          if (write(newsockfd,write_buf,num_to_write) < 0){
            temp_errno = errno;
            error_handle("write() to client", temp_errno);
          }
        }
        else{
        //no compression
          for(int i = 0; i<num_read; i++){
            if (read_buf[i] == '\004'){
              if (close(from_child_pipe[0]) < 0){
                temp_errno = errno;
                error_handle("close()", temp_errno);
              }
              fds[1].fd *= -1;
              exit(0);
            }
            else if (read_buf[i]=='\n'){
              unsigned char *rn = "\r\n";
              if (write(newsockfd,rn,2) == -1){
                temp_errno = errno;
                if ((close(from_child_pipe[0])) < 0){
                  int temp_errno2 = errno;
                  error_handle("close()", temp_errno2);
                }
                error_handle("write()",temp_errno);
              }
            }
            else{
              unsigned char cur = read_buf[i];
              if (write(newsockfd,&cur,1) == -1){
                temp_errno = errno;
                if ((close(from_child_pipe[0])) < 0){
                  int temp_errno2 = errno;
                  error_handle("close()", temp_errno2);
                }
                error_handle("write()", temp_errno);
              }
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
  else {
    if (close(from_child_pipe[0]) < 0){
      temp_errno = errno;
      error_handle("close()", temp_errno);
    }

    if (close(to_child_pipe[1]) < 0){
      temp_errno = errno;
      error_handle("close()", temp_errno);
    }

    if (dup2(to_child_pipe[0],STDIN_FILENO) < 0){
      temp_errno = errno;
      error_handle("dup2()", temp_errno);
    }
    
    if (dup2(from_child_pipe[1],STDERR_FILENO)< 0){
      temp_errno = errno;
      error_handle("dup2()", temp_errno);
    }

    if (dup2(from_child_pipe[1],STDOUT_FILENO) < 0){
      temp_errno = errno;
      error_handle("dup2()", temp_errno);
    }

    char *execvp_argv[2];
    char execvp_filename[] = "/bin/bash";
    execvp_argv[0] = execvp_filename;
    execvp_argv[1] = NULL;
    if (execvp(execvp_filename, execvp_argv) == -1){
      temp_errno = errno;
      error_handle("execvp()", temp_errno);
    }
  }

  exit(0);
}

