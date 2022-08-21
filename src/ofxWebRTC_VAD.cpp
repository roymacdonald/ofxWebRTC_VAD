
#include "ofxWebRTC_VAD.h"

template<typename T>
void downSample(const ofSoundBuffer &in, vector<T> & out){
    out.resize(in.size());
    
    auto mn = std::numeric_limits<T>::min();
    auto mx = std::numeric_limits<T>::max();
    
    for(size_t i = 0; i < in.size(); i++){
        if(in[i] > 0){
            out[i] = mx * in[i];
        }else{
            out[i] = mn * in[i];
        }
    }
}


ofxWebRTC_VAD::ofxWebRTC_VAD():ofxSoundObject(OFX_SOUND_OBJECT_PROCESSOR){
    setName("ofxWebRTC_VAD");
    bResetVads = false;
    sampleRate = 16000;
    aggressiveness = Vad::kVadNormal;
 
    listeners.push(vadAggressiveness.newListener([&](int& a){
        setAggressiveness((Vad::Aggressiveness)a);
    }));
 
    setAggressiveness(Vad::kVadVeryAggressive);
    
}
ofxWebRTC_VAD::~ofxWebRTC_VAD(){
    
}


void ofxWebRTC_VAD::setProcessSamplerate(int sr){
    // only valid values are 8000, 16000, 32000, 48000
    if(sr == 8000 || sr ==  16000 || sr ==  32000 || sr ==  48000){
        sampleRate = sr;
    }else{
        ofLogError("ofxWebRTC_VAD::setProcessSamplerate") << "Can not set sample rate to " << sr << ". Only acceptable values are 8000, 16000, 32000 and 48000";
    }
}

int ofxWebRTC_VAD::getProcessSamplerate(){
    return sampleRate.load();
}



void ofxWebRTC_VAD::process(ofSoundBuffer &in, ofSoundBuffer &out) {
    
    audioOutCount ++;
    
    if(inBufferSize != in.getNumFrames()){
        if(inBufferSize != 0){
            ofLogWarning("ofxWebRTC_VAD::process") << "Input buffer size changed. This could mess up some stuff with teh circular buffer. old size: " << inBufferSize << " new size: " << in.getNumFrames();
        }
        inBufferSize = in.getNumFrames();
        
    }
    if(inSampleRate != in.getSampleRate()){
        if(inSampleRate != 0){
            ofLogWarning("ofxWebRTC_VAD::process") << "Input samplerate changed. This could mess up some stuff with teh circular buffer. old: " << inSampleRate << " new: " << in.getSampleRate();
        }
        inSampleRate = in.getSampleRate();
    }
    
    out = in;
    circularBuffer.push(in);
    if(bResetVads.load()){
        bResetVads = false;
        vads.clear();
    }
    
    if(vads.size() != in.getNumChannels()){
        vads.clear();
        for(size_t i = 0; i < in.getNumChannels(); i++){
            vads.emplace_back(CreateVad(getAggressiveness()));
        }
    }
        
    vector<int16_t> intInput;
    bool bChangeSampleRate = sampleRate != in.getSampleRate();
    ofSoundBuffer inChannel;
    
    if(score.channelsScore.size() != in.getNumChannels()){
        score.channelsScore.resize(in.getNumChannels());
    }
    
    int chunkSize = sampleRate/ 1000 * 10; // the vad algorith requires to process chunks of either 10, 20 or 30 milliseconds.
    /// TODO: Automate the finding the correct value of chunkSize, because it is invalid for some configurations.
    for(size_t i =0; i < in.getNumChannels(); i++){
        if(vads[i]){
            in.getChannel(inChannel, i);
            if(bChangeSampleRate){
                ofSoundBuffer tempBuff;
                srConverter.changeSampleRate(inChannel, tempBuff, sampleRate);
                
                downSample(tempBuff, intInput);
            }else{
                downSample(inChannel, intInput);
            }
            if(chunkSize >= intInput.size()){
                ofLogError("ofxWebRTC_VAD::process") << "Chunk size (" << chunkSize<< ") is larger than the buffer size ("<<intInput.size()<<").  Not processing. lower the samplerate of this object or change the soundsteam buffersize";
                return;
            }
            int activity = 0;
            for(size_t startSample = 0; startSample + chunkSize < intInput.size(); startSample += chunkSize) {
                auto a = vads[i] ->VoiceActivity(&intInput[startSample], chunkSize, sampleRate);
                if(a != Vad::kError){
                    activity += int(a);
                }else{
//                    score.channelsScore[i].error ++;
                }
            }
            updateRecording(i, score.channelsScore[i].updateState(activity, attack.get(), release.get()));
            
//            score.channelsScore[i].bActive |= (score.channelsScore[i].activity > 0);
        }
//        states[i].add(score.channelsScore[i].bActive, 1, attack.get(), release.get());
    }
    score.numFrames ++;
    
    
}

void copyFromCircularBuffer(ofxCircularSoundBuffer& src, shared_ptr<ofSoundBuffer>& dest, size_t samplesToCopy, size_t channel, size_t startFrame, bool bAppend){
    if(!dest){
        ofLogError("ofxWebRTC_VAD::copyFromCircularBuffer") << "Destination buffer is null";
        return;
    }
    size_t startDestIndex = 0;
    if(bAppend){
        startDestIndex = dest->size();
        dest->resize(dest->getNumFrames() + samplesToCopy);
    }
    
    auto nc = src.getNumChannels();
    size_t j = startDestIndex;
    for(size_t i = startFrame + channel; i < src.size() && j < dest->size(); i+= nc){
        dest->getBuffer()[j] = src [i];
        j++;
    }
    if(j < dest->size()){
        for(size_t i = channel; i < src.size() && j < dest->size(); i+= nc){
            dest->getBuffer()[j] = src [i];
            j++;
        }
    }
}

void ofxWebRTC_VAD::updateRecording(size_t channelIndex, ChannelState state){
    
    if(currentRecordings.size() != vads.size()){
        currentRecordings.resize(vads.size(), nullptr);
        lastRecEndFrame.resize(vads.size(), 0);
        currentRecStartFrame.resize(vads.size(), 0);
        ofLogNotice("ofxWebRTC_VAD::updateRecording") << "Recordings size was updated: " << currentRecordings.size();
    }
    
    if(state == OFX_VAD_ACTIVE){
        if(currentRecordings[channelIndex]){
            // copy the last buffer added to circular buffer
            copyFromCircularBuffer(circularBuffer, currentRecordings[channelIndex], inBufferSize,  channelIndex, circularBuffer.getPushIndex(), true);
        }
    }
    else if(state == OFX_VAD_CHANGE_TO_ACTIVE){
        int totalCopy = 0;
        bool bAppend = false;
        if(currentRecordings[channelIndex]){
            bAppend = true;
            auto n = audioOutCount - lastRecEndFrame[channelIndex];
            if( n < preRec.get()){
                totalCopy = n + 1;
            }else {
                ofLogError("ofxWebRTC_VAD::updateRecording") << "this should not be happening";
                return;
            }
        }else{
        
            totalCopy = (attack.get() + preRec.get() + 1);
            currentRecordings[channelIndex] = make_shared<ofSoundBuffer>();
            currentRecordings[channelIndex]->allocate(inBufferSize* totalCopy, 1);
            currentRecordings[channelIndex]->setSampleRate(inSampleRate);
            RecordingEventArgs args(currentRecordings[channelIndex], audioOutCount, 0, channelIndex);
            currentRecStartFrame[channelIndex] = audioOutCount - totalCopy - 1;
            newRecordingEvent.notify(this, args);
        }
        int pushIndex = circularBuffer.getPushIndex();
        int startCopy = pushIndex - ((totalCopy -1)* inBufferSize*circularBuffer.getNumChannels());
        
        if(startCopy < 0){
            startCopy = circularBuffer.getBuffer().size() + startCopy;
        }
        
        copyFromCircularBuffer(circularBuffer, currentRecordings[channelIndex], inBufferSize* totalCopy,  channelIndex, startCopy, bAppend);
        
    }
    else if(state == OFX_VAD_CHANGE_TO_INACTIVE){
        
        if(currentRecordings[channelIndex]){
            // copy the last buffer added to circular buffer
            copyFromCircularBuffer(circularBuffer, currentRecordings[channelIndex], inBufferSize,  channelIndex, circularBuffer.getPushIndex(), true);
            lastRecEndFrame[channelIndex] = audioOutCount;
            
//            std::scoped_lock lock(recordingsMutex);
//            recordings.push_back(currentRecordings[channelIndex]);
//
//            currentRecordings[channelIndex] = nullptr;

            if(preRec.get() == 0){
                pushCurrentRecording(channelIndex);
            }
        }
    }
    if(state == OFX_VAD_INACTIVE){
        if(currentRecordings[channelIndex]){
            if(audioOutCount - lastRecEndFrame[channelIndex] >= preRec.get()){
                pushCurrentRecording(channelIndex);
            }
        }
    }

    
}


void ofxWebRTC_VAD::pushCurrentRecording(size_t channelIndex){
    std::scoped_lock lock(recordingsMutex);
    recordings.push_back(currentRecordings[channelIndex]);
    
    currentRecordings[channelIndex] = nullptr;
    RecordingEventArgs args(recordings.back(), currentRecStartFrame[channelIndex], lastRecEndFrame[channelIndex], channelIndex);
    newRecordingEvent.notify(this, args);
    
}

ofxWebRTC_VAD::Score ofxWebRTC_VAD::getActivityScore(){
    std::scoped_lock<ofMutex> lck(scoreMutex);
    auto s = score;
    score.reset();
    return s;
}

void ofxWebRTC_VAD::setAggressiveness(Vad::Aggressiveness _aggressiveness){
    if(aggressiveness.load() != _aggressiveness){
        aggressiveness = _aggressiveness;
        bResetVads = true;
    }
}
Vad::Aggressiveness ofxWebRTC_VAD::getAggressiveness(){
    return (Vad::Aggressiveness) aggressiveness.load();
}

