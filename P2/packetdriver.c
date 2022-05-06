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

void init_packet_driver(NetworkDevice *nd, void *mem_start, unsigned long mem_length, FreePacketDescriptorStore **fpds_ptr){
	/* create Free Packet Descriptor Store using mem_start and mem_length */
	FreePacketDescriptorStore_create(mem_start, mem_length);
	/* create any buffers required by your thread[s] */
	
	/* create any threads you require for your implementation */
	/* return the FPDS to the code that called you */

}

void blocking_send_packet(PacketDescriptor *pd) {
	/* queue up packet descriptor for sending */
	/* do not return until it has been successfully queued */
}

int nonblocking_send_packet(PacketDescriptor *pd) {
	/* if you are able to queue up packet descriptor immediately, do so and return 1 */
	/* otherwise, return 0 */
}

void blocking_get_packet(PacketDescriptor **pd, PID pid) {
	/* wait until there is a packet for `pid’ */
	/* return that packet descriptor to the calling application */
}

int nonblocking_get_packet(PacketDescriptor **pd, PID pid) {
	/* if there is currently a waiting packet for `pid’, return that packet */
	/* to the calling application and return 1 for the value of the function */
	/* otherwise, return 0 for the value of the function */
}