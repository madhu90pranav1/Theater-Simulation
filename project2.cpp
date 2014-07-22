/* CS5348
Operating Systems Concepts
Project 2
Threads

Name: Madhusudan Pranav Venugopal
UTD ID: 2021163441
netid : mxv130430*/

#include <iostream> // Header files required for the functions used in the program
#include <fstream>
#include <string.h>
#include <cstdlib>
#include <semaphore.h>
#include <pthread.h>

using namespace std; // Using standard naming convention

struct movie // Structure to store the details of the movie
{
  char moviename[100];
  int noofseatsavailable;
  sem_t moviestatus; // Semaphore to indicate that the movie is used by a box office agent
}*m;
int noofmovies;
int movieno[330];
char seatsold[330];
int done=0;
int concessionstanddecide[330];
int concessionstandchoice[330];
char csstand[330][40];

// Semaphores used in the project
sem_t taq; //  For enqueuing in queue for buying ticket
sem_t customer[330]; // for synchronizing customer thread [i]
sem_t cagent[330]; // for synchronizing box office agent thhread with customer thread [i]
sem_t ttqueue; // for enqueuing in tickettaker queue
sem_t csqueue;// for enqueuing in concession stand queue
sem_t tq;// for dequeueing by box office agent
sem_t ttqueue1; // for dequeuing by ticket taker
sem_t csd; // for dequeuing by concession stand
sem_t cttready;//for indicating that customer has entered tickettaker queue 
sem_t cscready;// for indicating that customer has entered concession stand queue

// Queue defination for ticket buying, ticket taker and concesion stand queue
class queueline
{
  int qu[330];
  int front1,rear1;
public:
  queueline() // constructor to intialize the private data of the class
  {
    front1=0;
    rear1=-1;
  }
  void enqueue(int c)// function to insert value into the queue
  {
    rear1++;
    
    qu[rear1]=c;
  }
  int dequeue() // function to remove value from the queue
  {
    int r;
    r= qu[front1];
    
    front1++;
    return r;

  }
}ta,ttq,cs;
void moviescount(char filename[20]) // function to count the number of movies present in the file
{
  std::string movieline;
  std::ifstream file(filename);
  while(std::getline(file,movieline))
    noofmovies++;
}
void databasecreation(char filename[20]) // function to create the database of the movies
{
  char *p,t[100];
  char *a;
  int i=0;
  m= new movie[noofmovies];
  std::string movieline;
  std::ifstream file(filename);
  while(std::getline(file,movieline))
    {
      a= &movieline[0];
      p=strtok(a,"\t");
      strcpy(m[i].moviename,p);
      p=strtok(t,p);
      m[i].noofseatsavailable=atoi(t);
      sem_init(&m[i].moviestatus,0,1);
      i++;
    }
}
void *customer1( void *id) // fnction containing the code for task to be performed by customer
{
  int *cip;
  cip= (int *)id;
  int cid;
  cid = *cip;

  sem_wait(&taq); // critical section to enter queue to buy ticket
  
  ta.enqueue(cid);
  sem_post(&taq);
  sem_wait(&customer[cid]); // wait for siganl from box office agent
  
  movieno[cid]=rand()%(noofmovies); // Generate movie name
  printf("\n Customer %d buying ticket to %s",cid,m[movieno[cid]].moviename);
  
  sem_post(&cagent[cid]);
  sem_wait(&customer[cid]);
  // Check if the seat is available
    if(seatsold[cid]=='N')
  goto re;
  else
    {
      sem_wait(&ttqueue); // Critical section to enter queue to see tickettaker
      ttq.enqueue(cid);
      printf("\n Customer %d standing in line to see ticket taker ",cid);
      
      sem_post(&ttqueue);
      sem_post(&cttready); // Signal that customer has entered the queue
      sem_wait(&customer[cid]);

      concessionstanddecide[cid]= rand()%2; // Decide if the customer wants to enter the concession stand
      if(concessionstanddecide[cid]==0)
	goto t;
      else
        {
	  concessionstandchoice[cid]= rand()%3; // Generate the concession stand choice
	  if(concessionstandchoice[cid]==0)
	    strcpy(csstand[cid],"popcorn");
	  else if(concessionstandchoice[cid]== 1)
	    strcpy(csstand[cid],"soda");
	  else if(concessionstandchoice[cid]== 2)
	    strcpy(csstand[cid],"Popcorn & soda");
	  sem_wait(&csqueue); // Critical section to enter queue for the concesion stand
	  cs.enqueue(cid); 
	  printf("\n Customer %d standing in line to buy %s ",cid,csstand[cid]);
	  sem_post(&csqueue);
	  sem_post(&cscready); // Indicate that customer has entered the queue for concession stand
	  sem_wait(&customer[cid]);
	  printf("\n Customer %d receives %s ",cid,csstand[cid]);
	  

        }

    }
 t:
    printf("\n Customer %d enters theatre to see %s ",cid,m[movieno[cid]].moviename);
    
  goto e;
 re:
     printf("\n Customer %d exits due to no seats avalable for %s",cid,m[movieno[cid]].moviename);
	 

 e:
  ;
}


void *boxagent(void *bi)// Function containing code for task to be performed by box office agent
{
  int cid1,mno;

  int *boi;
  boi= (int*)bi;
  int bid= *boi;
  printf("\n Box office agent %d created ",bid);
  
 b:
  sem_wait(&tq); // critical section to dequeue 
  if(done==1)
    goto l;
  cid1=ta.dequeue();
  sem_post(&tq);
  sem_post(&customer[cid1]); // signal ready to customer [i]
  printf("\n Box Office Agent %d serving customer %d",bid,cid1);
  
  sem_wait(&cagent[cid1]); // wait for movie name from customer [i]
  mno=movieno[cid1];
  sem_wait(&m[mno].moviestatus); // wait for the movie to be free
  if(m[mno].noofseatsavailable>=0)
    { // Check if seats are available
      printf("\n Box Office Agent %d sold ticket for %s to customer %d ",bid,m[mno].moviename,cid1);
      
      m[mno].noofseatsavailable--;
      seatsold[cid1]='y';
    }
  else
    seatsold[cid1]=='N';
  sem_post(&m[mno].moviestatus);
  sleep(1.5);
  sem_post(&customer[cid1]); // indicate to customer that task is over
  if(done!=1)
    goto b; // goto next customer in the queue
 l:
  ;
}

void *ticketagent( void *a) // function containing code for task to be performed by ticket taker thread
{
  int cid2;
  printf("\n Ticket taker created");
  
 tt:
  if(done==1)
    goto l;
  sem_wait(&cttready); // wait for customer to enter the queue
  sem_wait(&ttqueue1);
  cid2=ttq.dequeue(); // dequeue customer from the queue
  sem_post(&ttqueue1);
  sleep(0.25);
  printf("\n Ticket taken from customer %d",cid2);
  
  sem_post(&customer[cid2]);
  if(done!=1)
    goto tt; // goto to next customer in the queue
 l:
  ;
}

void *concessionstand(void *c) // function containing code for task to be performed by concession stand
{
  int cid3;
  printf("\n Concession stand created ");
  
 csa:
  if(done ==1)
    goto l;
  sem_wait(&cscready); // wait for customer to enter the queue
  sem_wait(&csd);
  cid3= cs.dequeue(); // dequeue customer from the queue
  sem_post(&csd);
  printf("\n Order for %s taken from customer %d",csstand[cid3],cid3);
  
  sleep(3);
  sem_post(&customer[cid3]);
  if(done!=1)
    goto csa; // go to the next customer in the queue
 l:
  ;
}
int main(int argc, char *argv[]) // main function
{
  char filename[20];
  strcpy(filename,argv[1]);
  moviescount(filename);// calling function to count the number of movies
  databasecreation(filename); // calling function to create a movie database
  // Initializing the semaphores used in the code
  sem_init(&taq,0,1);
  sem_init(&ttqueue,0,1);
  sem_init(&csqueue,0,1);
  sem_init(&tq,0,1);
  sem_init(&ttqueue1,0,1);
  sem_init(&csd,0,1);
  sem_init(&cttready,0,0);
  sem_init(&cscready,0,0);
  for(int i=0;i<330;i++)
    sem_init(&customer[i],0,0);
  for(int j=0;j<330;j++)
    sem_init(&cagent[j],0,0);
  // Declaring the threads
  pthread_t customert[300]; // 300 customer threads
  pthread_t boxofficet[2]; // 2 box office agent threads
  pthread_t tickettakert; // 1 ticket taker thread
  pthread_t concessionstandt; // 1 concession stand thread
  for(int k=0;k<300;k++)
    {
      int *snum= new int;
      *snum =k; // Creating the 300 customer threads
      pthread_create(&customert[k],NULL,customer1,(void *)snum);
    }
  for(int l=0;l<2;l++)
    {
      int *bnum;
      bnum= new int;
      *bnum=l; // creating the 2 box office agent thread
      pthread_create(&boxofficet[l],NULL,boxagent,(void *)bnum);

    }// creating the ticket taker thread
   pthread_create(&tickettakert,NULL,ticketagent,NULL);
   // creating the concession stand thread
  pthread_create(&concessionstandt,NULL,concessionstand,NULL);

    for(int f=0;f<300;f++)
      { // Rejoining of the customer threads
      pthread_join(customert[f],NULL);
      printf("\n Joined customer %d",f);
      
      }
   
    cout<<endl;
  done=1;
  // End of the program
  return 0;
}
