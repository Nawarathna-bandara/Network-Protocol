/*===================================================================================================
 *  Group 18 - E/10/049, E/10/170 - Project II - Part II
 ====================================================================================================*/

#include "header.h"

int main(void)
{
	int sockfd;
	char packet[PACKET_SIZE];
	char *buffer;
	FILE *fp;
	int end=1, n = 0,count=0,ack=0,rand;
	socklen_t len;
	uint32_t SEED;
	long lseek=0;
	struct sockaddr_in addr;

	int oldwindow[WINDOW_SIZE],nextwindow[WINDOW_SIZE],a=0,k,win=WINDOW_SIZE,dmg=0;
	
	for(k=0;k<WINDOW_SIZE;k++){
		oldwindow[k]=PACKET_SIZE*k;
	}
	
	sockfd=setsocket(); 
	SEED = (int)time(NULL);
	len = sizeof(struct sockaddr);
	buffer = malloc (BUFFFER_SIZE);
	
	printf("receiving data....\n");
	
	while(1)
	{
		/* recieve a packet from sender */
		n= recvfrom(sockfd, &packet, PACKET_SIZE, 0, (struct sockaddr *)&addr, &len);
		
		/* if the recieved packet is the last one then adjust the window size */
		if(packet[n-1] == '\0'){
				win=count+1;
		}
		
		/* genrate a random number and check wether the packet is damaged */
		rand=URandom(&SEED, 1000);
		if(rand<=1000*ERROR_RATIO){
			ack+=pow(2,(WINDOW_SIZE-count-1));
			nextwindow[a]=oldwindow[count];
			a++;
			dmg++;
		}
		
		/* if it's not damaged then copy it to the buffer */
		else{
			memcpy((buffer+oldwindow[count]), packet, n);
			
			if(packet[n-1] == '\0'){
				lseek=oldwindow[count]+n;
				end=0;
			}
		}	
		
		/* if last packet recieved and all are error free then stop the reciever */
		if(end==0 && ack==0){
			ack=50;
			sendto(sockfd, &ack, 1, 0, (struct sockaddr*)&addr,sizeof(struct sockaddr));
			break;
		}
		count++;
		
		/* if a complete window or partial window including last packet recieved, */
		if(count==win){
			
			/* send the ack */
			sendto(sockfd, &ack, 1, 0, (struct sockaddr*)&addr,sizeof(struct sockaddr));
			
			k=1;
			while(a<WINDOW_SIZE && end==1){
				nextwindow[a]=oldwindow[count-1]+PACKET_SIZE*k;
				a++;
				k++;
			}
			
			/* move window to next required packet */
			ack=0;
			count=0;
			a=0;
			
			/* if the last packet arrived then we will not get a full window.
			 *  only damaged packets will come as retransmission. So winsize = no of damages */
			if(end==0)
				win=dmg;
				
			dmg=0;
			
			/* copy the values in nextwindow to oldwindow */
			for(k=0;k<WINDOW_SIZE;k++){
				oldwindow[k]=nextwindow[k];
			}
		}
		
	}
	
	printf(" * %d bytes received\n", (int)lseek);
	
	/* open "recieved.txt" and write recieved data from buffer */
	if ((fp = fopen ("recieved.txt","wt")) == NULL){
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buffer , 1 , lseek-1 , fp);
	fclose(fp);
	
	/* compare two files using shell command "diff" to check wether both original and recieved data are same */
	system("diff data.txt recieved.txt -s");
	
	/* close the socket and free the used memory */
	close(sockfd);
	free(buffer);
	
	return 0;
}                            

/* function to genarate a random number between 0 to max */
double URandom(uint32_t *uPtr, double max)
{
	*uPtr = (3141592653 * (*uPtr) + 2718281829) & MAXINT;
	return ((double) *uPtr / (double) MAXINT * max);
}

/* function to initialize the socket */
int setsocket()
{
	int sockfd;
	struct sockaddr_in my_addr;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	bzero(&(my_addr.sin_zero), 8);
	bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
	
	return sockfd;
}

/*================================== end of code ===========================================*/
