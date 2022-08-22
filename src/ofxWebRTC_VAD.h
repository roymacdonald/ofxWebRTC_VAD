
#pragma once

#include "ofxSoundObjects.h"
#include "vad.h"


#include "ofxSamplerate.h"
#include "ofxSoundUtils.h"
#include "VadConstants.h"

#include "ofxVadRecorder.h"

class Score{
public:
    class ChannelScore{
        
        ChannelState state;
        bool bActive = false;
        bool bActiveFiltered = false;
    public:
        
        const ChannelState& getState(){return state;}
        
        size_t numFramesSinceChange;

        ChannelState updateState( int activity, int attack, int release){
            bool active = (activity > 0);
            
            if(active == bActive){
                numFramesSinceChange ++;
            }else{
                numFramesSinceChange=1;
                bActive = active;
            }
            ChannelState ret = (bActiveFiltered?OFX_VAD_ACTIVE:OFX_VAD_INACTIVE);
            if(numFramesSinceChange > (active?attack:release)){
                if(bActiveFiltered != active){
                    ret = (active?OFX_VAD_CHANGE_TO_ACTIVE:OFX_VAD_CHANGE_TO_INACTIVE);
                }
                bActiveFiltered = active;
            }
            state = ret;
            return ret;
        }
        friend std::ostream& operator << (std::ostream& os, const ChannelScore& s) {
            os << "Active: " << s.bActive ;
            os << "  ActiveFiltered: " << s.bActiveFiltered ;
            os << "  numFramesSinceChange: " << s.numFramesSinceChange ;
            return os;
        }
    };
    
    vector<ChannelScore> channelsScore;
    size_t numFrames = 0;
    
    
    void reset(){
        numFrames = 0;
    }
    
    friend std::ostream& operator << (std::ostream& os, const Score& s) {
            os << "Num Frames: " << s.numFrames<< "\n";
            for(auto& c: s.channelsScore){
                os << "   " << c << "\n";
            }
        return os;
    }
    
    
};



class ofxWebRTC_VAD: public ofxSoundObject {
public:

    ofxWebRTC_VAD();
    virtual ~ofxWebRTC_VAD();
 
    
    void setProcessSamplerate(int sr);
    
    int getProcessSamplerate();
  
    void process(ofSoundBuffer &in, ofSoundBuffer &out) ;
    
        
    Score getActivityScore();
    
    
    ofParameter<int> vadAggressiveness = {"VAD Aggressiveness", 3, 0, 3};
    ofParameter<int> attack = {"Attack", 20, 0, 100};
    ofParameter<int> release = {"Release", 20, 0, 100};
    
    ofParameterGroup parameters = {"ofxWebRTC_VAD", vadAggressiveness, attack, release};
    

    Score score;
    
    size_t getInBufferSize(){return inBufferSize;}
    size_t getInSampleRate(){return inSampleRate;}
    size_t getAudioOutCount(){return audioOutCount;}

    shared_ptr<ofxVadRecorder> recorder = nullptr;
    
protected:
    

private:
    
    void setAggressiveness(Vad::Aggressiveness _aggressiveness);
    Vad::Aggressiveness getAggressiveness();

    
    
    ofMutex scoreMutex;

    std::atomic<int> sampleRate;

    ofEventListeners listeners;
    
    
//    kVadNormal = 0,
//    kVadLowBitrate = 1,
//    kVadAggressive = 2,
//    kVadVeryAggressive = 3
    
    std::atomic<int> aggressiveness;
    vector<std::unique_ptr<Vad> > vads;
    std::atomic<bool>  bResetVads;
    ofxSamplerate srConverter;
    
    size_t inBufferSize = 0;
    size_t inSampleRate = 0;
    
    size_t audioOutCount = 0;
    
    
    
    
};
