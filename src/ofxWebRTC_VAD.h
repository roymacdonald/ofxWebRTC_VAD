
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
        float rms = 0;
        size_t rmsCount = 0;
    public:
        
        const ChannelState& getState(){return state;}
        
        size_t numFramesSinceChange;

        ChannelState updateState( int activity, float rms, int attack, int release, float rmsMultiplier, float minRms, float maxRms){
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
            
            if(ret == OFX_VAD_ACTIVE || ret == OFX_VAD_CHANGE_TO_ACTIVE){
                this->rms = ofMap( rms * rmsMultiplier, minRms, maxRms,0,1, true);
            }else{
                this->rms= 0;
            }
            
            if(ret == OFX_VAD_CHANGE_TO_ACTIVE){
                if(rms * rmsMultiplier < minRms){
                    ret = OFX_VAD_INACTIVE;
                    bActive = false;
                    bActiveFiltered = false;
                }
            }
            
            state = ret;
//            this->rms = rms;
            rmsCount++;
            return ret;
        }
        
        float getRms(){
            // auto r = maxRms;
            // maxRms = 0;
            return rms;
//            if(rmsCount == 0){
//                return 0.0;
//            }
//
//            float temp = rms;
//            temp /= rmsCount;
//            rms = 0;
//            rmsCount = 0;
//            return temp;
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
    
    ofParameter<float> minRms = {"Min RMS", 0., 0, 1};
    ofParameter<float> maxRms = {"Max RMS", 1., 0, 1};
    
    ofParameter<float> rmsMultiplier = {"RMS Multiplier", 1000, 0, 10000};
    ofParameterGroup activityRms = {"Activity Rms", minRms, maxRms, rmsMultiplier};
    
    
    ofParameterGroup parameters = {"ofxWebRTC_VAD", vadAggressiveness, attack, release, activityRms};
    
    
    

    Score score;
    
    size_t getInBufferSize(){return inBufferSize;}
    size_t getInSampleRate(){return inSampleRate;}
    size_t getAudioOutCount(){return audioOutCount;}

    shared_ptr<ofxVadRecorder> recorder = nullptr;
    
    std::atomic<bool> bBypass;
    
    
    void disableRecordings();
    void enableRecordings();
    
    
    
    
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
