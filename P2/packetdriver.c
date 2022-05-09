/*
 * AUTHOR : AHMET TURAN BULUT
 * LOGIN : aturanb
 * TITLE: CIS 415 Project 2
 * This is my own work except for help hours from Timmy & Nick, and Pseudocode that was given during lab hours.
*/

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

//Global Variables
BoundedBuffer *buf_pool;
BoundedBuffer *buf_receive [MAX_PID + 1];
BoundedBuffer *buf_send;

FreePacketDescriptorStore *fpds;
NetworkDevice *networkDevice;

//Helper Functions
static void *send();
static void *receive();

void init_packet_driver(NetworkDevice *nd, void *mem_start, unsigned long mem_length, FreePacketDescriptorStore **fpds_ptr){
	//Create Free Packet Descriptor Store using mem_start and mem_length
	fpds = FreePacketDescriptorStore_create(mem_start, mem_length);
	
	//Create any buffers required by send and receive threads 
	buf_pool = BoundedBuffer_create(MAX_PID);
	if(buf_pool == NULL) { DIAGNOSTICS("[Driver> ERROR: Failed to create Pool Buffer.\n"); }
	
	buf_send = BoundedBuffer_create(MAX_PID);
	if(buf_send == NULL) { DIAGNOSTICS("[Driver> ERROR: Failed to create Send Buffer.\n"); }
	
	for(int i = 0; i < MAX_PID + 1; i++){
		buf_receive[i] = BoundedBuffer_create(MAX_PID);
		if(buf_receive[i] == NULL) { 
			DIAGNOSTICS("[Driver> ERROR: Failed to create Receive[%d] Buffer.\n", i); 
		}
	}

	/* return the FPDS to the code that called you */
	networkDevice = nd;
	*fpds_ptr = (FreePacketDescriptorStore *) fpds;
	
	/* create any threads you require for your implementation */
	pthread_t senderT, receiverT;
	if(pthread_create(&senderT, NULL, send, NULL) != 0){
		DIAGNOSTICS("[Driver> ERROR: Failed to create Send Thread.\n");
	}
	if(pthread_create(&receiverT, NULL, receive, NULL) != 0){
		DIAGNOSTICS("[Driver> ERROR: Failed to create Receive Thread.\n");
	}
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
	buf_receive[pid]->blockingRead(buf_receive[pid], (void **) pd);
}

int nonblocking_get_packet(PacketDescriptor **pd, PID pid) {
	/* if there is currently a waiting packet for `pid’, return that packet */
	/* to the calling application and return 1 for the value of the function */
	/* otherwise, return 0 for the value of the function */
	return buf_receive[pid]->nonblockingRead(buf_receive[pid], (void **) pd);
}

//Sends a packet descriptor to network
static void *send(){
	
	PacketDescriptor *pd = NULL;
	int i;
	
	while(1){
		
		//Get Packet Descriptor from send buffer
		buf_send->blockingRead(buf_send, (void **) &pd);
		
		//Try sending the packet descriptor to Network Device
		for(i = 0; i < TRY; i++){
			if(networkDevice->sendPacket(networkDevice, pd)){
				DIAGNOSTICS("[Driver> Packet sent after %d tries.\n", i + 1);
				break;
			}
			else if(i == (TRY - 1)){
				DIAGNOSTICS("[Driver> Sending packet failed.\n");
			}
		}
		
		//Send the packet descriptor to pool, if it fails send it to FPDS
		if(!buf_pool->nonblockingWrite(buf_pool, pd)){
			DIAGNOSTICS("[Driver> Non Blocking Write to Buffer Pool failed,\n");
			DIAGNOSTICS("         Sending Packet Descriptor to Free Packet Descriptor Store.\n");
			if(!fpds->nonblockingPut(fpds, pd)){
				DIAGNOSTICS("[Driver> ERROR: Non Blocking Put to Free Packet Descriptor Store failed,\n");
			}
		}
	}
	return NULL;
}

//Receives a packet descriptor from the network
static void *receive(){

	PacketDescriptor *curr_pd = NULL;
	PacketDescriptor *filled_pd = NULL;
	PID pid;
	
	//Get the free packet descriptor from fpds
	fpds->blockingGet(fpds, &curr_pd);
	
	//Initialize the packet descriptor
	initPD(curr_pd);
	
	//Tell the network device to use the indicated PacketDescriptor for next incoming data packet
	networkDevice->registerPD(networkDevice, curr_pd);
	
	while(1){
		

		//Block the thread until there is a packet
		networkDevice->awaitIncomingPacket(networkDevice);

		//Make filled_pd point to curr_pd
		filled_pd = curr_pd;

		//If we get a Packet Descriptor from pool
		if(buf_pool->nonblockingRead(buf_pool, (void **)&curr_pd)){
			DIAGNOSTICS("Received Packet Descriptor From Pool.\n");
			
			//Initialize curr_pd
			initPD(curr_pd);

			//Tell the network device to use the indicated PacketDescriptor for next incoming data packet
			networkDevice->registerPD(networkDevice, curr_pd);
			
			//Get the PID of the application
			pid = getPID(filled_pd);
			DIAGNOSTICS("[Driver> Packet Received for PID: %d\n", pid);
			
			//Write to buf_receive of the application, if it fails send it to pool, if that fails send it to FPDS
			if(!buf_receive[pid]->nonblockingWrite(buf_receive[pid], curr_pd)){
				DIAGNOSTICS("[Driver> Non Blocking Write to Buffer Receive failed for PID: %d,\n", pid);
				DIAGNOSTICS("         Sending Packet Descriptor to Buffer Pool.\n");
				if(!buf_pool->nonblockingWrite(buf_pool, filled_pd)){
					DIAGNOSTICS("[Driver> Non Blocking Write to Buffer Pool failed for PID: %d,\n", pid);
					DIAGNOSTICS("         Sending Packet Descriptor to Free Packet Descriptor Store.\n");
					if(!fpds->nonblockingPut(fpds, filled_pd)){
						DIAGNOSTICS("[Driver> ERROR: Non Blocking Put to Free Packet Descriptor Store failed,\n");
					}
				}
			}
			
		}

		//If we don't get a Packet Descriptor from pool:
		else {
			curr_pd = filled_pd;
			//Initialize and register curr_pd for next coming packet
			initPD(curr_pd);
			networkDevice->registerPD(networkDevice, curr_pd);
		}
	}
	
	return NULL;
}