/*===================================================================================================
 *  Group 18 - E/10/049, E/10/170 - Project II - Part I
 ====================================================================================================*/
#include "header.h"

int main(int argc, char **argv)
{
	int sockfd,ack=0,seq=0,count=0,len,filesize, i=0,j=0,diff;
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
	
	printf(" * file size: %d bytes \n * Packet size = %d bytes\n * Window size = %d \n", (int)filesize,PACKET_SIZE,WINDOW_SIZE);

    /* allocate memory to contain the whole file */
	buffer = (char *) malloc (filesize+1);

    /* copy the file into the buffer */
	fread (buffer,1,filesize,fp);
	
	/* get the current time */
	gettimeofday(&sendt, NULL);	
	
	/* main loop which runs until end of the file*/						
	while(i<= filesize)
	{
		/* get the length of next packet (at the end of file it can be less than packet size)*/
		if ((filesize+1-i) <= PACKET_SIZE)
			len = filesize+1-i;
		else 
			len = PACKET_SIZE;
		
		/* copy the required bytes from buffer to packet */			
		memcpy(packet, (buffer+i), len);
		
		/* send the packet */
		sendto(sockfd, &packet, len, 0, addr,sizeof(struct sockaddr));
		i+=len;
		count++;
		
		/* if a window is complely sent or we came to the end of the file, */
		if(count==WINDOW_SIZE || i > filesize){
			addrlen = sizeof(struct sockaddr);
			
			/* recieve the acknowledgement */
			recvfrom(sockfd, &ack,1, 0, addr, &addrlen);                 
			
			/* ack=8 is a special case which inform the sender that all the bytes were recieved */
			if(ack==8){
				printf(" file successfully sent\n");
				break;
				}
				
			/* set the next window starting position(i) according to the ack */
			else{
				diff=(ack>=seq)?ack-seq:ack-seq+SEQ_NO;
				seq=ack;
				i=j+diff*PACKET_SIZE;
				j=i;
				count=0;
			}
		}
		
	}
		
	/* get the current time */	
	gettimeofday(&recvt, NULL); 
	
	/* get the transfer time */                                                    
	tv_sub(&recvt, &sendt); 
	                                                              
	ti = (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;                                    
	printf(" * %d bytes sent in %.3f(ms)\n * Data rate: %f (Kbytes/s)\n",(int)i,ti,i/(float)ti);
	
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
