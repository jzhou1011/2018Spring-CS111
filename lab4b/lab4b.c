
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

#ifdef DUMMY
typedef int mraa_aio_context;
mraa_aio_context mraa_aio_init(int p){(void)(p); return 0;}
int mraa_aio_read(mraa_aio_context c){(void)(c); return 650;}
void mraa_aio_close(mraa_aio_context c){(void)(c);}

typedef int mraa_gpio_context;
mraa_gpio_context mraa_gpio_init(int p) {(void)(p); return 0;}
#define MRAA_GPIO_IN 0
#define 
void mraa_gpio_dir(mraa_gpio_context c, int d){(void)(c); (void)(d);}
int mraa_gpio_read(mraa_gpio_context c){(void)(c); return 0;}
void mraa_gpio_close(mraa_gpio_context c){(void)(c);}
void mraa_gpio_isr(mraa_gpio_context c, int d, void f, void *n){(void)(c); (void)(d); (void)(f); (void)(n);}
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
int temp_errno;
int run_flag = 1;
int generating = 1;
int debug_flag = 0;
mraa_aio_context sensor;
mraa_gpio_context button;
struct timeval cur_clock;
struct tm *cur_time;

void error_handle(char *func_nm, int err_num){
  fprintf(stderr, "Error with %s due to %s.\r\n",func_nm,strerror(err_num));
  exit(1);
}

void handle_shutdown(){
  cur_time = localtime(&(cur_clock.tv_sec));
  char time_str[256];
  strftime(time_str, 256, "%2H:%2M:%2S", cur_time);
  fprintf(stdout, "%s %s\n", time_str, "SHUTDOWN");
  if (log_flag){
    fprintf(log_file, "%s %s\n", time_str, "SHUTDOWN");
    fclose(log_file);
  }
  mraa_gpio_close(button);
  mraa_aio_close(sensor);
  exit(0);
}

void handle_command(char *command){
  if (debug_flag){
    fprintf(stdout, "command received.\n");
  }
  if (strncmp(command, "OFF",3) == 0){
    if (log_flag)
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

  if (log_flag)
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
//parsing options
  static struct option long_options[] =
  {
    {"period",          required_argument, 0, 'p'},
    {"scale",           required_argument, 0, 's'},
    {"log",      		    required_argument, 0, 'l'},
    {"debug",           no_argument,       0, 'd'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "p:s:l:",
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
      case 'd':
        debug_flag = 1;
        break;
      default:
        fprintf(stderr, "Usage: lab4b [--period=#] [--scale=C or F] [--log=filename].\n");
        exit(1);
    }
  }

  button = mraa_gpio_init(60);
  sensor = mraa_aio_init(1);
  // if (!button){
  //   temp_errno = errno;
  //   error_handle("button init", temp_errno);
  // }
  // if (!sensor){
  //   temp_errno = errno;
  //   error_handle("sensor init", temp_errno);
  // }
  mraa_gpio_dir(button, MRAA_GPIO_IN);


  struct pollfd fd_stdin;
  fd_stdin.fd = STDIN_FILENO;
  fd_stdin.events = POLLIN;
  int ret;

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
      fprintf(stdout, "%s %d.%1d\n", time_str, temp_ten/10, temp_ten%10);
      if (log_flag){
        fprintf(log_file,"%s %d.%1d\n", time_str, temp_ten/10, temp_ten%10);
      }
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
      if (debug_flag){
        fprintf(stdout, "input detected.\n");
      }
      int num_read, buf_pos, cmd_pos = 0;
      num_read = read(STDIN_FILENO, cmd_buf, 2048);
      if (num_read == 0){
        //EOF received
        handle_shutdown();
      }
      else if (num_read < 0){
        temp_errno = errno;
        error_handle("read() from stdin", temp_errno);
      }
      char command[2048];
      memset(command,0,2048);
      for(buf_pos = 0; buf_pos < num_read; buf_pos++){
        if (cmd_buf[buf_pos] == '\n'){
          if (debug_flag)
            fprintf(stdout, "sending command: %s\n",command);
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
      //sense the button
    }
    if (debug_flag){
      fprintf(stdout, "DEBUG: Sensing button.\n");
    }
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &do_when_interrupted, NULL);
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