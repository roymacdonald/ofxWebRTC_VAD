
#include "ofxWebRTC_VAD.h"

template<typename T>
void downSample(const ofSoundBuffer &in, vector<T> & out){
    out.resize(in.size());
    
    auto mn = std::numeric_limits<int16_t>::min();
    auto mx = std::numeric_limits<int16_t>::max();
    
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
    
    out = in;
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
    Score currentScore = score.load();
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
            for(size_t startSample = 0; startSample + chunkSize < intInput.size(); startSample += chunkSize) {
                auto a = vads[i] ->VoiceActivity(&intInput[startSample], chunkSize, sampleRate);
                if(a != Vad::kError){
                    currentScore.activity += int(a);
                }else{
                    currentScore.error ++;
                }
            }
        }
    }
    currentScore.numFrames ++;
    score = currentScore;
    
}


ofxWebRTC_VAD::Score ofxWebRTC_VAD::getActivityScore(){
    ofxWebRTC_VAD::Score s = score.load();
    score = ofxWebRTC_VAD::Score();
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
