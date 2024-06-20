/*
/* Poor man's audio.device implementation.
 *
 * Copyright 2022, Juergen Wothke
 */

#include "audiodevice.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "devices/audio/audio.h"
#include "exec/libraries.h"


#include "sysconfig.h"
#include "sysdeps.h"	// FIXME the below includes need this.. UADE's include files should be fixed!
#include "audio.h"
#include "custom.h"
#include "memory.h"

extern void reply_msg_to_port(uint32_t al_msgAddr);

static int _use_audio_device = 0;

// just a simple list to keep try of pending "WRITE" commands (per audio channel)
struct MsgNode {
	struct  MsgNode *next;
	uint32_t al_msgAddr;		// little-endian Message address in amiga mem
};

struct MsgNode* _writeMsgList[4];

static struct MsgNode* getWriteMessage(int channel) {
	return _writeMsgList[channel];
}
static struct MsgNode* newMsg() {
	return calloc(1, sizeof(struct MsgNode));
}
static void deleteMsg(struct MsgNode* n) {
	free(n);
}

static struct MsgNode* getLastNode(struct MsgNode* n) {
	while(n->next) {
		n= n->next;
	}
	return n;
}

static void addWriteMessage(int channel, uint32_t al_msgAddr) {	
	struct MsgNode*n = newMsg();
	n->al_msgAddr = al_msgAddr;
	
	if (_writeMsgList[channel] == 0) {
		_writeMsgList[channel] = n;
	} else {
		getLastNode(_writeMsgList[channel])->next = n;
	}
}

static void removeWriteMessage(int channel) {
	if (_writeMsgList[channel] != 0) {
		struct MsgNode* n = _writeMsgList[channel];
		_writeMsgList[channel] = n->next;
		
		deleteMsg(n);
	}
}	

static void resetChannel(int channel) {
	while (getWriteMessage(channel)) {
		removeWriteMessage(channel);
	}
}

// utils for directly accessing amiga's mem: seems Amiga uses big-endian
uint16_t wordSwapEndian(uint16_t in) {
	return (in >> 8) | (in << 8);
}
uint32_t intSwapEndian(uint32_t in) {
	return((in>>24)&0xff) |
			((in<<8)&0xff0000) |
			((in>>8)&0xff00) |
			((in<<24)&0xff000000);
}

static const char* cmd_labels[] = {
	"CMD_INVALID", 		// 0
	"CMD_RESET", 		// 1
	"CMD_READ",			// 2
	"CMD_WRITE",		// 3
	"CMD_UPDATE", 		// 4
	"CMD_CLEAR",		// 5
	"CMD_STOP",			// 6
	"CMD_START",		// 7
	"CMD_FLUSH",		// 8		
	"ADCMD_FREE",		// 9
	"ADCMD_SETPREC",	// 10 
	"ADCMD_FINISH",		// 11
	"ADCMD_PERVOL",		// 12
	"ADCMD_LOCK",		// 13
	"ADCMD_WAITCYCLE",	// 14
	"","","","","","","","","","","","","","","","","",
	"ADCMD_ALLOCATE",	// 32		
};
static const char* get_cmd_label(struct IOAudio *req) {	
	uint16_t cmd = wordSwapEndian(req->ioa_Request.io_Command);
	return cmd <= 32 ? cmd_labels[cmd] : "invalid";
}

static int8_t getTargetChannel(uint32_t unit) {
	unit &= 0xf; // only bottom bits are relevant
	for (int i= 0; i<4; i++) {
		if (unit & 0x1) {
			return i;
		}
		unit >>= 1;
	}
	return -1;
}

void audiodevice_DMA_signal(int audioChannel) {
	if (!_use_audio_device) return;
	
	// DMA playback has reached the end of the sample buffer
	
	struct MsgNode* msgNode = getWriteMessage(audioChannel);			
	if (msgNode) {

		struct IOAudio *req = (struct IOAudio*) get_real_address(intSwapEndian(msgNode->al_msgAddr));
		uint16_t   ioa_Cycles = wordSwapEndian(req->ioa_Cycles);	// 0  thru 65535, 0 for infinite

		if (ioa_Cycles == 1) {
			// write command completed
			disable_audio_dma(audioChannel);			// stop automatic looping
			
			reply_msg_to_port(msgNode->al_msgAddr);	// return them to their MessagePort		
			removeWriteMessage(audioChannel);
		} else if (ioa_Cycles == 0) {
			// infinite looping - must be cancelled via CMD_FLUSH to stop
		} else {
			ioa_Cycles -= 1;
			req->ioa_Cycles = wordSwapEndian(ioa_Cycles);	// this might be a problem if player does not reset before reuse
		}

	} else {
		fprintf(stderr, "error: audio.device no playing channel found!"); 
	}
}


static void allocWriteLists() {
	resetChannel(0);
	resetChannel(1);
	resetChannel(2);
	resetChannel(3);
}

uint8_t _allocatedChannels = 0;

void audiodevice_reset() {
	allocWriteLists();
	_use_audio_device= _allocatedChannels= 0;

}

uint8_t allocChannels(uint8_t *prefs, int prefsLen) {
	// flawed impl: Request's prio (req->ioa_Request.mn_Node.ln_Pri) allows to potentially 
	// steal channels -> but that would be rather braindead in a standalone song scenario...
	for (int i= 0; i<prefsLen; i++) {
		uint8_t chnMask= prefs[i] & 0xf;	// only lower four bits are considered for channel alloc
		
		if (chnMask && ((_allocatedChannels & chnMask) == 0)) {
			_allocatedChannels |= chnMask;
			return chnMask;			
		}
	}
	return 0; // alloc failed
}


// "al_" prefix signifies that the var contains an original amiga memory 
// address in little endian byte order
void audiodevice_open(uint32_t al_msgAddr) {
	struct IOAudio *req = (struct IOAudio*) get_real_address(intSwapEndian(al_msgAddr));
	struct Library *lib = (struct Library*)get_real_address(intSwapEndian(req->ioa_Request.io_Device));
	
	// unclear which additional fields might need to be initialized
	lib->lib_OpenCnt = wordSwapEndian(1);	// just mark as "in use".. could track open/close if needed
	lib->lib_Version = wordSwapEndian(39);	// MaxTrax uses this to act differently depending on OS version
	lib->lib_NegSize = wordSwapEndian(36);	// see audio.device's function vectors
	
	
	uint16_t cmd = wordSwapEndian(req->ioa_Request.io_Command);
	
	// only bottom 4-bits are considered for channel allocation!
	uint16_t ioa_AllocKey = wordSwapEndian(req->ioa_AllocKey);
	
		// channel combination array, e.g. [0xf, 0xf]
		// note: seems this garbage API later reuses the same ioa_Data field to 
		// pass the audio buffer.. fucking morons..
	uint8_t* ioa_Data = (uint8_t*)get_real_address(intSwapEndian(req->ioa_Data));
	uint32_t ioa_Length = intSwapEndian(req->ioa_Length);
		
//	req->io_Error = IOERR_OPENFAIL; 	// if ioa_AllocKey and io_Device could not be set
	
	if ((ioa_AllocKey == 0 || (cmd == ADCMD_ALLOCATE)) && ioa_Length > 0) {
		// implicit allocation
		uint8_t allo= allocChannels(ioa_Data, ioa_Length);
//		fprintf(stderr, "uade: audio.device Open implicit ADCMD_ALLOCATE %d\n", allo);	

		// testcase: MaxTrax, e.g. 1 x 15 (i.e. all 4 channel bits set) means the 
		// app wants all 4 channels.. 
		// testcase: "G&T Game Systems".. song performs 3 open calls - using ioa_AllocKey=0
					
		uint32_t al_portAddr = req->ioa_Request.io_Message.mn_ReplyPort;
		if (al_portAddr) {
			struct  MsgPort *replyPort = (struct  MsgPort*) get_real_address(intSwapEndian(al_portAddr));
			// http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node04B8.html says:
			// "To allocate channels, OpenDevice also requires a properly initialized
			// reply port (mn_ReplyPort) with an allocated signal bit."
			
			// however: this doc seems to be incorrect: testcase: "Music-X Driver" which comes here
			// without ever having called AllocSignal before.. QED
			//uint32_t allocatedSignals = 0;	// fixme: not in sync with score.s impl
			
			if ((replyPort->mp_SigBit > 31)/* || ((1<<replyPort->mp_SigBit) & allocatedSignals) == 0*/) {
				fprintf(stderr, "uade: audio.device error: Open without allocated signal bit\n");
				allo = 0;
			}
		} else {
//			fprintf(stderr, "uade: audio.device error: Open without reply message port\n", allo);	
		}		
		
		if (allo) {
			req->ioa_Request.io_Unit = intSwapEndian(allo);
					
			// FIXME: only reset when 0?
			req->ioa_AllocKey = 0x7777;	// hack: unique key
			req->ioa_Request.io_Error = 0;
		} else {
			req->ioa_Request.io_Unit= 0;
			req->ioa_Request.io_Error = ADIOERR_ALLOCFAILED; 	// if requested channel combo no available 
		}
					
	} else {
		fprintf(stderr,"uade: audio.device Open\n");
		req->ioa_Request.io_Error = 0;
	}

	_use_audio_device= 1;
	audio_use_text_scope(); // see use of TEXT_SCOPE in AUDxXXX calls..	
}

static void startWriteCommand(int8_t targetChnl, uint32_t al_msgAddr) {
	struct IOAudio* req = (struct IOAudio*) get_real_address(intSwapEndian(al_msgAddr));
				
	req->ioa_Request.io_Flags = req->ioa_Request.io_Flags & ~IOF_QUICK;	// only clear if no error
	
	if (targetChnl == -1) {
		fprintf(stderr, "uade: audio.device WRITE - error: no target channel\n");
		return;
	}
	
//	fprintf(stderr, "	uade: write chn: 0x%x buf: %x len: %x cycles: %x\n", targetChnl, intSwapEndian(req->ioa_Data), intSwapEndian(req->ioa_Length), wordSwapEndian(req->ioa_Cycles)  );			

	uint8_t flags = req->ioa_Request.io_Flags;
	
	if (flags & ADIOF_PERVOL) {
		uint16_t	ioa_Period=  wordSwapEndian(req->ioa_Period);; //  >= 124!
		uint16_t	ioa_Volume=  wordSwapEndian(req->ioa_Volume);; //  0 thru 64, linear
		
//		fprintf(stderr, "	uade: audio.device WRITE - period %x volume %x\n", ioa_Period, ioa_Volume);	
		AUDxPER(targetChnl, ioa_Period);
		AUDxVOL(targetChnl, ioa_Volume);
	}
	
	
	int8_t* ioa_Data = (uint8_t *)get_real_address(intSwapEndian(req->ioa_Data));
	int8_t* ioa_Data_amiga = (uint8_t *)(intSwapEndian(req->ioa_Data));
	
	int8_t* data = ioa_Data_amiga;	// seems the below calls need the original amiga address for their chipmem logic
	
		
	// UADE's semantics of these regs seem to be different from the original!
	AUDxLCL(targetChnl, (uint32_t)data & 0xffff);	// 16-bit instead of 15
	AUDxLCH(targetChnl, (uint32_t)data >> 16);		// 16-bit instead of 5
	
	uint32_t ioa_Length = intSwapEndian(req->ioa_Length);	// 2 thru 131072 must be even number!!!
	AUDxLEN(targetChnl, ioa_Length >> 1);	// in words? or did UADE also change these semantics?


	// all the DMA setup should be ready...try to turn on DMA for the target audio channel..
//	audio_channel[targetChnl].state = 0; 
	DMACON(0x8200 | (1<<targetChnl));
	
	update_audio();	// needed?

	if (flags & ADIOF_WRITEMESSAGE) {
		// means that separate ioa_WriteMsg should be used to signal when command starts processing
		fprintf(stderr, "uade: audio.device WRITE - use of ioa_writeMsg not implemented!");
	}
	
		
//	fprintf(stderr, "	uade: audio.device WRITE len: %d addr: 0x%x cycles: %d\n", ioa_Length, (long)ioa_Data, ioa_Cycles);

							
	// another code example uses Wait(port->mp_SigBit) and GetMsg(port)==0 to "poll" for the "Write" to complete..
	// => not implemented yet (find a testcase 1st)
	
	req->ioa_Request.io_Error = 0;			
}

static void addWriteCommand(int8_t targetChnl, uint32_t al_msgAddr) {
	struct MsgNode* msgNode = getWriteMessage(targetChnl);
	
	if (msgNode != 0) {
		addWriteMessage(targetChnl, al_msgAddr); // just add to the end of the list
	} else {
		addWriteMessage(targetChnl, al_msgAddr);
		startWriteCommand(targetChnl, al_msgAddr);		
	}	
}

static void handleFlushCommand(uint32_t al_msgAddr, struct IOAudio *req) {
	// synchronous; multiple channels; cancels all pending I/O
	// see http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_3._guide/node006B.html
	
	uint8_t channelMap = intSwapEndian((uint32_t)req->ioa_Request.io_Unit) & 0xf;	// channel bit map
	
	for (int i= 0; i<4; i++) {
		if(channelMap & 0x1) {
			struct MsgNode* msgNode = getWriteMessage(i);			
			if (msgNode) {
				disable_audio_dma(i);
			}
			while (msgNode = getWriteMessage(i)) {				
				reply_msg_to_port(msgNode->al_msgAddr);	// return them to their MessagePort		
				removeWriteMessage(i);
			}
		}
		channelMap >>= 1;
	}

	uint8_t flags = req->ioa_Request.io_Flags;

	if (!(flags & IOF_QUICK)) {
		// send message to mn_ReplyPort (testcase: "G&T Game Systems" player)		
		reply_msg_to_port(al_msgAddr);
// 		fprintf(stderr, "uade: audio.device CMD_FLUSH quick: %d not implemented\n", !(flags & IOF_QUICK));
	}	
}

static void handlePERVOLCommand(int8_t targetChnl, struct IOAudio *req) {
	// see http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node04AA.html

	uint8_t flags = req->ioa_Request.io_Flags;

	if (!(flags & IOF_QUICK)) {
		// send message to mn_ReplyPort
		fprintf(stderr, "uade: audio.device ADCMD_PERVOL - IOF_QUICK mode not implemented!\n");
	}
	
	if (flags & ADIOF_SYNCCYCLE) {
		// make change at end of cycle
		fprintf(stderr, "uade: audio.device ADCMD_PERVOL - ADIOF_SYNCCYCLE mode not implemented!\n");
	}
	
	uint16_t ioa_Period = wordSwapEndian(req->ioa_Period);; //  >= 124!
	uint16_t ioa_Volume = wordSwapEndian(req->ioa_Volume);; //  0 thru 64, linear
	
//		fprintf(stderr, "	uade: audio.device ADCMD_PERVOL - period %x volume %x\n", ioa_Period, ioa_Volume);	
	AUDxPER(targetChnl, ioa_Period);
	AUDxVOL(targetChnl, ioa_Volume);
	
//	fprintf(stderr, "uade: audio.device ADCMD_PERVOL quick: %d sync: %d period: %d vol: %d\n", !(flags & IOF_QUICK), (flags & ADIOF_SYNCCYCLE), ioa_Period, ioa_Volume);
}


void audiodevice_abortIO(uint32_t al_msgAddr) {
	struct IOAudio* req = (struct IOAudio*) get_real_address(intSwapEndian(al_msgAddr));
	
	uint32_t unit = intSwapEndian((uint32_t)req->ioa_Request.io_Unit);	// channel bit map
	int8_t targetChnl= getTargetChannel(unit);	// if more than 1 channel is selected get the "LOWEST" channel used!
	
	uint16_t cmd = wordSwapEndian(req->ioa_Request.io_Command);
	
	switch (cmd) {
		case CMD_WRITE:
//			fprintf(stderr, "uade: audio.device might try to AbortIO for channel %d\n", targetChnl);
			break;
		default:
//			fprintf(stderr, "uade: audio.device ignoring attempt to AbortIO for %s\n", cmd_labels[cmd]);
			break;
	}
}

void audiodevice_beginIO(uint32_t al_msgAddr) {
	struct IOAudio* req = (struct IOAudio*) get_real_address(intSwapEndian(al_msgAddr));
	
	uint32_t unit = intSwapEndian((uint32_t)req->ioa_Request.io_Unit);	// channel bit map
	int8_t targetChnl= getTargetChannel(unit);	// if more than 1 channel is selected get the "LOWEST" channel used!
	
	uint16_t cmd = wordSwapEndian(req->ioa_Request.io_Command);
	
	switch (cmd) {
		case CMD_INVALID:	 	// 0
		case CMD_RESET: 		// 1 synchronous; only replies if flag (IOF_QUICK) is clear
		case CMD_READ: 			// 2 single channel; synchronous; only replies if flag (IOF_QUICK) is clear
		case CMD_UPDATE: 		// 4 synchronous; only replies if flag (IOF_QUICK) is clear
		case CMD_CLEAR:			// 5 synchronous; only replies if flag (IOF_QUICK) is clear
		case CMD_STOP:			// 6 synchronous; only replies if flag (IOF_QUICK) is clear
		case CMD_START:			// 7 synchronous; only replies if flag (IOF_QUICK) is clear
		case ADCMD_FREE:		// 9
		case ADCMD_SETPREC:		// 10 
		case ADCMD_FINISH:		// 11
		case ADCMD_LOCK:		// 13
		case ADCMD_WAITCYCLE:	// 14
		case ADCMD_ALLOCATE:	// 32
			fprintf(stderr, "audio.device IO cmd %s not implemented\n", cmd_labels[cmd]);
			break;
			
		case CMD_WRITE:			// 3 queues up requests; asynchronous; only replies if flag (IOF_QUICK) is clear			
			addWriteCommand(targetChnl, al_msgAddr);	// WRITE is a single channel command
			break;
			
		case CMD_FLUSH:			// 8 synchronous; only replies if flag (IOF_QUICK) is clear
			handleFlushCommand(al_msgAddr, req);
			break;

		case ADCMD_PERVOL:		// 12; testcase MaxTrax
			handlePERVOLCommand(targetChnl, req);
			break;

		default: 
			fprintf(stderr, "uade: audio.device cmd %s not implemented!\n", get_cmd_label(cmd));
	}
}

