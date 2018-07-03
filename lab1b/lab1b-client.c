
#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ulimit.h>
#include <assert.h>
#include "zlib.h"


/* Use this variable to remember original terminal attributes. */

struct termios saved_attributes;
int port_flag = 0;
int log_flag = 0;
int compress_flag = 0;
pid_t child_pid = -1;
int temp_errno;
int log_fd;
int to_child_pipe[2];
int from_child_pipe[2];
ssize_t num_read,num_write,num_to_write;
z_stream client_to_stdout;
z_stream stdin_to_client;

void error_handle(char *func_nm, int err_num){
  fprintf(stderr, "Error with %s due to %s.\r\n",func_nm,strerror(err_num));
  exit(1);
}

void close_stream(void){
  inflateEnd(&client_to_stdout);
  deflateEnd(&stdin_to_client);
}

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

//returns num_after_compress
ssize_t my_decompress(int num_read, unsigned char *read_buf, unsigned char *write_buf){
  memset(write_buf,0,2048);
  client_to_stdout.avail_in = num_read;
  client_to_stdout.next_in = read_buf;
  client_to_stdout.avail_out = 2048;
  client_to_stdout.next_out = write_buf;

  do{
    if (inflate(&client_to_stdout, Z_SYNC_FLUSH) == Z_STREAM_ERROR){
      temp_errno = errno;
      error_handle("inflate()", temp_errno);
    }
  }while(client_to_stdout.avail_in > 0);
  num_to_write = 2048 - client_to_stdout.avail_out;
  return num_to_write;
}

ssize_t my_compress(int num_read, unsigned char *read_buf, unsigned char *write_buf){
  memset(write_buf,0,2048);
  stdin_to_client.avail_in = num_read;
  stdin_to_client.next_in = read_buf;
  stdin_to_client.avail_out = 2048;
  stdin_to_client.next_out = write_buf;

  do{
    if (deflate(&stdin_to_client, Z_SYNC_FLUSH) == Z_STREAM_ERROR){
      temp_errno = errno;
      error_handle("deflate()", temp_errno);
    }
  }while(stdin_to_client.avail_in > 0);

  num_to_write = 2048 - stdin_to_client.avail_out;
  return num_to_write;
}

void write_log(unsigned char* msg, int msg_size, unsigned char* buf, int buf_len){
  int num_wri= write(log_fd, msg, msg_size);
  if (num_wri < 0){
    temp_errno = errno;
    error_handle("write() to log",temp_errno);
  }
  num_wri = write(log_fd, buf, buf_len);
  if (num_wri < 0){
    temp_errno = errno;
    error_handle("write() to log",temp_errno);
  }
  num_wri = write(log_fd, "\n", 1);
  if (num_wri < 0){
    temp_errno = errno;
    error_handle("write() to log",temp_errno);
  }
}

void logging(unsigned char* buf, int buf_len, int sending){
/*
  unsigned char msg[21];
  int msg_size = 0;
  memset(msg,0,21);
  if(sending){
    msg_size += 5;
    strcpy(msg,"SENT ");
  }
  else{
    msg_size += 9;
    strcpy(msg,"RECEIVED ");
  }

  int n = buf_len;
  int digit = 0;
  while (n != 0){
    n /= 10;
    digit++;
  }
  switch (digit){
    case 1:
      msg[msg_size] = '0' + buf_len;
      msg_size += 1;
      break;
    case 2:

  }
*/
    int digit = 0;
    int n = buf_len;
    while (n != 0){
      n /= 10;
      digit++;
    }
    int msg_size = 0;
    int oneth,tenth,hundth,thouth;

  if(sending){
    switch (digit){
      case 1:
        msg_size = 14;
        unsigned char msg1[14] = "SENT k bytes: ";
        msg1[5] = '0' + buf_len;
        write_log(msg1, msg_size, buf, buf_len);
        break;
      case 2:
        msg_size = 15;
        unsigned char msg2[15] = "SENT kk bytes: ";
        tenth = buf_len/10;
        oneth = buf_len - 10*tenth;
        msg2[5] = '0' + tenth;
        msg2[6] = '0' + oneth;
        write_log(msg2, msg_size, buf, buf_len);
        break;
      case 3:
        msg_size = 16;
        unsigned char msg3[16] = "SENT kkk bytes: ";
        hundth = buf_len/100;
        tenth = (buf_len - 100*hundth)/10;
        oneth = buf_len - 100*hundth - 10*tenth;
        msg3[5] = '0' + hundth;
        msg3[6] = '0' + tenth;
        msg3[7] = '0' + oneth;
        write_log(msg3, msg_size, buf, buf_len);
        break;
      case 4:
        msg_size = 17;
        unsigned char msg4[17] = "SENT kkkk bytes: ";
        thouth = buf_len/1000;
        hundth = (buf_len - 1000 * thouth)/100;
        tenth = (buf_len- 1000 * thouth - 100 * hundth)/10;
        oneth = buf_len - 1000 * thouth - 100 * hundth - 10 * tenth;
        msg4[5] = '0' + thouth;
        msg4[6] = '0' + hundth;
        msg4[7] = '0' + tenth;
        msg4[8] = '0' + oneth;
        write_log(msg4, msg_size, buf, buf_len);
        break;
      default:
        fprintf(stderr, "Error with switching.");
        exit(1);
    }
    
  }
  else{
    switch (digit){
      case 0:
        break;
      case 1:
        msg_size = 18;
        unsigned char msg1[18] = "RECEIVED k bytes: ";
        msg1[9] = '0' + buf_len;
        write_log(msg1, msg_size, buf, buf_len);
        break;
      case 2:
        msg_size = 19;
        unsigned char msg2[19] = "RECEIVED kk bytes: ";
        tenth = buf_len/10;
        oneth = buf_len - 10*tenth;
        msg2[9] = '0' + tenth;
        msg2[10] = '0' + oneth;
        write_log(msg2, msg_size, buf, buf_len);
        break;
      case 3:
        msg_size = 20;
        unsigned char msg3[20] = "RECEIVED kkk bytes: ";
        hundth = buf_len/100;
        tenth = (buf_len - 100*hundth)/10;
        oneth = buf_len - 100*hundth - 10*tenth;
        msg3[9] = '0' + hundth;
        msg3[10] = '0' + tenth;
        msg3[11] = '0' + oneth;
        write_log(msg3, msg_size, buf, buf_len);
        break;
      case 4:
        msg_size = 21;
        unsigned char msg4[21] = "RECEIVED kkkk bytes: ";
        thouth = buf_len/1000;
        hundth = (buf_len - 1000 * thouth)/100;
        tenth = (buf_len- 1000 * thouth - 100 * hundth)/10;
        oneth = buf_len - 1000 * thouth - 100 * hundth - 10 * tenth;
        msg4[9] = '0' + thouth;
        msg4[10] = '0' + hundth;
        msg4[11] = '0' + tenth;
        msg4[12] = '0' + oneth;
        write_log(msg4, msg_size, buf, buf_len);
        break;
      default:
        fprintf(stderr, "Error with switching.");
        exit(1);
    }
  }
}


int main(int argc, char **argv){

//initialize z_streams
  client_to_stdout.zalloc = Z_NULL;
  client_to_stdout.zfree = Z_NULL;
  client_to_stdout.opaque = Z_NULL;

  if (inflateInit(&client_to_stdout) != Z_OK){
    temp_errno = errno;
    error_handle("inflateInit()", temp_errno);
  }

  stdin_to_client.zalloc = Z_NULL;
  stdin_to_client.zfree = Z_NULL;
  stdin_to_client.opaque = Z_NULL;

  if (deflateInit(&stdin_to_client, Z_DEFAULT_COMPRESSION) != Z_OK){
    temp_errno = errno;
    error_handle("deflate()", temp_errno);
  }

  atexit(close_stream);
//parsing options
  static struct option long_options[] =
  {
    {"log",       required_argument, 0, 'l'},
    {"port",      required_argument, 0, 'p'},
    {"compress",  no_argument, 0,       'c'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  char *log_fn;
  int port_num;
  while((c = getopt_long(argc, argv, "l:p:c",
                       long_options, &option_index)) != -1){
    switch (c)
    {
      case 'l':
        log_fn = optarg;
        log_flag = 1;
        //create log file
        log_fd = creat(log_fn,0666);;
        if (log_fd < 0){
          temp_errno = errno;
          error_handle("creat()",temp_errno);
        }
        break;
      case 'p':
        port_flag = 1;
        port_num = atoi(optarg);
        break;
      case 'c':
        compress_flag = 1;
        break;
      default:
        fprintf(stderr, "Usage: lab1b-client --port=port_no [--log=filename] [--compress].\r\n");
        exit(1);
    }
  }

//check mandatory port number
  if (!port_flag){
    fprintf(stderr, "Usage: lab1b-client --port=port_no [--compress] [--log=filename].\r\n");
    fprintf(stderr, "Port number is a mandatory option.\r\n");
    exit(1);
  }

  set_input_mode();

//open the socket
  struct sockaddr_in serv_addr;
  struct hostent *server;
  unsigned char read_buf[2048];
  unsigned char write_buf[2048];
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    temp_errno = errno;
    error_handle("socket()", temp_errno);
  }
  server = gethostbyname("localhost");
  if (server == NULL){
    fprintf(stderr, "No such host: %s","localhost");
    exit(1);
  }
  memset((unsigned char *) &serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy((unsigned char *)server->h_addr, (unsigned char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(port_num);
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    temp_errno = errno;
    error_handle("connect()",temp_errno);
  }

//polling
  struct pollfd fds[2];
  int ret;

  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  fds[1].fd = sockfd;
  fds[1].events = POLLIN|POLLRDHUP;

  while(1){
    ret = poll(fds, 2, 0);
    if (ret == -1){
      temp_errno = errno;
      error_handle("poll()", temp_errno);
    }
    //read from stdin
    if (fds[0].revents & POLLIN){
      memset(read_buf,0,2048);
      num_read = read(STDIN_FILENO,read_buf,2048);
      if (num_read < 0){
        temp_errno = errno;
        error_handle("read()", temp_errno);
      }
      memset(write_buf,0,2048);
      //write to stdout
      int i;
      for (i=0; i < num_read; i++){
        if (read_buf[i]=='\r' || read_buf[i]=='\n'){
          char *rn = "\r\n";
          if (write(STDOUT_FILENO,rn,2) == -1){
            temp_errno = errno;
            error_handle("write() to stdout", temp_errno);
          }
        }
        else{
          unsigned char cur = read_buf[i];
          if (write(STDOUT_FILENO,&cur,1) == -1){
            temp_errno = errno;
            error_handle("write() to stdout", temp_errno);
          }
        }
      }
      //write to server
      if (compress_flag)
        num_to_write = my_compress(num_read, read_buf,write_buf);
      else{
        memcpy(write_buf,read_buf,num_read);
        num_to_write = num_read;
      }

      num_write = write(sockfd,write_buf,num_to_write);
      if (num_write < 0){
        temp_errno = errno;
        error_handle("write() to server", temp_errno);
      }
      if (log_flag)
        logging(write_buf, num_write,1);
    }
    //read from server
    if(fds[1].revents & POLLIN){
      memset(read_buf, 0, 2048);
      num_read = read(sockfd, read_buf, 2048);
      if (num_read < 0){
        temp_errno = errno;
        error_handle("read()", temp_errno);
      }
      if (log_flag)
        logging(read_buf, num_read,0);

      memset(write_buf,0,2048);
      if (compress_flag)
        num_to_write = my_decompress(num_read,read_buf,write_buf);
      else{
        memcpy(write_buf,read_buf,num_read);
        num_to_write = num_read;
      }
      //write to stdout
      num_write = write(STDOUT_FILENO, write_buf, num_to_write);
      if (num_write < 0){
        temp_errno = errno;
        error_handle("write() to stdout", temp_errno);
      }
    }
    if(fds[1].revents & POLLHUP){
      exit(0);
    }

    if(fds[1].revents & POLLERR){
      exit(0);
    }

    if(fds[1].revents & POLLRDHUP){
      exit(0);
    }
  }

  exit(0);
}


