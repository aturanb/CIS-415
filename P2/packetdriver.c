#include <pthread.h>
#include "BoundedBufer.h"
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

BoundedBufer *buf_pool;
BoundedBufer *buf_receive [MAX_PID + 1];
BoundedBufer *buf_send;

FreePacketDescriptorStore *fpd_store;
NetworkDevice *networkDevice;

void init_packet_driver(NetworkDevice *nd, void *mem_start, unsigned long mem_length, FreePacketDescriptorStore **fpds_ptr){
	/* create Free Packet Descriptor Store using mem_start and mem_length */
	*fpds = FreePacketDescriptorStore_create(mem_start, mem_length);
	//FIXME; 
	
	/* create any buffers required by your thread[s] */
	buf_pool = BoundedBuffer_create(MAX_PID);
	buf_send = BoundedBuffer_create(MAX_PID);
	for(int i = 0; i < MAX_PID + 1; i++){
		buf_receive[i] = BoundedBuffer_create(MAX_PID);
	}
	
	/* create any threads you require for your implementation */
	pthread_t senderT, receiverT;
	pthread_create(&senderT, NULL, send, NULL);
	pthread_create(&receiverT, NULL, send, NULL);

	/* return the FPDS to the code that called you */

}

void blocking_send_packet(PacketDescriptor *pd) {
	/* queue up packet descriptor for sending */
	blockingWrite(buf_send, pd);
	/* do not return until it has been successfully queued */
}

int nonblocking_send_packet(PacketDescriptor *pd) {
	/* if you are able to queue up packet descriptor immediately, do so and return 1 */
	/* otherwise, return 0 */
	return nonblockingWrite(buf_send, pd);
}

void blocking_get_packet(PacketDescriptor **pd, PID pid) {
	/* wait until there is a packet for `pid’ */
	/* return that packet descriptor to the calling application */
	*pd = (PacketDescriptor *)blockingRead(buf_receive[pid]);
}

int nonblocking_get_packet(PacketDescriptor **pd, PID pid) {
	/* if there is currently a waiting packet for `pid’, return that packet */
	/* to the calling application and return 1 for the value of the function */
	/* otherwise, return 0 for the value of the function */
	return nonblockingRead(buf_receive[pid], (void **)&pd);
}

static void *send(){
	PacketDescriptor *pd = NULL;
	while(1){
		// Get PD
		//	info
		//try bufferadd(PD)
			//pool->getnew PD
			//Fill Info
			//Network->add(newPD)
			//pool->add(newPD)
		

	}

}