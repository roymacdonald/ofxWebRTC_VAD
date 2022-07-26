//
//  ofxVadRecorder.cpp
//  soundManagerVADRecord
//
//  Created by Roy Macdonald on 22-08-22.
//

#include "ofxVadRecorder.h"
#include "ofxWebRTC_VAD.h"


ofxVadRecorder::ofxVadRecorder(ofxWebRTC_VAD* _vad):vad(_vad){
    bEnabled = true;
}

void ofxVadRecorder::enable(){
    bEnabled = true;
}
void ofxVadRecorder::disable(){
    bEnabled = false;
    
    std::lock_guard<std::mutex> lck(disableMutex);
    for(auto& r: currentRecordings){
        if(r){
            recordings.push_back(r);
            r = nullptr;
        }
    }
}

void ofxVadRecorder::process(ofSoundBuffer &in) {
    if(!bEnabled) return;
    
    circularBuffer.push(in);
    lastInBuffer = in;
    
}

bool ofxVadRecorder::appendLastInToCurrentRecording(size_t channelIndex){
    if(channelIndex < currentRecordings.size() && currentRecordings[channelIndex] && channelIndex < lastInBuffer.getNumChannels() ){
        
        ofSoundBuffer chan;
        lastInBuffer.getChannel(chan, channelIndex);
        if(chan.getNumChannels() == currentRecordings[channelIndex]->getNumChannels()){
            currentRecordings[channelIndex]->append(chan);
            return true;
        }
    }
    return false;
}

void ofxVadRecorder::updateRecording(size_t channelIndex, ChannelState state){
    if(!bEnabled) return;
    
    if(!vad){
        ofLogError("ofxVadRecorder::updateRecording") << "vad pointer is null. returning immediately";
    }
    
    size_t numChannels   = vad -> getNumChannels();
    size_t inBufferSize  = vad -> getInBufferSize();
    size_t inSampleRate  = vad -> getInSampleRate();
    size_t audioOutCount = vad -> getAudioOutCount();

    checkSizes(numChannels);
        

    if(state == OFX_VAD_ACTIVE){
//        if(currentRecordings[channelIndex]){
//            // copy the last buffer added to circular buffer
////            circularBuffer.copyIntoBuffer(currentRecordings[channelIndex], inBufferSize,  channelIndex, circularBuffer.getPushIndex(), true);
//
//            ofSoundBuffer chan;
//            lastInBuffer.getChannel(chan, channelIndex);
//            currentRecordings[channelIndex]->append(chan);
//
//
//        }
        appendLastInToCurrentRecording(channelIndex);
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

//        if(currentRecordings[channelIndex]){
        if(appendLastInToCurrentRecording(channelIndex)){
            // copy the last buffer added to circular buffer
//            circularBuffer.copyIntoBuffer(currentRecordings[channelIndex], inBufferSize,  channelIndex, circularBuffer.getPushIndex(), true);
            
//            ofSoundBuffer chan;
//            lastInBuffer.getChannel(chan, channelIndex);
//            currentRecordings[channelIndex]->append(chan);
//
//
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
