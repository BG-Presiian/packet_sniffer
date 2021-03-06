//Presiian Iskrenov
//netid : presiian
//Turned in: 4/24/2020
//Spring 2020
#define RETSIGTYPE void
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef setsignal_h
#define setsignal_h

RETSIGTYPE (*setsignal(int, RETSIGTYPE (*)(int)))(int);
#endif
int count_ip;
int count_arp;
int count_icmp;
int count_tcp;
int count_dns;
int count_smtp;
int count_pop;
int count_imap;
int count_http;
char cpre580f98[] = "netdump";

void raw_print(u_char *user, const struct pcap_pkthdr *h, const u_char *p);

int packettype;

char *program_name;

/* Externs */
extern void bpf_dump(const struct bpf_program *, int);

extern char *copy_argv(char **);

/* Forwards */
 void program_ending(int);

/* Length of saved portion of packet. */
int snaplen = 1500;;

static pcap_t *pd;

extern int optind;
extern int opterr;
extern char *optarg;
int pflag = 0, aflag = 0;

int
main(int argc, char **argv)
{
	int cnt, op, i, done = 0;
	bpf_u_int32 localnet, netmask;
	char *cp, *cmdbuf, *device;
	struct bpf_program fcode;
	 void (*oldhandler)(int);
	u_char *pcap_userdata;
	char ebuf[PCAP_ERRBUF_SIZE];

	cnt = -1;
	device = NULL;
	
	if ((cp = strrchr(argv[0], '/')) != NULL)
		program_name = cp + 1;
	else
		program_name = argv[0];

	opterr = 0;
	while ((i = getopt(argc, argv, "pa")) != -1)
	{
		switch (i)
		{
		case 'p':
			pflag = 1;
		break;
		case 'a':
			aflag = 1;
		break;
		case '?':
		default:
			done = 1;
		break;
		}
		if (done) break;
	}
	if (argc > (optind)) cmdbuf = copy_argv(&argv[optind]);
		else cmdbuf = "";

	if (device == NULL) {
		device = pcap_lookupdev(ebuf);
		if (device == NULL)
			error("%s", ebuf);
	}
	pd = pcap_open_live(device, snaplen,  1, 1000, ebuf);
	if (pd == NULL)
		error("%s", ebuf);
	i = pcap_snapshot(pd);
	if (snaplen < i) {
		warning("snaplen raised from %d to %d", snaplen, i);
		snaplen = i;
	}
	if (pcap_lookupnet(device, &localnet, &netmask, ebuf) < 0) {
		localnet = 0;
		netmask = 0;
		warning("%s", ebuf);
	}
	/*
	 * Let user own process after socket has been opened.
	 */
	setuid(getuid());

	if (pcap_compile(pd, &fcode, cmdbuf, 1, netmask) < 0)
		error("%s", pcap_geterr(pd));
	
	(void)setsignal(SIGTERM, program_ending);
	(void)setsignal(SIGINT, program_ending);
	/* Cooperate with nohup(1) */
	if ((oldhandler = setsignal(SIGHUP, program_ending)) != SIG_DFL)
		(void)setsignal(SIGHUP, oldhandler);

	if (pcap_setfilter(pd, &fcode) < 0)
		error("%s", pcap_geterr(pd));
	pcap_userdata = 0;
	(void)fprintf(stderr, "%s: listening on %s\n", program_name, device);
	if (pcap_loop(pd, cnt, raw_print, pcap_userdata) < 0) {
		(void)fprintf(stderr, "%s: pcap_loop: %s\n",
		    program_name, pcap_geterr(pd));
		exit(1);
	}
	pcap_close(pd);
	exit(0);
}

/* routine is executed on exit */
void program_ending(int signo)
{
	struct pcap_stat stat;

	if (pd != NULL && pcap_file(pd) == NULL) {
		(void)fflush(stdout);
		putc('\n', stderr);
		if (pcap_stats(pd, &stat) < 0)
			(void)fprintf(stderr, "pcap_stats: %s\n",
			    pcap_geterr(pd));
		else {
			(void)fprintf(stderr, "%d packets received by filter\n",
			    stat.ps_recv);
			(void)fprintf(stderr, "%d packets dropped by kernel\n",
			    stat.ps_drop);
		}
		
	}


	printf("Total IPV4: %d\n",count_ip);
	printf("Total ARP: %d\n",count_arp);
	printf("Total ICMP: %d\n",count_icmp);
	printf("Total TCP: %d\n", count_tcp);
	printf("Total DNS: %d\n", count_dns);
	printf("Total SMTP: %d\n",count_smtp);
	printf("Total POP: %d\n",count_pop);
	printf("Total IMAP: %d\n", count_imap);
	printf("Total HTTP: %d\n", count_http);
	exit(0);
}

/* Like default_print() but data need not be aligned */
void
default_print_unaligned(register const u_char *cp, register u_int length)
{
	register u_int i, s;
	register int nshorts;

	nshorts = (u_int) length / sizeof(u_short);
	i = 0;
	while (--nshorts >= 0) {
		if ((i++ % 8) == 0)
			(void)printf("\n\t\t\t");
		s = *cp++;
		(void)printf(" %02x%02x", s, *cp++);
	}
	if (length & 1) {
		if ((i % 8) == 0)
			(void)printf("\n\t\t\t");
		(void)printf(" %02x", *cp);
	}
}

/*
 * By default, print the packet out in hex.
 */
void
default_print(register const u_char *bp, register u_int length)
{
	register const u_short *sp;
	register u_int i;
	register int nshorts;

	if ((long)bp & 1) {
		default_print_unaligned(bp, length);
		return;
	}
	sp = (u_short *)bp;
	nshorts = (u_int) length / sizeof(u_short);
	i = 0;
	while (--nshorts >= 0) {
		if ((i++ % 8) == 0)
			(void)printf("\n\t");
		(void)printf(" %04x", ntohs(*sp++));
	}
	if (length & 1) {
		if ((i % 8) == 0)
			(void)printf("\n\t");
		(void)printf(" %02x", *(u_char *)sp);
	}
}

/*
insert your code in this routine

*/

void raw_print(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
        u_int length = h->len;
        u_int caplen = h->caplen;
	uint16_t e_type;
	uint16_t c_type;
	int t = 14;
	int k=34;
	int e = 54;
	e_type=p[12] *256 + p[13];
	c_type= p[20] *256 + p[21];
	int z=0;
	int b =0;
	//int i = 0;
	int o =0;
	int l=0;
	int a =0;
        default_print(p, caplen);
	printf("\n");
	printf("=====================Decoding ETHERNET HEADER=====================\n");
	printf("DEST Address = %02x:%02x:%02x:%02x:%02x:%02x\n", p[0],p[1],p[2],p[3],p[4],p[5]);
	printf("SRC Address = %02x:%02x:%02x:%02x:%02x:%02x\n", p[6],p[7],p[8],p[9],p[10],p[11]);
	printf("E_type = 0x%04x\n",e_type);

	if(e_type == 0x0800)
	{
	  printf("Payload = IPV4\n");
	  printf("\n\t=====================Decoding IP HEADER=====================\n");
	  
	  int tests = (unsigned char) p[14] %0x10;
	  
	  printf("\tVersion Number = %01hhx\n", (unsigned char)p[14]>>4);
	  printf("\tHeader Length = %d bytes\n", tests * 4);
	  printf("\tType of Service = 0x%02x\n", p[15]);
	  printf("\tLength = %02x%02x\n", p[16],p[17]); 
	  
	  printf("\tIdentifier = 0x%02x%02x\n", p[18],p[19]);
	 
	  char test[4];
	  test[0] = p[20] >>4;
	  printf("\tFlags = ");
	 
	   switch(test[0]){
	   case 0:
		printf("000\n"); 
		printf("\t\t(Reserved):0 \n");
	        printf("\t\t(DF):0 -disabled\n");
		printf("\t\t(MF):0 -disabled\n");
		break;
	   case 1:
		printf("000\n"); 
		printf("\t\t(Reserved):0 \n");
	        printf("\t\t(DF):0 -disabled\n");
		printf("\t\t(MF):0 -disabled\n");
		break;
	   case 2:
		printf("001\n"); 
		printf("\t\t(Reserved):0 \n");
	        printf("\t\t(DF):0 -disabled\n");
		printf("\t\t(MF):1 -enabled\n");
		break;
	   case 3:
		printf("001\n"); 
		printf("\t\t(Reserved):0 \n");
	        printf("\t\t(DF):0 -disabled\n");
		printf("\t\t(MF):1 -enabled\n");
		break;
	   case 4:
		printf("010\n"); 
		printf("\t\t(Reserved):0 \n");
	        printf("\t\t(DF):1 -enabled\n");
		printf("\t\t(MF):0 -disabled\n");
		break;
	   case 5:
		printf("010\n");
		printf("\t\t(Reserved):0 \n");
	        printf("\t\t(DF):1 -disabled\n");
		printf("\t\t(MF):0 -enabled\n");
		break;
	   case 6:
		printf("011\n");
		printf("\t\t(Reserved):0 \n");
	        printf("\t\t(DF):1 -enabled\n");
		printf("\t\t(MF):1 -enabled\n"); 
		break;
	   case 7:
		printf("011\n"); 
		printf("\t\t(Reserved):0 \n");
	        printf("\t\t(DF):1 -enabled\n");
		printf("\t\t(MF):1 -enabled\n");
		break;
	   default:
	        printf("\nInvalid Hex Digit\n");
	    }
	  
	  int temp = (unsigned char) p[23] %0x10;
	  printf("\tOffset= %02x\n", p[21]);
	  printf("\tTTL = %02x\n", p[22]);
	  printf("\tProtocol = %hhx -> \n", temp);
	  printf("\tChecksum = 0x%02x%02x\n", p[24],p[25]);
	  printf("\tSource IP Address = %d.%d.%d.%d\n", p[26],p[27],p[28],p[29]);
	  printf("\tDestination IP Address = %d.%d.%d.%d\n", p[30],p[31],p[32],p[33]);
	 
	  printf("\tData: ");
	  while(l<=caplen)
	    {
	      printf("%d.",p[k+l]);
	      l++;
	    }
	  printf("\n");
	  count_ip++;
	  if(temp == 1)
	  {
	   count_icmp++;
	   printf("ICMP \n");
	   printf("\t\t=====================Decoding ICMP HEADER=====================\n");
	   printf("\t\tType = %hhx\n",(unsigned char) p[34] >>4);
	   printf("\t\tCode = %hhx\n", (unsigned char) p[35] >> 4);
	   int type = (unsigned char) p[34] >>4;
	   int code = (unsigned char) p[35] >>4;
	 
	   if(type == 0)
	   {
	     printf("\t\t\tICMP Destination Unreachable\n");
	     if(code == 0)
	     {
		printf("\t\t\tNetwork Unreachable\n");
	     }
	     else if(code == 1)
	     {
		printf("\t\t\tHost Unreachable\n");
	     }
	     else if(code == 2)
	     {
		printf("\t\t\tProtocol unreachable on the target host\n");
	     }
	     else if(code == 3)
	     {
		printf("\t\t\tPort unreachable on the target host\n");
	     }
	     else if(code == 4)
	     {
		printf("\t\t\tFragmentation needed and 'don't fragment' bit is set\n");
	     }
	     else if(code == 5)
	     {
		printf("\t\t\tSource route failed\n");
	     }
	   }
	   else if(type == 11)
	   {
	     printf("\t\t\tICMP Time Exceeded\n");
	   }
	   else if(type == 5)
	   {
	     printf("\t\t\tICMP Redirection\n");
	     if(code == 0)
	     {
		printf("\t\t\tNetwork-based redirect\n");
	     }
	     else if(code == 1)
	     {
		printf("\t\t\tHost-based redirect\n");
	     }
	     else if(code == 2)
	     {
		printf("\t\t\tNetwork-based redirect of the type of service specified\n");
	     }
	     else if(code == 3)
	     {
		printf("\t\t\tHost-based redirect of the type of service specified\n");
	     }
	   }
	   else if(type ==8)
	   {
		printf("\t\t\tICMP Echo Request\n");
	   }
	   else if(type==8 && type ==0)
	   {
		printf("\t\t\tICMP Echo Request and Reply\n");
	   }
	   else if(type ==13)
	   {
		printf("\t\t\tICMP Timestamp Request\n");
	   }
	   else if(type ==14)
	   {
		printf("\t\t\tICMP Timestamp Reply\n");
	   }
	  
	   printf("\t\tChecksum = 0x%02x%02x\n",p[36],p[37]);
	   printf("\t\tParameter = 0x%02x%02x%02x%02x\n",p[38],p[39],p[40],p[41]);
	   printf("\t\tData = 0x%02x%02x%02x%02x\n",p[42],p[43],p[44],p[45]);
	   printf("\t\t=============================================================\n");
	  }
	  else if(temp == 6)
 	  {
	   int src_port = (p[34]<<8) | p[35];
	   int dest_port = (p[36]<<8) | p[37];
	   count_tcp++;
	   printf("TCP \n");
	   printf("\t\t=====================Decoding TCP HEADER=====================\n");
	   printf("\t\tSource Port Number = %02x%02x\n",p[34],p[35]);
	   if(p[34] == 0x35 || p[35] == 0x35)
	   {
		count_dns++;
	   }
	   else if(p[36] == 0x35 || p[37] == 0x35)
	   {
		count_dns++;
	   }
	   printf("\t\tDestination Port Number = %02x%02x\n",p[36],p[37]);
	   
	   int ind;
	   
	   //SMTP
	   if(dest_port == 25 || dest_port == 587 || dest_port == 465)
	   {
	     count_smtp++;
	     //decode now
	     printf("\t\tDecoding SMTP: \n");
	     for(ind = 53; ind<caplen; ind++)
	     {
	       printf("%c ", p[ind+1]);
	     }
	     
	   }
	   //pop
	   if(src_port == 110 || src_port == 995)
	   {
	     count_pop++;
	     printf("\t\tDecoding POP: \n");
	     for(ind = 53; ind<caplen; ind++)
	     {
	       printf("%c ", p[ind+1]);
	     }
	     
	   }
	   //imap
	   if(src_port == 143 || src_port == 993)
	   {
	     count_imap++;
	     printf("\t\tDecoding IMAP: \n");
	     for(ind = 53; ind<caplen; ind++)
	     {
	       printf("%c ", p[ind+1]);
	     }
	     
	   }
	   //http
	   if(src_port == 80 || src_port == 443 || src_port == 8080)
	   {
	     count_http++;
	     printf("\t\tDecoding HTTP: \n");
	     for(ind = 53; ind<caplen; ind++)
	     {
	       printf("%c ", p[ind+1]);
	     }
	     
	   }
		
	   printf("\n\t\tSequence Number = 0x%02x%02x%02x%02x\n",p[38],p[39],p[40],p[41]);
	   printf("\t\tAcknowledgment Number = 0x%02x%02x%02x%02x\n",p[42],p[43],p[44],p[45]);
	   printf("\t\tHdr-Len = %02x bytes\n",p[46]);
	   
	   int tester[4];
	   int whi = 0;
	   
	   tester[1] = (unsigned char) p[47] %0x10;
	   tester[0] = (unsigned char) p[47]>>4;
	   
	   printf("\t\tflags = \n");
	   while(whi<2)
	   {
	   switch(tester[whi]){
	   case 0:
		printf("\t\t0000\n"); 
		break;
	   case 1:
		printf("\t\t0001\n");
		if(tester[0] == 0001)
		{
		  printf("\t\t\tACK - acknowledgement number is valid\n");
		}
		else if(tester[1] == 0001){
		  printf("\t\t\tFIN - Finish Packet\n");
		}
		break;
	   case 2:
		printf("\t\t0010\n"); 
		if(tester[0] == 0010)
		{
		  printf("\t\t\tURG - Packet contains urgent data\n");
		}
		else if(tester[1] == 0010){
		  printf("\t\t\tSYN - Synchronize Packet\n");
		}
		break;
	   case 3:
		printf("\t\t0011\n"); 
		if(tester[0] == 0011)
		{
		  printf("\t\t\tURG - Packet Contains Urgent Data\n");
		  printf("\t\t\tACK - Acknowledgement Number is Valid\n");
		}
		else if(tester[1] == 0011){
		  printf("\t\t\tSYN - Synchronize Packet\n");
		  printf("\t\t\tFIN - Finish Packet\n");
		}
		break;
	   case 4:
		printf("\t\t0100\n"); 
		if(tester[0] == 0100)
		{
		 
		}
		else if(tester[1] == 0100){
		  printf("\t\t\tRST - Reset Packet\n");
		}
		break;
	   case 5:
		printf("\t\t0101\n");
		if(tester[0] == 0101)
		{
		  printf("\t\t\tACK - Acknowledgement number is valid\n");
		}
		else if(tester[1] == 0101){
		  printf("\t\t\tRST - Reset Packet\n");
		  printf("\t\t\tFIN - Finish Packet\n");
		}
		break;
	   case 6:
		printf("\t\t0110\n");
		if(tester[0] == 0110)
		{
		  printf("\t\t\tURG - Packet Contains Urgent Data\n");
		}
		else if(tester[1] == 0110){
		  printf("\t\t\tRST - Reset Packet\n");
		  printf("\t\t\tSYN - Synchronize Packet\n");
		}
		break;
	   case 7:
		printf("\t\t0111\n");
		if(tester[0] == 0111)
		{
		  printf("\t\t\tURG - Packet Contains Urgent Data\n");
		  printf("\t\t\tACK - Acknowledgement number is valid\n");
		}
		else if(tester[1] == 0111){
		  printf("\t\t\tRST - Reset Packet\n");
		  printf("\t\t\tSYN - Synchronize Packet\n");
		  printf("\t\t\tFIN - Finish Packet\n");
		}
		break;
	   case 8:
		printf("\t\t1000\n"); 
		if(tester[0] == 1000)
		{
		  
		}
		else if(tester[1] == 1000){
		  printf("\t\t\tPSH - Data Should be pushed to the application\n");
		}
		break;
	   case 9:
		printf("\t\t1001\n"); 
		if(tester[0] == 1001)
		{
		  printf("\t\t\tACK - Acknowledgement Number is valid\n");
		}
		else if(tester[1] == 1001){
		  printf("\t\t\tPSH - data should be pushed to the application\n");
		  printf("\t\t\tFIN - Finish Packet\n");
		}
		break;
	   case 'a':
		printf("\t\t1010\n"); 
		if(tester[0] == 'a')
		{
		  printf("\t\t\tURG - Packet contains urgent Data\n");
		}
		else if(tester[1] == 'a'){
		  printf("\t\t\tPSH - Data should be pushed to the application\n");
		  printf("\t\t\tSYN - Sycnchronize packet\n");
		}
		break;
	   case 'b':
		printf("\t\t1011\n"); 
		if(tester[0] == 'b')
		{
		  printf("\t\t\tURG - Packet Contains urgent data\n");
		  printf("\t\t\tACK - Acknowledgement number is valid\n");
		}
		else if(tester[1] == 'b'){
		  printf("\t\t\tPSH - Data should be pushed to the application\n");
		  printf("\t\t\tSYN - Synchronize Packet\n");
		  printf("\t\t\tFIN - Finish Packet\n");
		}
		break;
	   case 'c':
		printf("\t\t1100\n"); 
		if(tester[0] == 'c')
		{
		  
		}
		else if(tester[1] == 'c'){
		  printf("\t\t\tPSH - Data should be pushed to the application\n");
		  printf("\t\t\tRST - Reset packet\n");
		}
		break;
	   case 'd':
		printf("\t\t1101\n"); 
		if(tester[0] == 'd')
		{
		  printf("\t\t\tACK - Acknowledgement number is valid\n");
		}
		else if(tester[1] == 'd'){
		  printf("\t\t\tPSH - Data should be pushed to the application\n");
		  printf("\t\t\tRST - Reset Packet\n");
		  printf("\t\t\tFIN - Finish Packet\n");
		}
		break;
	   case 'e':
		printf("\t\t1110\n"); 
		if(tester[0] == 'e')
		{
		  printf("\t\t\tURG - Packet contains urgent data\n");
		}
		else if(tester[1] == 'e'){
		  printf("\t\t\tPSH - Data should be pushed to the application\n");
		  printf("\t\t\tRST - Reset Packet\n");
		  printf("\t\t\tSYN - Synchronize Packet\n");
		}
		break;
	   case 'f':
		printf("\t\t1111\n"); 
		if(tester[0] == 'f')
		{
		  printf("\t\t\tURG - Packet Contains urgent data\n");
		  printf("\t\t\tACK - Acknowledgement number is valid\n");
		}
		else if(tester[1] == 'f'){
		  printf("\t\t\tPSH - Data should be pushed to the application\n");
		  printf("\t\t\tRST - Reset Packet\n");
		  printf("\t\t\tSYN - Synchronize Packet\n");
		  printf("\t\t\tFIN - Finish Packet\n");
		}
		break;
	   default:
	        printf("\n\t\tInvalid Hex Digit\n");
	    }
	    whi++;
	    printf("\n");
	   }
	   printf("\n");
	   printf("\t\tWindow Size = %02x%02x\n",p[48],p[49]);
	   printf("\t\tChecksum = 0x%02x%02x\n",p[50],p[51]);
	   printf("\t\tUrgent Pointer = %02x%02x\n",p[52],p[53]);
	   printf("\t\tOptions: ");
	   if(p[e] == 00)
	   {
	     printf("No Options");
	   }
	   else{
	
	    while(a<=caplen)
	    {
	      printf("%d.",p[e+a]);
	      a++;
	    }
	   }
	  printf("\n");
	  
	   printf("\t\t=============================================================\n");
	  }
	  else if(temp == 17)
	  {
	   printf("UDP \n");
	  }
	  
	}
	else if(e_type == 0x0806){
	  printf("Payload = ARP\n");
	  printf("\n\t=====================Decoding ARP HEADER=====================\n");
	  printf("\tHardware Type = %02x%02x\n", p[14],p[15]);
	  printf("\tProtocol Type = %02x%02x\n", p[16],p[17]);
	  printf("\tHLEN = %02x\n", p[18]);
	  printf("\tPLEN = %02x\n", p[19]);
	  if(c_type == 0x0002) //Will go to reply if enters this 
	  {
	    printf("\tOperation = %02x%02x\n", p[20],p[21]);
	    printf("\t\t=====================Decoding ARP Reply=====================\n");
	    printf("\t\tDEST Address = %d.%d.%d.%d.%d.%d\n", p[0],p[1],p[2],p[3],p[4],p[5]);
	  
	    printf("\t\tSRC Address = %d.%d.%d.%d.%d.%d\n", p[6],p[7],p[8],p[9],p[10],p[11]);
	    printf("\t\tE_type = 0x%04x\n",e_type);
	
	    printf("\t\tTarget Hardware Address = %02x.%02x.%02x.%02x.%02x.%02x\n", 	                 		    p[32],p[33],p[34],p[35],p[36],p[37]);
	    printf("\t\tSource Hardware Address = %02x.%02x.%02x.%02x.%02x.%02x\n", 	                 		    p[22],p[23],p[24],p[25],p[26],p[27]);
	    printf("\t\t0x806\n");
	    printf("\t\tData:");
	    
	    while(o<=caplen)
	    {  
	     printf("%d.",p[t+o]);
	 	
	      o++;
	    }
	    printf("\n");
	    printf("\t\t=============================================================\n");
	  }
	  else if(c_type  == 0x0001) //will go to request if entered
	  {
	    printf("\tOperation = %02x%02x\n", p[20],p[21]);
	    printf("\t\t=====================Decoding ARP Request=====================\n");
	    printf("\t\tDEST Address = %02x.%02x.%02x.%02x.%02x.%02x\n", p[0],p[1],p[2],p[3],p[4],p[5]);
	    printf("\t\tSRC Address = %02x.%02x.%02x.%02x.%02x.%02x\n", p[6],p[7],p[8],p[9],p[10],p[11]);
	    printf("\t\tE_type = 0x%04x\n",e_type);
	    printf("\t\tBroadcast = %02x.%02x.%02x.%02x.%02x.%02x\n", 	                 		    		    p[32],p[33],p[34],p[35],p[36],p[37]);
	    printf("\t\tSource Hardware Address = %02x.%02x.%02x.%02x.%02x.%02x\n", 	                 		    p[22],p[23],p[24],p[25],p[26],p[27]);
	    printf("\t\t0x806\n");
	    printf("\t\tData:");
	    while(b<=caplen)
	    {
	      printf("%d.",p[t+b]);
	      b++;
	    }
	    printf("\n");
	    printf("\t\t=============================================================\n");
	     
	  }
	  printf("\tSender Hardware Address = %02x.%02x.%02x.%02x.%02x.%02x\n", 	                 		  p[22],p[23],p[24],p[25],p[26],p[27]);
	  printf("\tSender Protocol Address = %02x.%02x.%02x.%02x\n", 	                 		  		  p[28],p[29],p[30],p[31]);
	  printf("\tTarget Hardware Address = %02x.%02x.%02x.%02x.%02x.%02x\n", 	                 		  p[32],p[33],p[34],p[35],p[36],p[37]);
 	  printf("\tTarget Protocol Address = %02x.%02x.%02x.%02x\n", 	                 		  	    p[38],p[39],p[40],p[41]);
	  count_arp++;
	}
	
        putchar('\n');
} 
