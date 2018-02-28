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

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
int buffer_index = 0;
int buffer_length = 0;
int buffer_size = 0;
# define TIMER_VALUE 15
# define AHOST 0
# define BHOST 1
# define BUFFER_SIZE 1500
struct msg message_buffer[BUFFER_SIZE];
struct pkt resend_buffer[BUFFER_SIZE];
int base;
int next_seqnum;
int sent_seqnum;
int window_size;
int last_received;
int expected_packet;
//int resend_length = 0;
void A_output(message)
  struct msg message;
{
	if(next_seqnum < base+window_size)
	{
		struct pkt packet;
                packet.seqnum = next_seqnum;
                int checkSum = next_seqnum;
                checkSum = checkSum + packet.acknum;
                int len = sizeof(message.data);
                for(int i=0;i<20;i++)
                {       
                        checkSum = checkSum + (int)message.data[i];
                 
                }
                strcpy(packet.payload,message.data);
                packet.checksum = checkSum;
		printf("\n Sent Msg : %s \t Seqnum : %d \t CheckSum : %d \n",packet.payload,packet.seqnum,checkSum);
                tolayer3(AHOST,packet);
		//sent_seqnum = packet.seqnum;
                if(base == next_seqnum)
		{	
			starttimer(AHOST,30);
		}
		resend_buffer[next_seqnum] = packet;
		//resend_length++;
		next_seqnum++;
                
	}
	else
	{
		//struct msg buf_msg;
                //strcpy(buf_msg.data,message.data);
                message_buffer[buffer_length] = message;
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
       	printf("\n Coming here \n"); 
        if(packet.checksum == check)    // base or sent_seqnum
        {
                base = packet.acknum + 1;
                while((next_seqnum < base+window_size) && (buffer_index < buffer_length))//if(buffer_size > 0)  //check this
                {
                struct pkt packet;
                packet.seqnum = next_seqnum;
                int checkSum = next_seqnum;
                checkSum = checkSum + packet.acknum;
                struct msg buf_msg;
                buf_msg = message_buffer[buffer_index];
                for(int i=0;i<20;i++)
                {       
                        checkSum = checkSum + (int)buf_msg.data[i];
                        
                }
                strcpy(packet.payload,buf_msg.data);
                packet.checksum = checkSum;
                //starttimer(AHOST,30);
		//printf("\n Sent from A_input Msg : %s \n",buf_msg.data);
                tolayer3(AHOST,packet);
		//sent_seqnum = packet.seqnum;
		resend_buffer[next_seqnum] = packet;
		next_seqnum++;
		//resend_length++;
                //buffer_size--;
                buffer_index++;
                }
		if(base == next_seqnum)
		{
                	stoptimer(AHOST);
		}
		else
		{
			starttimer(AHOST,30);
		}
        }

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	starttimer(AHOST,30);	
	for(int i=base;i<next_seqnum;i++)
	{
		tolayer3(AHOST,resend_buffer[i]);
	}	
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	window_size = getwinsize();
	next_seqnum = 0;
        base = 0;
        memset(&message_buffer,0,1000);
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
	int ackNum;
        if(sum == packet.checksum)
        {	
			if(packet.seqnum == expected_packet)
			{
				tolayer5(BHOST,packet.payload);
				ackNum = packet.seqnum;
				//last_received = packet.seqnum;
				expected_packet++;
			}
			else
			{
				ackNum = expected_packet-1;;
				//send ack for last received	
			}

			send_packet.acknum = ackNum;
                        int check = send_packet.acknum;
                        check = check + send_packet.seqnum;
                        for(int i=0;i<20;i++)
                        {
                                check = check + (int)send_packet.payload[i];

                        }
                        send_packet.checksum = check;
                        tolayer3(BHOST,send_packet);
        }

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	last_received = 0;
	expected_packet = 0;
}
