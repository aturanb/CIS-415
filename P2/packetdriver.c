#include <pthread.h>
#include "BoundedBuffer.h"
#include "destination.h"
#include "diagnostics.h"
#include "fakeapplications.h"
#include "freepacketdescriptorstore__full.h"
#include "freepacketdescriptorstore.h"
#include "networkdevice__full.h"
#include "networkdevice.h"
#include "packetdescriptorcreator.h"
#include "packetdescriptor.h"
#include "pid.h"
#include "queue.h"

#include "packetdriver.h"

#define TRY 15

BoundedBuffer *buf_pool;
BoundedBuffer *buf_receive [MAX_PID + 1];
BoundedBuffer *buf_send;

FreePacketDescriptorStore *fpds;
NetworkDevice *networkDevice;

static void *send();
static void *receive();

void init_packet_driver(NetworkDevice *nd, void *mem_start, unsigned long mem_length, FreePacketDescriptorStore **fpds_ptr){
	/* create Free Packet Descriptor Store using mem_start and mem_length */
	*fpds_ptr = FreePacketDescriptorStore_create(mem_start, mem_length);
	
	/* create any buffers required by your thread[s] */
	buf_pool = BoundedBuffer_create(MAX_PID);
	buf_send = BoundedBuffer_create(MAX_PID);
	for(int i = 0; i < MAX_PID + 1; i++){
		buf_receive[i] = BoundedBuffer_create(MAX_PID);
	}

	/* return the FPDS to the code that called you */
	networkDevice = nd;
	fpds = (FreePacketDescriptorStore *) fpds_ptr;
	
	/* create any threads you require for your implementation */
	pthread_t senderT, receiverT;
	pthread_create(&senderT, NULL, send, NULL);
	pthread_create(&receiverT, NULL, receive, NULL);




	
}

void blocking_send_packet(PacketDescriptor *pd) {
	/* queue up packet descriptor for sending */
	//blockingWrite(buf_send, pd);
	buf_send->blockingWrite(buf_send, pd);
	/* do not return until it has been successfully queued */
}

int nonblocking_send_packet(PacketDescriptor *pd) {
	/* if you are able to queue up packet descriptor immediately, do so and return 1 */
	/* otherwise, return 0 */
	return buf_send->nonblockingWrite(buf_send, pd);
}

void blocking_get_packet(PacketDescriptor **pd, PID pid) {
	/* wait until there is a packet for `pid’ */
	/* return that packet descriptor to the calling application */
	buf_receive[pid]->blockingRead(buf_receive[pid], &pd);
}

int nonblocking_get_packet(PacketDescriptor **pd, PID pid) {
	/* if there is currently a waiting packet for `pid’, return that packet */
	/* to the calling application and return 1 for the value of the function */
	/* otherwise, return 0 for the value of the function */
	return buf_receive[pid]->nonblockingRead(buf_receive[pid], (void **)&pd);
}

//Sends a packet descriptor to network
static void *send(){
	PacketDescriptor *pd = NULL;
	int i;
	while(1){
		
		//Get Packet Descriptor from send buffer
		buf_send->blockingRead(buf_send, &pd);
		
		//try sending the packet descriptor to Network Device
		for(i = 0; i < TRY; i++){
			if(networkDevice->sendPacket(networkDevice, pd)){
				DIAGNOSTICS("PACKETDRIVER: Packet Sent to Network\n");
				break;
			}
			else if(i == (TRY - 1)){
				DIAGNOSTICS("Packet Send Failed\n");
			}



		}
		
		//Send it to pool 
		if(!buf_pool->nonblockingWrite(buf_pool, pd)){
			DIAGNOSTICS("Non Locking Put\n");
			fpds->nonBlockingPut(fpds, pd);
		}
	}
	return NULL;

}

//Receives a packet descriptor from the network
static void *receive(){
	PacketDescriptor *pd = NULL;
	PID pid;

	//Get the free packet descriptor from fpds
	blockingGet(fpds, &pd);
	
	//Init the pd
	initPD(pd);
	
	//Tell the network device to use the indicated PacketDescriptor for next incoming data packet
	registerPD(networkDevice, pd);

	while(1){
		//Block the thread until pd has been filled with an incoming data packet
		awaitIncomingPacket(networkDevice);

		//Get the PID of the application
		pid = getPID(pd);

		DIAGNOSTICS("PACKETDRIVER: Packet Received for PID: %d\n", pid);

		//Write to buf_receive[pid];
		blockingWrite(buf_receive[pid], pd);

		
		//Reset the PD with initPD
		initPD(pd);
		
		//Return back to the store or pool???
		blockingPut(fpds, pd);


	}
	return NULL;
	
}