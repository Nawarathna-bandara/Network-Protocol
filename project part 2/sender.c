/*===================================================================================================
 *  Group 18 - E/10/049, E/10/170 - Project II - Part II
 ====================================================================================================*/
#include "header.h"

int main(int argc, char **argv)
{
	int sockfd,ack=0,count=0,len,filesize,a=0;
	float ti;
	struct sockaddr_in ser_addr;
	struct in_addr **addrs;
	struct timeval sendt, recvt;
	FILE *fp;
	char *buffer;
	char packet[PACKET_SIZE];
	socklen_t addrlen;
	
	if (argc != 2) {
		printf("usage : ./filename IPaddress");
	}
	
	int oldwindow[WINDOW_SIZE],nextwindow[WINDOW_SIZE],k;
	
	for(k=0;k<WINDOW_SIZE;k++){
		oldwindow[k]=PACKET_SIZE*k;
		printf("%d, ",oldwindow[k]);
	}
	printf("\n");
	
	/* set up the socket */
	addrs = (struct in_addr **)gethostbyname(argv[1])->h_addr_list;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);                           
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYTCP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);
	struct sockaddr *addr = (struct sockaddr *)&ser_addr;
	addrlen = sizeof(struct sockaddr_in);
	
	/* open the data file */
	if((fp = fopen ("data.txt","r+t")) == NULL){
		printf("Error: Data file doesn't exit\n");
		exit(0);
	}

	/* get the size of the file pointed by fp */
	fseek (fp , 0 , SEEK_END);
	filesize = ftell (fp);
	rewind (fp);
	
	printf(" * file size: %d bytes \n * Packet size = %d bytes\n * Window size = %d bytes\n", (int)filesize,PACKET_SIZE,WINDOW_SIZE);

    /* allocate memory to contain the whole file */
	buffer = (char *) malloc (filesize+1);

    /* copy the file into the buffer */
	fread (buffer,1,filesize,fp);
	buffer[filesize]='\0';
	
	/* get the current time */
	gettimeofday(&sendt, NULL);	
	
	/* main loop which runs until end of the file*/						
	while(1)
	{
		/* get the length of next packet (at the end of file it can be less than packet size)*/
		if ((filesize+1-oldwindow[count]) <= PACKET_SIZE)
			len = filesize+1-oldwindow[count];
		else 
			len = PACKET_SIZE;
		
		/* copy the required bytes from buffer to packet */			
		memcpy(packet, (buffer+oldwindow[count]), len);

		/* send the packet */
		sendto(sockfd, &packet, len, 0, addr,sizeof(struct sockaddr));
		count++;
		
		/* if a window is complely sent or we came to the end of the file, */
		if(count==WINDOW_SIZE || oldwindow[count] > filesize){
			
			/* recieve the acknowledgement */
			addrlen = sizeof(struct sockaddr);
			recvfrom(sockfd, &ack,2, 0, addr, &addrlen);                
			
			/* ack=50 is a special case which inform the sender that all the bytes were recieved */
			if(ack==50){
				printf(" file successfully sent\n");
				break;
				}
				
			/* decode the damaged packet numbers from ack  
				 * 
				 * ack is aninteger. its binary form will represent the damaged packets
				 * ex: ack = 13  ==>00....001101  ==> packets 1,3,4 damaged
				 *     ack = 27  ==>00....011011  ==> packets 1,2,4,5 damaged
				 *     ack = 13  ==>00....001000  ==> packet 4 damaged
				 *     ack = 0  - no damages
				 *     ack = 513 - end of transmission (assuming window size is not greater than 10)
				 * 
				 * */
			else{
				for(k=0;k<WINDOW_SIZE;k++){
					if(ack>=pow(2,(WINDOW_SIZE-k-1))){
						/* add damaged packets to nextwindow for retransmission */
						nextwindow[a]=oldwindow[k];
						ack-=(int)pow(2,(WINDOW_SIZE-k-1));
						a++;
					}
				}
				
				/* add packets other than damaged ones to next transmission */
				k=1;
				while(a<WINDOW_SIZE){
					nextwindow[a]=oldwindow[count-1]+PACKET_SIZE*k;
					a++;
					k++;
				}

				/* copy the values in nextwindow to oldwindow */
				for(k=0;k<WINDOW_SIZE;k++){
					oldwindow[k]=nextwindow[k];
				}
				
				count=0;
				a=0;
			}
		}
	}
		
	/* get the current time */	
	gettimeofday(&recvt, NULL); 
	
	/* get the transfer time */                                                    
	tv_sub(&recvt, &sendt); 
	                                                              
	ti = (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;                                    
	printf(" * %d bytes sent in %.3f(ms)\n * Data rate: %f (Kbytes/s)\n",oldwindow[count-1]+len,ti,(oldwindow[count-1]+len)/(float)ti);
	
	close(sockfd);
	fclose(fp);
	exit(0);
}

/* function to calculate the difference between two time values */
void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0){
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

/*================================== end of code ===========================================*/
