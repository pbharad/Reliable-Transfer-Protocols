#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
# define TIMER_VALUE 15
# define AHOST 0
# define BHOST 1
# define BUFFER_SIZE 1500
int buffer_index = 0;
int buffer_length = 0;
int buffer_size = 0;
struct msg message_buffer[BUFFER_SIZE];
struct pkt resend_packet;
int seqnum_sent;
int A_status = 0; 
int B_status = 0; 

void A_output(message)
  struct msg message;
{		
	if(A_status == 0)
	{
		struct pkt packet;
		packet.seqnum = seqnum_sent;
		int checkSum = seqnum_sent;
		checkSum = checkSum + packet.acknum;
		int len = sizeof(message.data);
		for(int i=0;i<20;i++)
		{	
			checkSum = checkSum + (int)message.data[i];
			
		}
		A_status = 1;
		strcpy(packet.payload,message.data);
		packet.checksum = checkSum;
		starttimer(AHOST,10);
		tolayer3(AHOST,packet);
		resend_packet = packet;
	}
	else
	{	
		struct msg buf_msg;
		strcpy(buf_msg.data,message.data);
		message_buffer[buffer_length] = buf_msg;
		buffer_length++;
		buffer_size++;
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
	int check = 0;
	check = check + packet.acknum;
	check = check + packet.seqnum;
	for(int i=0;i<20;i++)
	{	
		check = check + (int)packet.payload[i];
			
	}
	
	if(packet.acknum == seqnum_sent && packet.checksum == check)
	{
		


		stoptimer(AHOST);
		if(seqnum_sent == 0)
		{
			seqnum_sent = 1;
		}else{
			seqnum_sent = 0;
		}
		if(buffer_size > 0)  //check this
		{
		struct pkt packet;
		packet.seqnum = seqnum_sent;
		int checkSum = seqnum_sent;
		checkSum = checkSum + packet.acknum;
		struct msg buf_msg;
		buf_msg = message_buffer[buffer_index];
		for(int i=0;i<20;i++)
		{	
			checkSum = checkSum + (int)buf_msg.data[i];
			
		}
		strcpy(packet.payload,buf_msg.data);
		packet.checksum = checkSum;
		starttimer(AHOST,10);
		tolayer3(AHOST,packet);
		resend_packet = packet;
		buffer_size--;
		buffer_index++;
		}
		else
		{
			A_status = 0;
		}	
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{	
	tolayer3(AHOST,resend_packet);
	starttimer(AHOST,10);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	seqnum_sent = 0;
	A_status = 0;
	memset(message_buffer,0,1000);
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
	int sum = 0;
	struct pkt send_packet;
	sum = sum + packet.seqnum;
	sum = sum + packet.acknum;
	int len = sizeof(packet.payload);
	for(int i=0;i<20;i++)
	{
		sum = sum + (int)packet.payload[i];

	}
	if(sum == packet.checksum)
	{
			
			send_packet.acknum = packet.seqnum;
			int check = send_packet.acknum;
			check = check + send_packet.seqnum;
			for(int i=0;i<20;i++)
			{
				check = check + (int)send_packet.payload[i];

			}	
			send_packet.checksum = check;	
			tolayer3(BHOST,send_packet);
		if(packet.seqnum == B_status)
		{
			tolayer5(BHOST,packet.payload);
		if(B_status == 0){
			B_status = 1;
		}
		else
		{
			B_status--;
		}
		}
	}
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}
