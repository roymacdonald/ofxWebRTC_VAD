
#pragma once

#include "ofxSoundObjects.h"
#include "vad.h"


#include "ofxSamplerate.h"





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
        size_t activity =0;
        size_t numFrames = 0;
        size_t error =0;
        
        friend std::ostream& operator << (std::ostream& os, const Score& s) {
            os << "Activity: " << s.activity << " Num Frames: " << s.numFrames << " Errors: " << s.error;
            return os;
        }
        
    };
    
    Score getActivityScore();
    
    void setAggressiveness(Vad::Aggressiveness _aggressiveness);
    Vad::Aggressiveness getAggressiveness();
    
private:
    std::atomic<Score> score;

    std::atomic<int> sampleRate;

    
//    kVadNormal = 0,
//    kVadLowBitrate = 1,
//    kVadAggressive = 2,
//    kVadVeryAggressive = 3
    
    std::atomic<int> aggressiveness;
    vector<std::unique_ptr<Vad> > vads;
    std::atomic<bool>  bResetVads;
    ofxSamplerate srConverter;

    
};
