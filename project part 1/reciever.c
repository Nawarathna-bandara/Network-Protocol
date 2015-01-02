/*===================================================================================================
 *  Group 18 - E/10/049, E/10/170 - Project II - Part I
 ====================================================================================================*/

#include "header.h"

int main(void)
{
	int sockfd;
	char packet[PACKET_SIZE];
	char *buffer;
	FILE *fp;
	int end=1, n = 0,count=0,ack=0,seq=0,rand,dmg=0;
	socklen_t len;
	uint32_t SEED;
	long lseek=0;
	struct sockaddr_in addr;
	
	sockfd=setsocket(); 
	SEED = (int)time(NULL);
	len = sizeof(struct sockaddr);
	buffer = malloc (BUFFFER_SIZE);
	
	printf("receiving data....\n");
	
	while(end)
	{	
		/* recieve a packet from sender */
		n= recvfrom(sockfd, &packet, PACKET_SIZE, 0, (struct sockaddr *)&addr, &len);
		/* if a previous packet from this window is not damaged,*/
		if(dmg==0){
			
			/* check wether this packet is damaged using a random number */
			rand=URandom(&SEED, 1000);
			if(rand<=1000*ERROR_RATIO)
				dmg=1;
				
			/* if it's not damaged then copy it to the buffer */
			if(dmg==0){
				memcpy((buffer+lseek), packet, n);
				lseek += n;
				
				/* if the current packet is the last packet then stop then break */
				if (packet[n-1] == '\0'){
					end = 0;
					
					/* ack=8 is used to inform the sender all the packets were recieved */
					ack=8;  
					sendto(sockfd, &ack, 1, 0, (struct sockaddr*)&addr,sizeof(struct sockaddr));
					break;
				}
			}
			/* if the packet is damaged then make damaged packet sequence number  = ack */
			else
				ack=(seq+count)%SEQ_NO;	
		}	
		count++;
		
		/* if a complete window or partial window including last packet recieved, */
		if(count==WINDOW_SIZE || packet[n-1] == '\0'){
			
			/* if the window is damage free, ack = next sequence number */
			if(dmg==0)
				ack=(ack+count)%SEQ_NO;
			
			/* send the ack */
			sendto(sockfd, &ack, 1, 0, (struct sockaddr*)&addr,sizeof(struct sockaddr));
			
			/* move window to next required packet */
			seq=ack;
			count=0;
			dmg=0;
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
