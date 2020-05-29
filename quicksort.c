// ... ο κώδικάς σας για την υλοποίηση του quicksort
// με pthreads και thread pool...
// Final example with condition variables
// Compile with: gcc -O2 -Wall -pthread cycle_buffer.c -o cbuffer

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 1000000
#define MESSAGES 1000
#define CUTOFF 10

// ---- globals ----

// global integer buffer
int global_buffer;

// global avail messages count (0 or 1)
int global_availmsg = 0;	// empty

// mutex protecting common resources
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t msg_in = PTHREAD_COND_INITIALIZER;
pthread_cond_t msg_out = PTHREAD_COND_INITIALIZER;

typedef struct {
	
	int start;
	int end;
	int sort;
	int finish;
	double * a;
}Message;

typedef struct {
	Message array[MESSAGES];
	int cin, cout;
	int count;
}Queue;

void send_msg(Message msg);
Message rec_msg();

void *w_thread(void *args);
void inssort(double *a,int n);
int partition(double *a,int n);


void inssort(double *a,int n) {
int i,j;
double t;
  
  for (i=1;i<n;i++) {
    j = i;
    while ((j>0) && (a[j-1]>a[j])) {
      t = a[j-1];  a[j-1] = a[j];  a[j] = t;
      j--;
    }
  }

}
int partition(double *a,int n) {
int first,last,middle;
double t,p;
int i,j;

  // take first, last and middle positions
  first = 0;
  middle = n/2;
  last = n-1;  
  
  // put median-of-3 in the middle
  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
  if (a[last]<a[middle]) { t = a[last]; a[last] = a[middle]; a[middle] = t; }
  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
    
  // partition (first and last are already in correct half)
  p = a[middle]; // pivot
  for (i=1,j=n-2;;i++,j--) {
    while (a[i]<p) i++;
    while (p<a[j]) j--;
    if (i>=j) break;

    t = a[i]; a[i] = a[j]; a[j] = t;      
  }
  
  // return position of pivot
  return i;
}

int main() {
  	double *a;
  	int i;

	a = (double *)malloc(N*sizeof(double));
	if (a==NULL) {
		printf("error in malloc\n");
		exit(1);
	}

  	// fill array with random numbers
  	srand(time(NULL));
	for (i=0;i<N;i++) {
    	a[i] = (double)rand()/RAND_MAX;
  	}

  
	  Queue.cin = 0;
  	Queue.cout = 0;
  	Queue.count = 0;
  	
  	Message m;
  	m.start = 0;
	  m.end = N;
	  m.sort = 0;
	  m.finish = 0;
	  m.a = a;
	  send_msg(m);
  	
	pthread_t t1, t2, t3, t4;
	pthread_create(&t1,NULL,w_thread,NULL);
	pthread_create(&t2,NULL,w_thread,NULL);
	pthread_create(&t3,NULL,w_thread,NULL);
	pthread_create(&t4,NULL,w_thread,NULL);

	int numbers = 0;
	for(;;){
		if(numbers == N) break;
		m = rec_msg();
		if(m.sort == 1){
			numbers = numbers + m.end - m.start;

		}
		else{
			send_msg(m);
		}
	}

	m.finish = 1;
	send_msg(m);
	send_msg(m);
	send_msg(m);
	send_msg(m);

	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	pthread_join(t3,NULL);
	pthread_join(t4,NULL);

	// destroy mutex - should be unlocked
	pthread_mutex_destroy(&mutex);

	// destroy cvs - no process should be waiting on these
	pthread_cond_destroy(&msg_out);
	pthread_cond_destroy(&msg_in);
	 // check sorting
  	for (i=0;i<(N-1);i++) {
    	if (a[i]>a[i+1]) {
      		printf("Sort failed!\n");
      		break;
    	}	
  	}  
	free(a);
	return 0;
}



void send_msg(Message msg) {

    pthread_mutex_lock(&mutex);
    while (Queue.count == MESSAGES) { 
      pthread_cond_wait(&msg_out,&mutex);  
    }
    
    // send message
    Queue.array[Queue.cin] = msg;
    Queue.cin++;
    if(Queue.cin == MESSAGES)
    	Queue.cin = 0;
    Queue.count++;
    
    // signal the receiver that something was put in buffer
    pthread_cond_signal(&msg_in);
    
    pthread_mutex_unlock(&mutex);

}

Message rec_msg() {

    pthread_mutex_lock(&mutex);
    while (Queue.count == 0) {	
    
      pthread_cond_wait(&msg_in,&mutex);  
    
    }
    
    Message m = Queue.array[Queue.cout];
    Queue.cout++;
    if(Queue.cout == MESSAGES)
    	Queue.cout = 0;
      Queue.count--;
    
    // signal the sender that something was removed from buffer
    pthread_cond_signal(&msg_out);
    
    pthread_mutex_unlock(&mutex);

    return(m);
}

void *w_thread(void *args) {
	int pivot;
	Message m1, m2;
	for(;;){
		Message m = rec_msg();
		if(m.finish == 1){
			pthread_exit(NULL);
		}
		else if(m.sort == 1){
			send_msg(m);

		}
		else if((m.end - m.start) <= CUTOFF){
			inssort(m.a + m.start, m.end - m.start);
			m.sort = 1;
			send_msg(m);

		}
		else if(m.sort == 0){
			pivot = partition(m.a + m.start, m.end - m.start);
			m1.start = m.start;
			m1.end = m.start + pivot;
			m1.a = m.a;
			m1.finish = 0;
			m1.sort = 0;
			m2.start = m.start + pivot;
			m2.end = m.end;
			m2.a = m.a;
			m2.finish = 0;
			m2.sort = 0;
			send_msg(m1);
			send_msg(m2);
		}
	}
  pthread_exit(NULL); 
}
