//
//  ofxVadRecorder.cpp
//  soundManagerVADRecord
//
//  Created by Roy Macdonald on 22-08-22.
//

#include "ofxVadRecorder.h"
#include "ofxWebRTC_VAD.h"


ofxVadRecorder::ofxVadRecorder(ofxWebRTC_VAD* _vad):vad(_vad){
}


void ofxVadRecorder::process(ofSoundBuffer &in) {
    
    circularBuffer.push(in);
    lastInBuffer = in;
    
}


//
void ofxVadRecorder::updateRecording(size_t channelIndex, ChannelState state){
    
    if(!vad){
        ofLogError("ofxVadRecorder::updateRecording") << "vad pointer is null. returning immediately";
    }
    
    size_t numChannels   = vad -> getNumChannels();
    size_t inBufferSize  = vad -> getInBufferSize();
    size_t inSampleRate  = vad -> getInSampleRate();
    size_t audioOutCount = vad -> getAudioOutCount();

    checkSizes(numChannels);
        

    if(state == OFX_VAD_ACTIVE){
        if(currentRecordings[channelIndex]){
            // copy the last buffer added to circular buffer
//            circularBuffer.copyIntoBuffer(currentRecordings[channelIndex], inBufferSize,  channelIndex, circularBuffer.getPushIndex(), true);
            
            ofSoundBuffer chan;
            lastInBuffer.getChannel(chan, channelIndex);
            currentRecordings[channelIndex]->append(chan);
            
            
        }
    }
    else if(state == OFX_VAD_CHANGE_TO_ACTIVE){
        int totalCopy =  (vad ->attack.get() + preRec.get() + 1); // in number of buffers
        bool bAppend = false;
        if(currentRecordings[channelIndex]){
            bAppend = true;
            auto n = audioOutCount - lastRecEndBuffer[channelIndex];
            if( n < totalCopy ){
                totalCopy = n ;
            }else {
                ofLogError("ofxWebRTC_VAD::updateRecording") << "this should not be happening";
                return;
            }
        }else{

//            totalCopy =
            currentRecordings[channelIndex] = make_shared<ofSoundBuffer>();
            currentRecordings[channelIndex]->allocate(inBufferSize* totalCopy, 1);
            currentRecordings[channelIndex]->setSampleRate(inSampleRate);
            currentRecStartFrame[channelIndex] =audioOutCount;
            
            
            
        }

        int pushIndex = circularBuffer.getPushIndex() ;
        int startCopy = pushIndex - ((totalCopy -1)* inBufferSize*circularBuffer.getNumChannels());

        if(startCopy < 0){
            startCopy = circularBuffer.getBuffer().size() + startCopy;
        }

        circularBuffer.copyIntoBuffer( currentRecordings[channelIndex], inBufferSize* totalCopy,  channelIndex, startCopy, bAppend);

    }
    else if(state == OFX_VAD_CHANGE_TO_INACTIVE){

        if(currentRecordings[channelIndex]){
            // copy the last buffer added to circular buffer
//            circularBuffer.copyIntoBuffer(currentRecordings[channelIndex], inBufferSize,  channelIndex, circularBuffer.getPushIndex(), true);
            
            ofSoundBuffer chan;
            lastInBuffer.getChannel(chan, channelIndex);
            currentRecordings[channelIndex]->append(chan);
            
            
            lastRecEndBuffer[channelIndex] = audioOutCount;

            if(preRec.get() == 0){
                pushCurrentRecording(channelIndex);
            }
        }
    }
    if(state == OFX_VAD_INACTIVE){
        if(currentRecordings[channelIndex]){
            if(audioOutCount - lastRecEndBuffer[channelIndex] >  (vad ->attack.get() + preRec.get() + 1)){
                pushCurrentRecording(channelIndex);
            }
        }
    }


}


void ofxVadRecorder::pushCurrentRecording(size_t channelIndex){
//    std::scoped_lock lock(recordingsMutex);
    std::lock_guard<std::mutex> lck(recordingsMutex);
    recordings.push_back(currentRecordings[channelIndex]);

    currentRecordings[channelIndex] = nullptr;
//    RecordingEventArgs args(recordings.back(), currentRecStartFrame[channelIndex], lastRecEndBuffer[channelIndex], channelIndex);
//    newRecordingEvent.notify(this, args);

}

void ofxVadRecorder::checkSizes(size_t sizes){

    if(currentRecordings.size() != sizes){
        currentRecordings.resize(sizes, nullptr);
        lastRecEndBuffer.resize(sizes, 0);
        currentRecStartFrame.resize(sizes, 0);
        ofLogNotice("ofxVadRecorder::updateRecording") << "Recordings size was updated: " << currentRecordings.size();
    }
    
    
}
