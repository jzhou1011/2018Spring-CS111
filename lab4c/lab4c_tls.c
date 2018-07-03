
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef DUMMY
typedef int mraa_aio_context;
mraa_aio_context mraa_aio_init(int p){(void)(p); return 0;}
int mraa_aio_read(mraa_aio_context c){(void)(c); return 650;}
void mraa_aio_close(mraa_aio_context c){(void)(c);}

typedef int mraa_gpio_context;
mraa_gpio_context mraa_gpio_init(int p) {(void)(p); return 0;}
#define MRAA_GPIO_IN 0
void mraa_gpio_dir(mraa_gpio_context c, int d){(void)(c); (void)(d);}
int mraa_gpio_read(mraa_gpio_context c){(void)(c); return 0;}
void mraa_gpio_close(mraa_gpio_context c){(void)(c);}
#else
#include "mraa.h"
#include "mraa/aio.h"
#endif

const int B = 4275;
const int R0 = 100000;  
int period = 1;
char scale = 'F';
int log_flag = 0;
FILE* log_file;
char *filename;
int temp_errno, ret;
int run_flag = 1;
int generating = 1;
int debug_flag = 0;
int host_flag = 0;
char *hostname;
int id_flag = 0;
int id_no;
int port_no;
int sockfd;
char write_buf[2048];
mraa_aio_context sensor;
//mraa_gpio_context button;
struct timeval cur_clock;
struct tm *cur_time;
SSL *ssl;
SSL_CTX *ctx;
const SSL_METHOD *method;

void error_handle(char *func_nm, int err_num){
  fprintf(stderr, "Error with %s due to %s.\n",func_nm,strerror(err_num));
  exit(1);
}

void handle_shutdown(){
  cur_time = localtime(&(cur_clock.tv_sec));
  char time_str[256];
  strftime(time_str, 256, "%2H:%2M:%2S", cur_time);
  memset(write_buf,0,2048);
  sprintf(write_buf,"%s %s\n",time_str, "SHUTDOWN");
  ret = SSL_write(ssl,write_buf,strlen(write_buf));
  fprintf(log_file, "%s %s\n", time_str, "SHUTDOWN");
  fclose(log_file);
  //mraa_gpio_close(button);
  mraa_aio_close(sensor);
  SSL_free(ssl);
  close(sockfd);
  SSL_CTX_free(ctx);
  exit(0);
}

void handle_command(char *command){
  if (debug_flag){
    fprintf(stdout, "command received.\n");
  }
  if (strncmp(command, "OFF",3) == 0){
    fprintf(log_file,"%s\n", command);
    handle_shutdown();
  }
  else if (strncmp(command, "SCALE=F", 7) == 0)
    scale = 'F';
  else if (strncmp(command, "SCALE=C", 7) == 0)
    scale = 'C';
  else if (strncmp(command, "PERIOD=", 7) == 0){
    int valid_num = 1;
    int num_digit = strlen(command)-7;
    int i;
    for (i = 0; i < num_digit; i++){
      if (!isdigit(command[i+7])){
        valid_num = 0;
        break;
      }
    }
    if (valid_num){
      char *temp_per = command+7;
      period = atoi(temp_per);
    }
  }
  else if (strcmp(command, "STOP") == 0)
    generating = 0;
  else if (strcmp(command, "START") == 0)
    generating = 1;

  fprintf(log_file,"%s\n", command);
}

void do_when_interrupted(){
  if (debug_flag)
    fprintf(stdout, "DEBUG: do when do_when_interrupted received." );
  run_flag = 0;
  handle_shutdown();
}

int main(int argc, char **argv){
  int i = 0;
  //port number
  for (i = 0; i < argc; i++){
    if (argv[i][0] != '-' && argv[i][1] != '-'){
      port_no = atoi(argv[i]);
    }
  }
//parsing options
  static struct option long_options[] =
  {
    {"period",          required_argument, 0, 'p'},
    {"scale",           required_argument, 0, 's'},
    {"log",      		required_argument, 0, 'l'},
    {"debug",           no_argument,       0, 'd'},
    {"id",              required_argument, 0, 'i'},
    {"host",            required_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "p:s:l:i:h:d",
                       long_options, &option_index)) != -1){
    switch (c)
    {
      case 'p': ;
        int len = strlen(optarg);
        for (i = 0; i < len; i++){
          if (!isdigit(optarg[i])){
            fprintf(stderr, "Wrong argument for period option. Available arguments: numbers only.\n");
            exit(1);
          }
        }
        period = atoi(optarg);
        break;
      case 's':
        if (strlen(optarg) != 1){
          fprintf(stderr, "Wrong argument for scale option. Available arguments: F, C.\n");
          exit(1);
        }
        switch(optarg[0]){
          case 'C':
            scale = 'C';
            break;
          case 'F':
            scale = 'F';
            break;
          default:
            fprintf(stderr, "Wrong arg for scale option. Available arguments: F, C.\n");
            exit(1);
        }
        break;
      case 'l':
        log_flag = 1;
        char *log_fn = optarg;
        log_file = fopen(log_fn, "a");
        if (log_file == NULL){
          temp_errno = errno;
          error_handle("fopen() log file",temp_errno);
        }
        if (debug_flag){
          fprintf(log_file, "Log flag set.\n");
          fprintf(stdout, "Log flag set.\n");
        }
        break;
      case 'i':
        id_flag = 1;
        id_no = atoi(optarg);
        break;
      case 'h':
        host_flag = 1;
        hostname = optarg;
        break;
      case 'd':
        debug_flag = 1;
        break;
      default:
        fprintf(stderr, "Usage: lab4c --id=9-digit-no --log=filename --host=name/address [--period=#] [--scale=C or F] .\n");
        exit(1);
    }
  }

  if (!id_flag){
    fprintf(stderr, "Mandatory parameter id missing.\nUsage: lab4c --id=9-digit-no --log=filename --host=name/address [--period=#] [--scale=C or F].\n" );
    exit(1);
  }

  if (!log_flag){
    fprintf(stderr, "Mandatory parameter log missing.\nUsage: lab4c --id=9-digit-no --log=filename --host=name/address [--period=#] [--scale=C or F].\n" );
    exit(1);
  }

  if (!host_flag){
    fprintf(stderr, "Mandatory parameter host missing.\nUsage: lab4c --id=9-digit-no --log=filename --host=name/address [--period=#] [--scale=C or F].\n" );
    exit(1);
  }
  if (debug_flag){
    fprintf(stdout,"Host is %s. ID is %d. Port number is %d.\n",hostname,id_no,port_no);
  }

  //open TCP connection
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    temp_errno = errno;
    error_handle("socket()", temp_errno);
  }
  server = gethostbyname(hostname);
  if (server == NULL){
    fprintf(stderr, "No such host: %s","localhost");
    exit(1);
  }
  memset((unsigned char *) &serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy((unsigned char *)&serv_addr.sin_addr.s_addr,(unsigned char *)server->h_addr, server->h_length);
  serv_addr.sin_port = htons(port_no);
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    temp_errno = errno;
    error_handle("connect()",temp_errno);
  }

  //SSL
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  if (SSL_library_init() < 0){
      temp_errno = errno;
      error_handle("SSL_library_init()",temp_errno);
  }
  const SSL_METHOD* method = SSLv23_method();
  ctx = SSL_CTX_new(method);
  if (ctx == NULL){
      temp_errno = errno;
      error_handle("SSL_CTX_new()",temp_errno);
  }
  ssl = SSL_new(ctx);
  SSL_set_fd(ssl,sockfd);
  if (debug_flag){
    fprintf(stdout, "SSL initiated.\n");
  }
  int err = SSL_connect(ssl);
  if (err<1) {
    err=SSL_get_error(ssl,err);
    printf("SSL error #%d in accept,program terminated\n",err);
//   if(err==5){printf("sockerrno is:%d\n",sock_errno());} 
    close(sockfd);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    exit(2);
  }
  if (debug_flag){
    fprintf(stdout, "SSL connected.\n");
  }

  //send (and log) an ID terminated with a newline: ID=ID-number
  memset(write_buf,0,2048);
  sprintf(write_buf,"ID=%d\n",id_no);
  ret = SSL_write(ssl,write_buf,strlen(write_buf));
  ret = fprintf(log_file,write_buf);

  //button = mraa_gpio_init(60);
  sensor = mraa_aio_init(1);
  //mraa_gpio_dir(button, MRAA_GPIO_IN);

  struct pollfd fd_stdin;
  fd_stdin.fd = sockfd;
  fd_stdin.events = POLLIN;

  char cmd_buf[2048];
  time_t next_time = 0;
  while(run_flag){
    gettimeofday(&cur_clock, NULL);
    if (debug_flag){
      fprintf(stdout, "got time of the day.\n");
    }
    //read temperature
    if (generating && (cur_clock.tv_sec >= next_time)){
      if (debug_flag){
        fprintf(stdout, "getting temperature.\n");
      }
      uint16_t value;
      float temperature;
      value = mraa_aio_read(sensor);
      float R = 1023.0/value - 1.0;
      R = R0*R;
      float temperature_c = 1.0/(log(R/R0)/B+1/298.15)-273.15;
      if (scale == 'F'){
        float temperature_f = temperature_c * 1.8 + 32.0;
        temperature = temperature_f;
      }
      else
        temperature = temperature_c;
      if (debug_flag){
        fprintf(stdout, "got temperature.\n");
      }
      int temp_ten = temperature * 10;
      cur_time = localtime(&(cur_clock.tv_sec));
      char time_str[256];
      strftime(time_str, 256, "%2H:%2M:%2S", cur_time);
      memset(write_buf,0,2048);
      sprintf(write_buf,"%s %d.%1d\n", time_str, temp_ten/10, temp_ten%10);
      ret = SSL_write(ssl, write_buf,strlen(write_buf));
      fprintf(log_file,"%s %d.%1d\n", time_str, temp_ten/10, temp_ten%10);
      //increment next time
      next_time = cur_clock.tv_sec + period;
    }
    //read command
    fd_stdin.revents = 0;
    //block for 500ms to wait for command
    if (debug_flag){
      fprintf(stdout, "starts polling.\n");
    }
    ret = poll(&fd_stdin, 1, 500);
    if (ret == -1){
      temp_errno = errno;
      error_handle("poll()", temp_errno);
    }
    else if (ret > 0){
      int num_read, buf_pos, cmd_pos = 0;
      num_read = SSL_read(ssl, cmd_buf, 2048);
      if (debug_flag){
          fprintf(stdout, "DEBUG:numread is %d.\n",num_read);
      }
      if (num_read == 0){
        //EOF received
        handle_shutdown();
      }
      else if (num_read < 0){
        temp_errno = errno;
        error_handle("read() from socket", temp_errno);
      }
      if (debug_flag){
          fprintf(stdout, "DEBUG:input detected.\n");
          fprintf(stdout,"DEBUG: command read is %s.\n",cmd_buf);
      }
      char command[2048];
      memset(command,0,2048);
      for(buf_pos = 0; buf_pos < num_read; buf_pos++){
        if (cmd_buf[buf_pos] == '\n'){
          if (debug_flag)
            fprintf(stdout, "handling command: %s\n",command);
          handle_command(command);
          if (!run_flag){
            handle_shutdown();
          }
          memset(command,0,2048);
          cmd_pos = 0;
        }
        else{
          command[cmd_pos] = cmd_buf[buf_pos];
          cmd_pos++;
        }
      }
    }
    //button not used as shutdown
    //mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &do_when_interrupted, NULL);
    // if (mraa_gpio_read(button)){
    //   if(debug_flag)
    //     fprintf(stdout, "DEBUG: sensed button being pressed.\n");
    //   run_flag = 0;
    //   handle_shutdown();
    //}
  }
  handle_shutdown();
  exit(0);
}