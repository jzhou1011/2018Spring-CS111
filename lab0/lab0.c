
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

void sighandler(int signum) {
   int a = signum;
   //just in case it prints warning
   a += a;
   char msg[] = "Caught Segmentation Fault.\n";
   write(2,msg,sizeof(msg));
   _exit(4);
}

int main(int argc, char **argv){
	int c;
	int fd0,fd1;
	int BUF_SIZE = 10000;
	ssize_t num_read;
	char *input_fn;
	char *output_fn;
	int segfault_flag = 0;
	int catch_flag = 0;
	int temp_errno;
  static struct option long_options[] =
  {
    {"input",     required_argument, 0, 'a'},
    {"output",    required_argument, 0, 'b'},
    {"segfault",  no_argument,       0, 'c'},
    {"catch",     no_argument,       0, 'd'},
    {0, 0, 0, 0}
  };
  int option_index = 0;
	while((c = getopt_long(argc, argv, "a:b:cd",
                       long_options, &option_index)) != -1){
      switch (c)
        {
        case 'a':
          input_fn = optarg;
          fd0 = open(input_fn,O_RDONLY);
          if(fd0 < 0){
          	temp_errno = errno;
          	fprintf(stderr, "Error with option --input, unable to open input file %s due to %s.\n", input_fn,strerror(temp_errno));
          	exit(2);
          }
          int ret = dup2(fd0,STDIN_FILENO);
          if(ret == -1){
          	temp_errno = errno;
          	fprintf(stderr, "Unable to redirect input file to standard input due to %s.\n",strerror(temp_errno));
          	exit(-1);
          }
	  close(fd0);
          break;

        case 'b':
          output_fn = optarg;
          fd1 = creat(output_fn,S_IRWXG|S_IRWXO|S_IRWXU);
          if(fd1 < 0){
          	temp_errno = errno;
          	fprintf(stderr, "Error with option --output, unable to open output file %s due to %s.\n", output_fn,strerror(temp_errno));
          	exit(3);
          }
          ret = dup2(fd1,STDOUT_FILENO);
          if(ret == -1){
          	temp_errno = errno;
          	fprintf(stderr, "Unable to redirect output file to standard output due to %s.\n.",strerror(temp_errno));
          	exit(-1);
          }
	  close(fd1);
          break;

        case 'c':
          segfault_flag = 1;
          break;

        case 'd':
          catch_flag = 1;
          break;

        default:
          fprintf(stderr, "Usage: lab0 [--input=input_filename] [--output=output_filename] [--segfault] [--catch].\n");
 		  exit(1);
        }
    }

    if(catch_flag){
    	if (signal(SIGSEGV, sighandler) == SIG_ERR){
    		fprintf(stderr, "Unable to register the signal handler.\n");
    		exit(-1);
    	}
    }

    if (segfault_flag){
    	char *pointer = NULL;
    	*pointer = 's';
    }

    char buf[BUF_SIZE+1];
    while((num_read = read(STDIN_FILENO,&buf,BUF_SIZE)) != 0 ){
    	if (num_read == -1){
    		temp_errno = errno;
    		fprintf(stderr,"Unable to read from standard input due to %s.\n",strerror(temp_errno));
    		exit(-1);
    	}
    	if (write(STDOUT_FILENO,&buf,num_read) == -1){
    		temp_errno = errno;
    		fprintf(stderr,"Unable to write to standard output due to %s.\n",strerror(temp_errno));
    		exit(-1);
    	}
	}
    exit(0);
}
