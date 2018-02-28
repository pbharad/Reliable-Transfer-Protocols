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
int buffer_index;
int buffer_length;
# define TIMER_VALUE 15
# define AHOST 0
# define BHOST 1
# define BUFFER_SIZE 3000
struct msg message_buffer[3000];
struct pkt resend_buffer[3000];
int base;
int next_seqnum;
int window_size;
int expected_packet;
struct pkt recv_buffer[3000];
float timeout_value[3000];
int timeout_ack[3000];
int recv_ack[3000];

void A_output(message)
  struct msg message;
{
	if(next_seqnum < base+window_size)
        {       
                struct pkt packet;
		memset(&packet,0,sizeof(packet));
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
                tolayer3(AHOST,packet);
                if(base == next_seqnum)
		{
			starttimer(AHOST,30);
		}
                resend_buffer[next_seqnum] = packet;
		timeout_value[next_seqnum] = get_sim_time()+30;
                next_seqnum++;

        }
        else
        {
                //struct msg buf_msg;
		//memset(&buf_msg,0,sizeof(buf_msg));
                //strcpy(buf_msg.data,message.data);
                message_buffer[buffer_length] = message;
                buffer_length++;
        }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
	int check = 0;
        check = check + packet.acknum;
        check = check + packet.seqnum;
       	window_size = getwinsize(); 
	for(int i=0;i<20;i++)
        {
                check = check + (int)packet.payload[i];

        }
	if(packet.checksum != check || (packet.acknum < base) || (packet.acknum>=base+window_size)){
		return;
	}
	timeout_value[packet.acknum] = 0;
	timeout_ack[packet.acknum] = 1;
        if(packet.acknum == base) // check base or sent_seqnum
        {	
		for(int i=base;i<next_seqnum;i++)
                {
                        if(timeout_ack[i]!= 1)
                                break;
                        base++;
                }
	}
        while((next_seqnum < base+window_size) && (buffer_index < buffer_length)) //check this
        {
               		struct pkt packet;
			memset(&packet,0,sizeof(packet));
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
			float current_time = get_sim_time();
                	tolayer3(AHOST,packet);
			timeout_value[packet.seqnum] = get_sim_time()+30; 
                	resend_buffer[packet.seqnum] = packet;
                	buffer_index++;
			next_seqnum++;
			
       }
       if(base == next_seqnum)
       {
                        stoptimer(AHOST);
       }
        
}

/* called when A's timer goes off */
void A_timerinterrupt()
{	
	for(int i=base;i<next_seqnum;i++)
	{
		if((timeout_value[i]  == get_sim_time()) && (timeout_ack[i] == 0))
		{
			tolayer3(AHOST,resend_buffer[i]);
			timeout_value[i] = get_sim_time()+30;
			break;
		}	
	}
        float next_timervalue = get_sim_time()+30;
	for(int i=base;i<next_seqnum;i++)
	{
		if((timeout_ack[i]==0) && (timeout_value[i]< next_timervalue))
		{
			next_timervalue = timeout_value[i];
		}
	}
	if(next_timervalue > 0)
	starttimer(AHOST,next_timervalue - get_sim_time());	
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
 	window_size = getwinsize();
        next_seqnum = 0;
        base = 0;
        memset(&message_buffer,0,sizeof(message_buffer)); 
        memset(&timeout_value,0,sizeof(timeout_value));
        memset(&timeout_ack,0,sizeof(timeout_ack));
        memset(&resend_buffer,0,sizeof(resend_buffer));
	buffer_index = 0;
	buffer_length = 0;
	 
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
	int sum = 0;
        struct pkt send_packet;
	memset(&send_packet,0,sizeof(send_packet));
        sum = sum + packet.seqnum;
        sum = sum + packet.acknum;
        int len = sizeof(packet.payload);
        for(int i=0;i<20;i++)
        {
                sum = sum + (int)packet.payload[i];

        }
        int ackNum;
        if(sum != packet.checksum || (packet.seqnum>=expected_packet+window_size) || (packet.seqnum<expected_packet-window_size))
	{
		return;
	}
                        send_packet.acknum = packet.seqnum;
                        int check = send_packet.acknum;
                        check = check + send_packet.seqnum;
                        for(int i=0;i<20;i++)
                        {
                                check = check + (int)send_packet.payload[i];

                        }
                        send_packet.checksum = check;
			tolayer3(BHOST,send_packet);
                        if(packet.seqnum == expected_packet)
                        {
                              	int len = expected_packet + window_size; 
				tolayer5(BHOST,packet.payload);
				expected_packet++;
                               // ackNum = packet.seqnum;
					for(int i=expected_packet;i<len;i++)
					{
						if(recv_ack[i] == 0)
							break;
						tolayer5(BHOST,recv_buffer[i].payload);
						expected_packet++;
					}
                        }
                        else if(packet.seqnum > expected_packet)
                        {	
                                recv_buffer[packet.seqnum] = packet;  
				recv_ack[packet.seqnum] = 1;
                                //ackNum = packet.seqnum;
                        }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	memset(&recv_buffer,0,sizeof(recv_buffer));
	memset(&recv_ack,0,sizeof(recv_ack));
	window_size = getwinsize();
        expected_packet = 0;
}
