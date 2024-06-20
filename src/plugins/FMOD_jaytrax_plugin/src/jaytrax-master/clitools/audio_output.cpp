#include "audio_output.h"

static const uint32_t DEFAULT_CHUNK_MS = 60;

CoreAudioStream::CoreAudioStream(callback useCb, void * userData, const uint32_t useSampleRate) :
	playerCallback(useCb), playerCallbackUserData(userData),
    sampleRate(useSampleRate), audioQueue(NULL)
{
	const uint32_t bufferSize = 2048;
    const uint32_t audioLatencyFrames = sampleRate * DEFAULT_CHUNK_MS / 1000;
	bufferByteSize = bufferSize << 2;
	// Number of buffers should be ceil(audioLatencyFrames / bufferSize)
	numberOfBuffers = (audioLatencyFrames + bufferSize - 1) / bufferSize;
	buffers = new AudioQueueBufferRef[numberOfBuffers];
}

CoreAudioStream::~CoreAudioStream() {
	close();
	delete[] buffers;
}

void CoreAudioStream::renderOutputBuffer(void *userData, AudioQueueRef queue, AudioQueueBufferRef buffer) {
	CoreAudioStream *stream = (CoreAudioStream *)userData;
	if (queue == NULL) {
		// Priming the buffers, skip timestamp handling
		queue = stream->audioQueue;
	}
    
	uint frameCount = buffer->mAudioDataByteSize >> 2;
    stream->playerCallback(stream->playerCallbackUserData, (short*)buffer->mAudioData, frameCount);

	AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
}

bool CoreAudioStream::start() {
	if (audioQueue != NULL) {
		return true;
	}

	AudioStreamBasicDescription dataFormat = {sampleRate, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian, 4, 1, 4, 2, 16, 0};
	OSStatus res = AudioQueueNewOutput(&dataFormat, renderOutputBuffer, this, NULL, NULL, 0, &audioQueue);
	if (res || audioQueue == NULL) {
		return false;
	}

	for (uint i = 0; i < numberOfBuffers; i++) {
		res = AudioQueueAllocateBuffer(audioQueue, bufferByteSize, buffers + i);
		if (res || buffers[i] == NULL) {
			res = AudioQueueDispose(audioQueue, true);
			audioQueue = NULL;
			return false;
		}
		buffers[i]->mAudioDataByteSize = bufferByteSize;
		// Prime the buffer allocated
		renderOutputBuffer(this, NULL, buffers[i]);
	}

	res = AudioQueueStart(audioQueue, NULL);
	if (res) {
		res = AudioQueueDispose(audioQueue, true);
		audioQueue = NULL;
		return false;
	}

	return true;
}

void CoreAudioStream::close() {
	if (audioQueue == NULL) return;
	OSStatus res = AudioQueueDispose(audioQueue, true);
	audioQueue = NULL;
}
