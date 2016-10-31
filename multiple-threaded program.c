

C program:

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>


					// The senaphore lock for each requester to make sure  requester thread must wait for the serving thread to finish handling its last request before issuing its next request
int qIndex = 0 ;
				// index for readyQueue insertion

sem_t Barrier[5];

sem_t myFull, myEmpty;					// the semaphores for the ready queue


pthread_mutex_t myMutex,myMutex1;		// myMutex lock for the ready queue, myMutex1 lock for NumLiveThreads

struct threadData{				//struct to pass parameter threadId and number of request per thread to pthread_create()
   int  threadId;
   int  requestQuan;
};


struct queueNode{				// node structre of ready queue
   int  threadId;
   int  jobTime;
};


int requestQuantity;

int sizeBuffer = 5;	

int maxTime = 100;
pthread_t tid[5],tid2;			//Thread id for requester and Service
	

int NumLiveThreads;			//max job time
			//ready queue size
pthread_attr_t attr; 

int threadNumber;			//Set of thread attributes


struct threadData threadDataArray[10];  //array to pass thread info
struct queueNode readyQueue[5];		  //ready queue

void *Request(void threadArg);		/* the requester thread */
void *Service(void *param); 			/* the Service thread */

void initializeData() {
	int iLoop;
	pthread_mutex_init(&myMutex, NULL);		   /* Create the myMutex lock */
  sem_init(&myFull, 0, 0);		   				/* Create the myFull semaphore and initialize to 0 */
	sem_init(&myEmpty, 0, sizeBuffer);		   /* Create the myEmpty semaphore and initialize to BUFFER_SIZE */
	for(iLoop=0;iLoop<5;iLoop++) {
		sem_init(&Barrier[iLoop], 0, 1);		   	 /* Create the myFull semaphore and initialize to 1 */
				
		readyQueue[iLoop].jobTime=0;
		
		readyQueue[iLoop].threadId =99;	
	}	
  pthread_attr_init(&attr);    					 /* Get the default attributes */
 
}



void *Service(void *param) {		/* Consumer Thread */
	int iLoop,jLoop;
	int requestId;
	
	int shortIndex;
	int shortTime;
	
	
  while(threadNumber) {
   		sleep(1);	
   				
   		shortIndex = 0;
		shortTime = 999;
   		requestId =0;
      sem_wait(&myFull);		    					/* aquire the myFull lock */
      pthread_mutex_lock(&myMutex);	      /* aquire the myMutex lock */
      
      for (iLoop=0;iLoop<5;iLoop++)
      {
      	if(readyQueue[iLoop].threadId == 99)
      		;
      	else {												 //search for the shortest job time
      		if(readyQueue[iLoop].jobTime < shortTime) {
      			shortTime = readyQueue[iLoop].jobTime;
      			shortIndex = iLoop;
      			requestId = readyQueue[iLoop].threadId;
      		}	
      	}
      }           
           
      printf("Servicer thread 0 scheduled job with runTime %d from requester %d\n",shortTime,requestId); 
      readyQueue[shortIndex].jobTime = 0; 
	  readyQueue[shortIndex].threadId = 99;		//reset the node id to 99 and time to 0, which indicate the node has been removed from ready queue
             
      qIndex = shortIndex;
      
      for(jLoop=0;jLoop<5;jLoop++) {					// print out the ready queue content
      	if(readyQueue[jLoop].threadId == 99)
      		printf("[myEmpty]");
      	else
					printf("[%d]",readyQueue[jLoop].jobTime);
			}	
			printf("\n");            

      pthread_mutex_unlock(&myMutex);       /* release the myMutex lock */
      sem_post(&myEmpty);		      /* signal myEmpty */
      sem_post(&Barrier[requestId]);		      				/* signal myFull */
   }
  pthread_exit(0); 
}


void *Request(void threadArg) {		/* Requester Thread */
	struct threadData *myData;
	
	int iLoop,jLoop;
  int taskId;
 
  
  int runTime;
   int num;
  myData = (struct threadData *) threadArg;  
  taskId = myData->threadId;		//get threadId from parameter
  num = myData->requestQuan;		//get number of request per thread from parameter
    
  for(iLoop=0;iLoop<num;iLoop++)
  {
   		sleep(1);	
   		  		
   		sem_wait(&Barrier[taskId]);       /* acquire the Barrier lock for thread taskId, each request thread has a Barrier lock */ 	  		
      runTime = rand()%(maxTime) + 1;   /* generate a random number between 1 to 100 */
      sem_wait(&myEmpty);		      				/* acquire the myEmpty lock */      
      pthread_mutex_lock(&myMutex);       /* acquire the myMutex lock */
      
      /*insert jobs into ready queue*/
      while(1)
      {
      	if( readyQueue[qIndex].threadId == 99) {		//if myEmpty, then insert   	
      		readyQueue[qIndex].jobTime = runTime;
			readyQueue[qIndex].threadId = taskId;
      		break;      		
      	}
      	else {
      		qIndex = (qIndex++)%sizeBuffer;  //otherwise index+1
      	}
      }
      qIndex = (qIndex++)%sizeBuffer; //after insersion, index +1
      printf("Requester thread %d inserted job with runTime %d\n",taskId,runTime);

      for(jLoop=0;jLoop<5;jLoop++) {		// print out the ready queue content
      	if(readyQueue[jLoop].threadId == 99)
      		printf("[myEmpty]");
      	else
					printf("[%d]",readyQueue[jLoop].jobTime);
			}	
			printf("\n");                  
      pthread_mutex_unlock(&myMutex);		  /* release the myMutex lock */
      sem_post(&myFull);		      				/* signal myFull */
  }
  pthread_mutex_lock(&myMutex1);       /* acquire the myMutex1 lock */
  threadNumber--;
  pthread_mutex_unlock(&myMutex1);		  /* release the myMutex1 lock */
  pthread_exit(0);
}
int main(int argc, char *argv[]) {
	int iLoop;
	printf("Main thread beginning\n");
	
   /* 1. Get command line arguments */	
	if(argc != 3) {		/* Verify the correct number of arguments were passed in */
		fprintf(stderr, "Please specify 2 parameters for main: 1.# of requester threads 2. #of request per requester thread "); //  
  }	
	/*
		Parameters
		1. The number of requester threads
		2. The number of request per requester thread
	*/  
  
  requestQuantity = atoi(argv[2]); 
	


  threadNumber = atoi(argv[1]); 
	NumLiveThreads = threadNumber;
	
  /* 2. Initialize buffer */
	memset(readyQueue,0,sizeof(readyQueue));  
  initializeData();		  
     
  /* 3. Create requester threads. */
  for(iLoop=0;iLoop<threadNumber;iLoop++) {
  	
  	threadDataArray[iLoop].requestQuan = requestQuantity;
	threadDataArray[iLoop].threadId = iLoop;
		pthread_create(&tid[iLoop],&attr,requester,(void *) &threadDataArray[iLoop]);		    
    printf("Creating requester thread with id %d and %d requests\n",iLoop,requestQuantity);
  } 
  /* 4. Create Service thread */
  pthread_create(&tid2,&attr,Service,NULL);		    
  printf("Creating Service thread with id 0\n");
  
  /* 5. Wait on Service thread */
  printf("Main thread waiting\n");
  pthread_join(tid2,NULL);
  
  /* 6. Exit  */
  printf("Main thread exiting\n");
  exit(0);   
   
}
