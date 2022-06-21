#ifndef _MEDIA_IPC_CONTROL_BLOCK
#define _MEDIA_IPC_CONTROL_BLOCK

#include "Formats.h"
#include <stdint.h>
#include <chrono>

namespace MediaIPC {

enum class VideoBuffer : uint8_t
{
	FrontBuffer = 0,
	BackBuffer = 1
};

class ControlBlock
{
	public:
		
		//Creates a blank control block
		//(Note that the relevant video and/or audio parameters will need to be set before the control block can be used)
		ControlBlock();
		
		//Determines the number of bytes required to hold the video framebuffer, based on our video parameters
		uint64_t calculateVideoBufsize() const;

		//Determines the number of bytes required to hold the video framebuffer, based on our video parameters
		uint64_t calculateVideoFramesize() const;
		
		//Determines the number of bytes required to hold the audio sample buffer, based on our audio parameters
		uint64_t calculateAudioBufsize() const;
		
		//Determines the interval in microseconds for sampling the video framebuffer, based on our video parameters
		std::chrono::microseconds calculateVideoInterval() const;

                std::chrono::microseconds calculateVideoInterval2() const;

		//Determines the interval in microseconds for sampling the audio sample buffer, based on our audio parameters
		std::chrono::microseconds calculateAudioInterval() const;
		
		
		//---- VIDEO PARAMETERS ----
		
		//The width of the video in pixels
		uint32_t width;
		
		//The height of the video in pixels
		uint32_t height;
		
		//The number of video frames per second
		uint32_t frameRate;
		
		//The pixel format of the video
		VideoFormat videoFormat;
		
		//The maximum width 
		uint32_t maxWidth;

		//The maximum height
		uint32_t maxHeight;

		
		//---- AUDIO PARAMETERS ----
		
		//The number of audio channels
		uint32_t channels;
		
		//The number of audio samples per second
		uint32_t sampleRate;
		
		//The number of samples we want to process together in a batch
		//(A value of 1 means we don't perform any batching at all)
		uint32_t samplesPerBuffer;
		
		//The format of the audio samples
		AudioFormat audioFormat;

		//The timestamp of producer write
		uint64_t mtime;	
		
		//The timestamp of consumer read
		uint64_t atime;	
		
		//Is the producer currently producing data?
		//(Access to this flag is protected by the "status" mutex)
		//(The "status" mutex also controls the initial access to the entire control block)
		bool active;
		

		uint32_t ProducerId;

		struct ProducerState {
			bool RequestPending;
			bool ResponsePending;
			uint32_t RequestId;
			uint32_t ResponseId;
		};

		uint32_t ConsumerId;

		struct ConsumerState {
			bool RequestPending;
			bool ResponsePending;
			uint32_t ResponseId;
		};

	private:
		
		//---- CONTROL FLAGS ----
		//(These are used internally by the MediaProducer and MediaConsumer classes)
		friend class MediaConsumer;
		friend class MediaProducer;
		
		//Was the front framebuffer or the back framebuffer most recently updated?
		//(Access to this flag is protected by the "video" mutex)
		VideoBuffer lastBuffer;
		
		//The current head position of the audio ring buffer
		//(Access to this flag is protected by the "audio" mutex)
		uint32_t ringHead;
};

} //End MediaIPC

#endif
