#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <time.h>
#define bool int
#define F(i,a,b) for(int i=a;i<b;i++)
#define false 0
#define true 1
#define RED 0 
#define GREEN 1
#define MAXNODES 100000
#define null 0

//declaration section
typedef struct node {
	int state;			//state of the channel
	bool allgreen;			//variable to check whether all incoming channel markers have been received  
	int channel_status[MAXNODES];	//status of incoming channels
	int channel_record[MAXNODES];	//record buffer of incoming channels
	int channel_trans[MAXNODES];
}node;

node g_node;    //A node to represent each processor 
int data=10000; //initialize the data to $10,000. 
int rank,size;  //determines the rank and size of processor
MPI_Status status;
char MARKER='m'; 
int initiator=0;//default initiator node
int d;
int sum=0;


//function prototypes
void display();
void perform_money_transaction_system();


void display() {
	printf("\t\t");	
	F(i,0,size) {
                if(rank==i) {
			printf("0\t");
                        continue;
		}
                else 
                        printf("%d\t",g_node.channel_trans[i]);
        }
	printf("\n\n");
	MPI_Barrier(MPI_COMM_WORLD);
}

void perform_money_transaction_system() {
	F(i,0,size) {
		 if(rank==i)
			continue;
                 d=rand()%10+1;
		 //g_node.channel_trans[i]=d;
                 MPI_Send(&d,1,MPI_INT,i,0,MPI_COMM_WORLD);
                 data-=d;
		 g_node.channel_trans[i]=d;               
                 MPI_Recv(&d,1,MPI_INT,i,0,MPI_COMM_WORLD,&status);
                 data+=d;                                     
         }
}

//main code begins here
int main(int argc,char **argv) {

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Request send_request,recv_request;
	double initial_time,final_time; 
	double totaltime;
	int ierr;
	
	int array[size+2];
	//initialize
	F(i,0,size) {
		g_node.channel_status[i]=RED;
		g_node.channel_record[i]=null;
	}
	g_node.state=data;
	srand(time(NULL));



	while(true) {
		//perform_money_system_transaction

		perform_money_transaction_system();
		scanf("%c %d",&MARKER,&initiator);
		if(initiator > size-1) {
				printf("Enter a valid node!\n");
				MPI_Abort(MPI_COMM_WORLD,1);
		}	

		//Assuming, ( N X N ) INTERCONNECTION NETWORK

		initial_time=MPI_Wtime();
		//SENDING MARKER
		F(i,0,size) {
			if(rank==i)
				continue;
			else if(rank==initiator) {
				//start recording on incoming channels,make incoming channels wide open
				F(j,1,size) 
					g_node.channel_status[j]=RED;
			}	
			//send markers to other processes		
			//MPI_Send(&MARKER,1,MPI_CHAR,i,0,MPI_COMM_WORLD);
			ierr=MPI_Isend(&MARKER,1,MPI_CHAR,i,0,MPI_COMM_WORLD,&send_request);
			ierr=MPI_Wait(&send_request,&status);
			g_node.state=data;	//record own state
		}

		//RECEIVING MARKER
		F(i,0,size) {
			if(rank==i)
				continue;
			//received marker for the first time			
			//MPI_Recv(&MARKER,1,MPI_CHAR,i,0,MPI_COMM_WORLD,&status);
			ierr=MPI_Irecv(&MARKER,1,MPI_CHAR,i,0,MPI_COMM_WORLD,&recv_request);
			ierr=MPI_Wait(&send_request,&status);
			//if marker is received for the first time, record own state
			if(g_node.channel_status[i]==RED) {
				g_node.state=data;	//record state
				g_node.channel_record[rank]=0;	//intiate NULL
				g_node.channel_status[i]=GREEN; //close the channel
			}
		
			else { //if received earlier
				
				if(rank==initiator) { 
					if(g_node.channel_status[i]==GREEN)
						;
					//check if all the incoming channels are recorded
						F(j,0,size) {
							if(j==initiator)
								continue;
							if(g_node.channel_status[j]==RED) {
								g_node.allgreen=false;
								break;
							}
						}
					//if all are green we are done
					if(g_node.allgreen) 
						break;
					else {
						g_node.allgreen=true;	
						break;
					}
				}
				else {
					//for non-initiator processes check if all are green
					F(j,0,size) {
						if(rank==j)
							continue;
						if(g_node.channel_status[j]==RED) {
							g_node.allgreen=false;
							break;
						}    
					}
					if(!g_node.allgreen)
						continue;
					else {
						g_node.allgreen=true;
						break;
					}
				}
			}
			
		}
	
		//ierr=MPI_Wait(&send_request,&status);
		//ierr=MPI_Wait(&recv_request,&status);
		final_time=MPI_Wtime();
		sum=0;
		if(rank!=0) {
			//calculate sum
			for(int i=0;i<size;i++) {
				sum+=g_node.channel_trans[i];
			}
			
			for(int i=0;i<size;i++) 
				g_node.state-=(g_node.channel_trans[i]);
			sum+=g_node.state;
		        		
			array[0]=g_node.state;
			for(int i=1;i<(size+2-1);i++)
				array[i]=g_node.channel_trans[i-1];
			array[size+2-1]=sum;
			MPI_Send(&array,size+2,MPI_INT,0,0,MPI_COMM_WORLD);
			/*if(rank==1) {
                        	int f;
        	                if(MPI_Recv(&f,1,MPI_INT,0,0,MPI_COMM_WORLD,&status)==1);
                	        printf("\nTotal sum=%d",f);
	                }*/
			
		}

		if(rank==0) {
			int tarray[6];
			int info[size][6];

			for(int i=0;i<size;i++) {
                                sum+=g_node.channel_trans[i];
                        }
			
			for(int i=0;i<size;i++)
                                g_node.state-=(g_node.channel_trans[i]);
                        sum+=g_node.state;

			printf("\n---------------------------------------------------");
			printf("\n   CHANDY    LAMPORTS     GLOBAL    SNAPSHOT");
			printf("\n---------------------------------------------------\n");
			
			printf("   PROCESSOR    STATE  ");
			for(int i=0;i<size;i++)
				printf("\tP%d ",i);
			printf("\n\n");
			printf("\t%d",rank);

			//store own info		
			//info[0][0]=g_node.state;
			printf("\t%d",g_node.state);//info[0][0]);
			for(int i=1;i<(size+2-1);i++) {
				//info[0][i]=g_node.channel_trans[i-1];
				printf("\t%d",g_node.channel_trans[i-1]);//info[0][i]);
			}
			printf("\n");
			info[0][5]=sum;
			//printf("  %d  \n",info[0][5]); 

			int f=0;
			f+=sum;
		        
			int i;	
			for(i=1;i<size;i++) {
				
				MPI_Recv(&array,size+2,MPI_INT,i,0,MPI_COMM_WORLD,&status);
				printf("\t%d",i);
				for(int j=1;j<(size+2);j++)
				{
					//info[i][j-1]=tarray[j-1];
					printf("\t%d",array[j-1]);
				}
				//printf("%d ",array[size+2-1]);  
				f+=array[size+2-1];
				if(i==size-1) {
					printf("\n\n\n\n   INITIATOR PROCESSOR: %d",initiator);
					printf("\n\n   TOTAL SUM: %d",f);
					totaltime = ((double) (final_time - initial_time));;
					printf("\n\n   TIME TAKEN : %f sec\n\n",totaltime);
				}
				else 
					printf("\n");
			}
			 
		}
		
		scanf("%c %d",&MARKER,&initiator);
		if(initiator > size-1)
			MPI_Abort(MPI_COMM_WORLD,0);
	}
	printf("\n");
	MPI_Finalize();
	return 0;
}
