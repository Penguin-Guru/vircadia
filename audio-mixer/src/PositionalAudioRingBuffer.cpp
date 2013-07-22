//
//  PositionalAudioRingBuffer.cpp
//  hifi
//
//  Created by Stephen Birarda on 6/5/13.
//  Copyright (c) 2013 HighFidelity, Inc. All rights reserved.
//

#include <cstring>

#include <PacketHeaders.h>

#include "PositionalAudioRingBuffer.h"

PositionalAudioRingBuffer::PositionalAudioRingBuffer() :
    AudioRingBuffer(false),
    _position(0.0f, 0.0f, 0.0f),
    _orientation(0.0f, 0.0f, 0.0f, 0.0f),
    _willBeAddedToMix(false),
    _sourceID(-1),
    _listenMode(AudioRingBuffer::NORMAL),
    _listenRadius(0.0f),
    _listenSourceCount(0),
    _listenSources(NULL)
{
    
}

PositionalAudioRingBuffer::~PositionalAudioRingBuffer() {
    if (_listenSources) {
        delete[] _listenSources;
    }
}

int PositionalAudioRingBuffer::parseData(unsigned char* sourceBuffer, int numBytes) {
    unsigned char* currentBuffer = sourceBuffer + numBytesForPacketHeader(sourceBuffer);
    currentBuffer += parseSourceData(currentBuffer, numBytes - (currentBuffer - sourceBuffer));
    currentBuffer += parseListenModeData(currentBuffer, numBytes - (currentBuffer - sourceBuffer));
    currentBuffer += parsePositionalData(currentBuffer, numBytes - (currentBuffer - sourceBuffer));
    currentBuffer += parseAudioSamples(currentBuffer, numBytes - (currentBuffer - sourceBuffer));
    
    return currentBuffer - sourceBuffer;
}

int PositionalAudioRingBuffer::parseSourceData(unsigned char* sourceBuffer, int numBytes) {
    unsigned char* currentBuffer = sourceBuffer;
    
    memcpy(&_sourceID, currentBuffer, sizeof(_sourceID));
    currentBuffer += sizeof(_sourceID);
    
    return currentBuffer - sourceBuffer;
}

int PositionalAudioRingBuffer::parseListenModeData(unsigned char* sourceBuffer, int numBytes) {
    unsigned char* currentBuffer = sourceBuffer;

    memcpy(&_listenMode, currentBuffer, sizeof(_listenMode));
    currentBuffer += sizeof(_listenMode);
    
    if (_listenMode == AudioRingBuffer::OMNI_DIRECTIONAL_POINT) {
        memcpy(&_listenRadius, currentBuffer, sizeof(_listenRadius));
        currentBuffer += sizeof(_listenRadius);
    } else if (_listenMode == AudioRingBuffer::SELECTED_SOURCES) {
        memcpy(&_listenSourceCount, currentBuffer, sizeof(_listenSourceCount));
        currentBuffer += sizeof(_listenSourceCount);

        if (_listenSources) {
            delete[] _listenSources;
        }
        _listenSources = new int[_listenSourceCount];
        memcpy(_listenSources, currentBuffer, sizeof(int) * _listenSourceCount);
        currentBuffer += sizeof(int) * _listenSourceCount;

    }
    
    return currentBuffer - sourceBuffer;
}

int PositionalAudioRingBuffer::parsePositionalData(unsigned char* sourceBuffer, int numBytes) {
    unsigned char* currentBuffer = sourceBuffer;
    
    memcpy(&_position, currentBuffer, sizeof(_position));
    currentBuffer += sizeof(_position);
    
    memcpy(&_orientation, currentBuffer, sizeof(_orientation));
    currentBuffer += sizeof(_orientation);
    
    // if this node sent us a NaN for first float in orientation then don't consider this good audio and bail
    if (std::isnan(_orientation.x)) {
        _endOfLastWrite = _nextOutput = _buffer;
        _isStarted = false;
        return 0;
    }
    
    return currentBuffer - sourceBuffer;
}

bool PositionalAudioRingBuffer::shouldBeAddedToMix(int numJitterBufferSamples) {
    if (_endOfLastWrite) {
        if (!_isStarted && diffLastWriteNextOutput() <= BUFFER_LENGTH_SAMPLES_PER_CHANNEL + numJitterBufferSamples) {
            printf("Buffer held back\n");
            return false;
        } else if (diffLastWriteNextOutput() < BUFFER_LENGTH_SAMPLES_PER_CHANNEL) {
            printf("Buffer starved.\n");
            _isStarted = false;
            return false;
        } else {
            // good buffer, add this to the mix
            _isStarted = true;
            return true;
        }
    }
    
    return false;
}
