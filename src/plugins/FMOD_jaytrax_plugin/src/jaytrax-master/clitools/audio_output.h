#ifndef _audio_output_h_
#define _audio_output_h_

#include <AudioToolbox/AudioQueue.h>

#include <stdint.h>

class CoreAudioStream {
public:
    typedef void (*callback)(void * context, short * samples, uint32_t count);

private:
	AudioQueueRef audioQueue;
	AudioQueueBufferRef *buffers;
	uint32_t numberOfBuffers;
	uint32_t bufferByteSize;
    
    uint32_t sampleRate;
    
    callback playerCallback;
    void * playerCallbackUserData;

	static void renderOutputBuffer(void *userData, AudioQueueRef queue, AudioQueueBufferRef buffer);

public:
    
	CoreAudioStream(callback cb, void * userData, const uint32_t sampleRate);
	~CoreAudioStream();
	bool start();
	void close();
};


#endif
