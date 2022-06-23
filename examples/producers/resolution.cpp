#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <thread>
#include <string>
#include <sstream>
using std::cin;
using std::cout;
using std::endl;

//When building your own producers, this will be #include <MediaIPC/MediaProducer.h>
#include "../../source/public/MediaProducer.h"
#include "../common/common.h"

//Generates a sinewave audio signal
double sinewave(uint64_t timestep, double frequency, double sampleRate) {
	return std::sin(2.0 * M_PI * (double)timestep * frequency / sampleRate);
}


struct { int x, y; } rezs[4] = {
 	{640,480},
 	{854,729},
 	{1024,768},
 	{1280,1024},
 };

int ind = 3;

int main (int argc, char* argv[])
{
	try
	{
		//If the user supplied a prefix string, use it instead of our default
		std::string prefix = ((argc > 1) ? argv[1] : "TestPrefix");
	
		MediaIPC::ControlBlock cb;

		cb.width = rezs[ind].x;
		cb.height = rezs[ind].y;
		cb.frameRate = 30;
		cb.videoFormat = MediaIPC::VideoFormat::RGBA;
		cb.channels = 2;
		cb.sampleRate = 44100;
		cb.samplesPerBuffer = 1;
		cb.audioFormat = MediaIPC::AudioFormat::PCM_F32LE;
		
		//Start producing data
		MediaIPC::MediaProducer producer(prefix, cb);
		
		//Allow the user to terminate the stream
		bool shouldExit = false;
		std::thread inputThread([&]()
		{
		    while (1 ) {
			cout << "Enter to change resolution" << endl;
			cin.ignore();
		    	ind = (ind + 1 ) % 4;
		    }
		});
		
		//Determine our sampling frequency
		auto samplingFrequency = cb.calculateVideoInterval();
		
		//Determine our starting time
		std::chrono::high_resolution_clock::time_point lastSample = std::chrono::high_resolution_clock::now();
		std::chrono::high_resolution_clock::time_point nextSample = lastSample;
		
		//Procedurally generate data until the user terminates the stream
		uint64_t frameNum = 0;
		uint64_t videoBufsize = cb.calculateVideoBufsize();
		uint64_t audioBufsize = cb.calculateAudioBufsize();
		std::unique_ptr<uint8_t[]> videoBuf( new uint8_t[videoBufsize] );
		std::unique_ptr<uint8_t[]> audioBuf( new uint8_t[audioBufsize] );
		while (shouldExit == false)
		{
			//Determine the time point for the next sampling iteration
			lastSample = nextSample;
			nextSample = lastSample + samplingFrequency;

		cb.width = rezs[ind].x;
		cb.height = rezs[ind].y;
			
			//Generate our video framebuffer
			uint64_t bpp = MediaIPC::FormatDetails::bytesPerPixel(cb.videoFormat);
			for (unsigned int y = 0; y < cb.height; ++y)
			{
				for (unsigned int x = 0; x < cb.width; ++x)
				{
					uint32_t i = (y*cb.width*bpp) + (x*bpp);
					uint8_t val = i + frameNum % 255;
					videoBuf[i] = val;
					videoBuf[i+1] = val + 50 % 255;
					videoBuf[i+2] = val - 50 % 255;
					videoBuf[i+3] = 0;
				}
			}
			
			//Generate our audio samples
			float audioSample = (float)((sinewave(frameNum, 261600.0, (double)cb.frameRate) + 1.0) / 2.0);
			for (unsigned int i = 0; i < audioBufsize / sizeof(float); ++i) {
				*((float*)(audioBuf.get()) + i) = audioSample;
			}
			
			//Submit the samples
			producer.submitVideoFrame(videoBuf.get(), cb.calculateVideoFramesize() , cb.width, cb.height);
			producer.submitAudioSamples(audioBuf.get(), audioBufsize);
			frameNum++;
			
			//Sleep until our next iteration
			std::this_thread::sleep_until(nextSample);
		}
		
		inputThread.join();
	}
	catch (std::runtime_error& e) {
		cout << "Error: " << e.what() << endl;
	}
	
	return 0;
}
