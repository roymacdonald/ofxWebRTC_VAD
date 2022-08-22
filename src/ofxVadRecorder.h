//
//  ofxVadRecorder.hpp
//  soundManagerVADRecord
//
//  Created by Roy Macdonald on 22-08-22.
//

#pragma once

#include "ofMain.h"
#include "ofxSoundObjects.h"
#include "VadConstants.h"


class ofxWebRTC_VAD;

//class RecordingEventArgs{
//public:
//    RecordingEventArgs(){}
//    RecordingEventArgs(shared_ptr<ofSoundBuffer> _buffer, size_t _startFrame, size_t _endFrame, size_t _channel):
//    buffer(_buffer),startFrame(_startFrame), endFrame(_endFrame), channel(_channel)
//    {
//        isFinished = (endFrame>0);
//    }
//    shared_ptr<ofSoundBuffer> buffer;
//    size_t startFrame, endFrame;
//    size_t channel;
//    bool isFinished = false;
//};

class ofxVadRecorder {
public:

    ofxVadRecorder(ofxWebRTC_VAD* vad);
    virtual ~ofxVadRecorder() {}
    
    void process(ofSoundBuffer &in) ;
    
    
    ofParameter<int> preRec = {"Pre Recording", 20, 0, 100};
    
    void updateRecording(size_t channelIndex, ChannelState state);
    
    vector<shared_ptr<ofSoundBuffer>> recordings;
    
    

//    ofEvent<RecordingEventArgs> newRecordingEvent;
protected:
    
    vector<size_t> lastRecEndBuffer;
    vector<size_t> currentRecStartFrame;
    void pushCurrentRecording(size_t channelIndex);
    
private:
    vector<shared_ptr<ofSoundBuffer> > currentRecordings;
    
    ofxCircularSoundBuffer circularBuffer;
    
    ofEventListeners listeners;
        
    std::mutex recordingsMutex;
    
    
    ofxWebRTC_VAD* vad = nullptr;
    
    
    void checkSizes(size_t );

    
    ofSoundBuffer lastInBuffer;
    
};
