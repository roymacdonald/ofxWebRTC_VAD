
#pragma once

#include "ofxSoundObjects.h"
#include "vad.h"


#include "ofxSamplerate.h"



class ofxVadChannelScoreState{
public:

    size_t numFramesSinceChange;
//    size_t numFramesSinceLastActive;
    bool bActive = false;
    bool bActiveFiltered = false;
    void add(bool active, size_t numFrames, int attack, int release){
        
        if(active == bActive){
            numFramesSinceChange += numFrames;
        }else{
            numFramesSinceChange=numFrames;
            bActive = active;
        }
        if(numFramesSinceChange > (active?attack:release)){
            
//            if(bActiveFiltered != active){
//                if(!active){
//                    numFramesSinceLastActive = 0;
//                }else{
//                    numFramesSinceLastActive
//                }
//
//            }
            
            bActiveFiltered = active;
        }
        

        
    }
};


class ofxWebRTC_VAD: public ofxSoundObject {
public:

    ofxWebRTC_VAD();
    virtual ~ofxWebRTC_VAD();
 
    
    void setProcessSamplerate(int sr);
    
    int getProcessSamplerate();
//
//    template<typename T>
//    void downSample(const ofSoundBuffer &in, vector<T> & out){
//        out.resize(in.size());
//
//        auto mn = std::numeric_limits<int16_t>::min();
//        auto mx = std::numeric_limits<int16_t>::max();
//
//        for(size_t i = 0; i < in.size(); i++){
//            if(in[i] > 0){
//                out[i] = mx * in[i];
//            }else{
//                out[i] = mn * in[i];
//            }
//        }
//    }
//
    
    void process(ofSoundBuffer &in, ofSoundBuffer &out) ;
    
    struct Score{
        struct ChannelScore{
            size_t activity =0;
            size_t error =0;
            size_t sinceChangeCount = 0;
//            size_t inactiveCount = 0;
            bool bActive = false;
            
            void reset(){
                activity =0;
                error =0;
                bActive = false;
            }
            friend std::ostream& operator << (std::ostream& os, const ChannelScore& s) {
                os << "Activity: " << s.activity  << " Errors: " << s.error << " Since Change Count: " << s.sinceChangeCount;
                return os;
            }
        };
        
        vector<ChannelScore> channelsScore;
        size_t numFrames = 0;
        
        
        
        void reset(){
            numFrames = 0;
            for(auto& c: channelsScore){
                c.reset();
            }
        }
        
        friend std::ostream& operator << (std::ostream& os, const Score& s) {
//            os << "Num Frames: " << s.numFrames<< "\n";
//            for(auto& c: s.channelsScore){
//                os << "   " << c << "\n";
//            }
            return os;
        }
        
    };
    
    
    vector<ofxVadChannelScoreState>states;
    
    Score getActivityScore();
    
    
    ofParameter<int> vadAggressiveness = {"VAD Aggressiveness", 3, 0, 3};
    ofParameter<int> attack = {"Attack", 2, 0, 20};
    ofParameter<int> release = {"Release", 1, 0, 20};
    ofParameter<int> preRec = {"Pre Recording", 2, 0, 40};
    
    ofParameterGroup parameters = {"ofxWebRTC_VAD", vadAggressiveness, attack, release, preRec};
    
private:
    
    void setAggressiveness(Vad::Aggressiveness _aggressiveness);
    Vad::Aggressiveness getAggressiveness();

    
    Score score;
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

    
    size_t audioOutCount = 0;
    
};
