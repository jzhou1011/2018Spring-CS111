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
#include <signal.h>

#include "SortedList.h"

int num_iter = 1;
int opt_yield = 0;
int insert_y = 0;
int delete_y = 0;
int lookup_y = 0;
int sync_flag = 0;
int temp_errno;
char sync_opt;
SortedList_t list;
SortedListElement_t *list_arr;
static int spinlock = 0;
static pthread_mutex_t lock;


void sighandler(int signum) {
   int a = signum;
   //just in case it prints warning
   a += a;
   char msg[] = "Caught Segmentation Fault.\n";
   write(2,msg,sizeof(msg));
   _exit(2);
}

void error_handle(char *func_nm, int err_num){
  fprintf(stderr, "Error with %s due to %s.\r\n",func_nm,strerror(err_num));
  exit(1);
}

void handle_corrupted_list(char *func_nm){
  fprintf(stderr, "Corrupted list discovered when doing %s.\n",func_nm);
    exit(2);
}

char *randstring(int length) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";        
    char *randomString = NULL;
    if (length) {
        randomString = malloc(sizeof(char) * (length +1));
        if (randomString) {            
            for (int n = 0;n < length;n++) {            
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }
            randomString[length] = '\0';
        }
    }
    return randomString;
}

void *thread_func(void* pointer){
  int thread_no = *((int *) pointer);
  int start = thread_no*num_iter;
  int end = start+num_iter;
  //fprintf(stderr,"the start and end for thread %d are %d and %d.\n",thread_no, start, end );
  int i;
  //insert
  for(i=start;i<end;i++){
    if (!sync_flag)
      SortedList_insert(&list,&list_arr[i]);
    else{
      switch(sync_opt){
        case 'm':
          pthread_mutex_lock(&lock);
          SortedList_insert(&list,&list_arr[i]);
          pthread_mutex_unlock(&lock);
          break;
        case 's':
          while(__sync_lock_test_and_set(&spinlock, 1));
          SortedList_insert(&list,&list_arr[i]);
          __sync_lock_release(&spinlock);
          break;
        default:
          fprintf(stderr, "Error with switch due to unrecognized sync option.\r\n");
          exit(1);
      }
    }
  }
  //get the length list
  int length;
  if (!sync_flag)
    length = SortedList_length(&list);
  else{
    switch(sync_opt){
      case 'm':
        pthread_mutex_lock(&lock);
        length = SortedList_length(&list);
        pthread_mutex_unlock(&lock);
        break;
      case 's':
        while(__sync_lock_test_and_set(&spinlock, 1));
        length = SortedList_length(&list);
        __sync_lock_release(&spinlock);
    }
  }
  if(length < 0){
    handle_corrupted_list("check length");
  }
  //looks up and deletes each of the keys
  SortedListElement_t *element_ptr;
  for(i=start;i<end;i++){
    if (!sync_flag){
      element_ptr = SortedList_lookup(&list, list_arr[i].key);
      if (element_ptr==NULL)
        handle_corrupted_list("lookup");
      if (SortedList_delete(element_ptr))
        handle_corrupted_list("delete");
    }
    else{
      switch(sync_opt){
        case 'm':
          pthread_mutex_lock(&lock);
          element_ptr = SortedList_lookup(&list, list_arr[i].key);
          if (element_ptr==NULL)
            handle_corrupted_list("lookup");
          if (SortedList_delete(element_ptr))
            handle_corrupted_list("delete");
          pthread_mutex_unlock(&lock);
          break;
        case 's':
          while(__sync_lock_test_and_set(&spinlock, 1));
          element_ptr = SortedList_lookup(&list, list_arr[i].key);
          if (element_ptr==NULL)
            handle_corrupted_list("lookup");
          if (SortedList_delete(element_ptr))
            handle_corrupted_list("delete");
          __sync_lock_release(&spinlock);
          break;
        default:
          fprintf(stderr, "Error with switch due to unrecognized sync option.\r\n");
          exit(1);
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
    {"sync",      		  required_argument, 0, 's'},
    {"yield",  			    required_argument, 0, 'y'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "t:i:s:y:",
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
        sync_flag = 1;
        if (strlen(optarg) != 1){
          fprintf(stderr, "Wrong arg for sync option. Available arguments: m,s.\n");
          exit(1);
        }
        switch(optarg[0]){
          case 'm':
            sync_opt = 'm';
            break;
          case 's':
            sync_opt = 's';
            break;
          default:
            fprintf(stderr, "Wrong arg for sync option. Available arguments: m,s.\n");
            exit(1);
        }
        break;
      case 'y':
        opt_yield = 1;
        int len_opt = strlen(optarg);
        int i;
        for (i=0;i<len_opt;i++){
          switch(optarg[i]){
            case 'i':
              insert_y = 1;
              break;
            case 'd':
              delete_y = 1;
              break;
            case 'l':
              lookup_y = 1;
              break;
            default:
              fprintf(stderr, "Wrong arg for yield option. Available arguments: i,d,l.\n");
              exit(1);
          }
        }
        break;
      default:
        fprintf(stderr, "Usage: lab2_list [--threads=#] [--num_iter=#] [--yield=[idl]] [--sync=sync_option].\n");
        exit(1);
    }
  }

  //format string part 1
  char test[20] = "-";
  if (!opt_yield){
    strcat(test, "none");
  }
  if(insert_y){
    opt_yield |= INSERT_YIELD;
    strcat(test,"i");
  }
  if(delete_y){
    opt_yield |= DELETE_YIELD;
    strcat(test,"d");
  }
  if(lookup_y){
    opt_yield |= LOOKUP_YIELD;
    strcat(test,"l");
  }

  if (sync_flag){
    strcat(test,"-");
    strcat(test,&sync_opt);
  }
  else{
    strcat(test, "-none");
  }

  if (signal(SIGSEGV, sighandler) == SIG_ERR){
    fprintf(stderr, "Unable to register the signal handler.\n");
    exit(-1);
  }

  list.prev = &list;
  list.next = &list;
  list.key = NULL;
  int num_ele = num_thread*num_iter;
  int i;
  list_arr = malloc(num_ele*sizeof(SortedListElement_t));//rmb to free
  for(i=0; i<num_ele;i++){
    list_arr[i].key = randstring(4);
  }

  struct timespec start_t,end_t;
  if ( clock_gettime(CLOCK_MONOTONIC, &start_t) < 0){
    temp_errno = errno;
    error_handle("clock_gettime() for end time", temp_errno);
  }
//threads
  pthread_t *thread_ids = malloc(num_thread * sizeof(pthread_t));
  int *thread_nos = malloc(num_thread * sizeof(int));
  for (i=0;i<num_thread;i++){
    thread_nos[i]= i;
    //fprintf(stderr, "Thread %d created.\n",thread_nos[i]);
    int create_sta = pthread_create(&(thread_ids[i]),NULL,thread_func,(void *)&(thread_nos[i]));
    if (create_sta){
      temp_errno = errno;
      free(thread_ids);
      free(list_arr);
      free(thread_nos);
      error_handle("pthread_create()", temp_errno);
    }
  }

  for (i=0;i<num_thread;i++){
    int join_sta = pthread_join(thread_ids[i],NULL);
    if (join_sta){
      temp_errno = errno;
      free(thread_ids);
      free(list_arr);
      free(thread_nos);
      error_handle("pthread_join()", temp_errno);
    }
  }

  if ( clock_gettime(CLOCK_MONOTONIC, &end_t) < 0){
    temp_errno = errno;
    error_handle("clock_gettime() for end time", temp_errno);
  }

  free(thread_ids);
  free(list_arr);
  free(thread_nos);

  int list_len = SortedList_length(&list);
  if (list_len != 0){
    handle_corrupted_list("final length checking");
  }

//format string part 2
  long long total_time = 1000000000*(end_t.tv_sec - start_t.tv_sec) + end_t.tv_nsec - start_t.tv_nsec;
  long long total_op = num_thread*num_iter*3;
  long long av_time = total_time/total_op;
  printf("list%s,%d,%d,%d,%lld,%lld,%lld\n",test,num_thread,num_iter,1,total_op,total_time,av_time);
  exit(0);

}