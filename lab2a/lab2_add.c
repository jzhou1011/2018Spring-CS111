#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>

int num_iter = 1;
int opt_yield = 0;
int sync_flag = 0;
int temp_errno;
char sync_opt;
static int spinlock = 0;
static pthread_mutex_t lock;

void error_handle(char *func_nm, int err_num){
  fprintf(stderr, "Error with %s due to %s.\r\n",func_nm,strerror(err_num));
  exit(1);
}

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield)
		if (sched_yield()){
			temp_errno = errno;
			error_handle("sched_yield()", temp_errno);
		}
	*pointer = sum;
}

void* thread_add(void *pointer){
	if (!sync_flag){
		for (int i = 0; i< num_iter; i++){
			add((long long*)pointer, 1);
		}
		for (int i = 0; i< num_iter; i++){
			add((long long*)pointer, -1);
		}
	}
	else{
		for (int i = 0; i< num_iter; i++){
			switch(sync_opt){
				case 'm':
					pthread_mutex_lock(&lock);
					add((long long*)pointer, 1);
					pthread_mutex_unlock(&lock);
					break;
				case 's':
					while(__sync_lock_test_and_set(&spinlock, 1));
					add((long long*)pointer, 1);
					__sync_lock_release(&spinlock);
					break;
				case 'c': ;
					long long old;
					do{
						old = *(long long*)pointer;
						if (opt_yield){
							if (sched_yield()){
								temp_errno = errno;
								error_handle("sched_yield()", temp_errno);
							}
						}
					}while(__sync_val_compare_and_swap((long long*)pointer, old, old+1)!=old);
					break;
			}
		}
		for (int i = 0; i< num_iter; i++){
			switch(sync_opt){
				case 'm':
					pthread_mutex_lock(&lock);
					add((long long*)pointer, -1);
					pthread_mutex_unlock(&lock);
					break;
				case 's':
					while(__sync_lock_test_and_set(&spinlock, 1));
					add((long long*)pointer, -1);
					__sync_lock_release(&spinlock);
					break;
				case 'c': ;
					long long old;
					do{
						old = *(long long*)pointer;
						if (opt_yield){
							if (sched_yield()){
								temp_errno = errno;
								error_handle("sched_yield()", temp_errno);
							}
						}
					}while(__sync_val_compare_and_swap((long long*)pointer, old, old-1)!=old);
					break;
			}
		}
	}
  return NULL;
}

int main(int argc, char **argv){
	int num_thread = 1;

//parsing options
  static struct option long_options[] =
  {
    {"threads",         required_argument, 0, 't'},
    {"iterations",      required_argument, 0, 'i'},
    {"sync",      		required_argument, 0, 's'},
    {"yield",  			no_argument, 	   0, 'y'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "t:i:s:y",
                       long_options, &option_index)) != -1){
    switch (c)
    {
      case 't':
        num_thread = atoi(optarg);
        break;
      case 'i':
        num_iter = atoi(optarg);
        break;
      case 's':
        if (strlen(optarg) != 1){
          fprintf(stderr, "Wrong arg for sync option. Available arguments: m,s,c.\n");
          exit(1);
        }
        switch(optarg[0]){
          case 'm':
            sync_opt = 'm';
            break;
          case 's':
            sync_opt = 's';
            break;
          case 'c':
            sync_opt = 'c';
            break;
          default:
            fprintf(stderr, "Wrong arg for sync option. Available arguments: m,s,c.\n");
            exit(1);
        }
        sync_flag = 1;
        break;
      case 'y':
        opt_yield = 1;
        break;
      default:
        fprintf(stderr, "Usage: lab2_add [--threads=#] [--num_iter=#] [--yield] [--sync=sync_option].\n");
        exit(1);
    }
  }

  long long counter = 0;
  struct timespec start_t,end_t;
  if ( clock_gettime(CLOCK_MONOTONIC, &start_t) < 0){
  	temp_errno = errno;
  	error_handle("clock_gettime() for end time", temp_errno);
  }
//threads
  int i;
  pthread_t *thread_ids = malloc(num_thread * sizeof(pthread_t));
  for (i=0;i<num_thread;i++){
  	int create_sta = pthread_create(&(thread_ids[i]),NULL,thread_add,&counter);
  	if (create_sta){
  		temp_errno = errno;
  		error_handle("pthread_create()", temp_errno);
  	}
  }

  for (i=0;i<num_thread;i++){
  	int join_sta = pthread_join(thread_ids[i],NULL);
  	if (join_sta){
  		temp_errno = errno;
  		error_handle("pthread_join()", temp_errno);
  	}
  }

  if ( clock_gettime(CLOCK_MONOTONIC, &end_t) < 0){
  	temp_errno = errno;
  	error_handle("clock_gettime() for end time", temp_errno);
  }

  long long total_time = 1000000000*(end_t.tv_sec - start_t.tv_sec) + end_t.tv_nsec - start_t.tv_nsec;

//format string
  char test[20] = "";
  if (opt_yield){
  	strcat(test, "-yield");
  }
  if (sync_flag){
  	strcat(test,"-");
  	strcat(test,&sync_opt);
  }
  else{
  	strcat(test, "-none");
  }
  long long total_op = num_thread*num_iter*2;
  long long av_time = total_time/total_op;
  printf("add%s,%d,%d,%lld,%lld,%lld,%lld\n",test,num_thread,num_iter,total_op,total_time,av_time,counter);
  exit(0);
}