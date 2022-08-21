
#pragma once

#include "ofxSoundObjects.h"
#include "vad.h"


#include "ofxSamplerate.h"
#include "ofxSoundUtils.h"



//class ofxVadChannelScoreState{
//public:

//    size_t numFramesSinceChange;
////    size_t numFramesSinceLastActive;
//    bool bActive = false;
//    bool bActiveFiltered = false;
//    void add(bool active, size_t numFrames, int attack, int release){
//
//        if(active == bActive){
//            numFramesSinceChange += numFrames;
//        }else{
//            numFramesSinceChange=numFrames;
//            bActive = active;
//        }
//        if(numFramesSinceChange > (active?attack:release)){
//
////            if(bActiveFiltered != active){
////                if(!active){
////                    numFramesSinceLastActive = 0;
////                }else{
////                    numFramesSinceLastActive
////                }
////
////            }
//
//            bActiveFiltered = active;
//        }
//
//
//
//    }
//};


class ofxWebRTC_VAD: public ofxSoundObject {
public:

    ofxWebRTC_VAD();
    virtual ~ofxWebRTC_VAD();
 
    
    void setProcessSamplerate(int sr);
    
    int getProcessSamplerate();

//
    
    void process(ofSoundBuffer &in, ofSoundBuffer &out) ;
    
    enum ChannelState{
        OFX_VAD_INACTIVE = 0,
        OFX_VAD_ACTIVE,
        OFX_VAD_CHANGE_TO_ACTIVE,
        OFX_VAD_CHANGE_TO_INACTIVE
    };
    
    struct Score{
        struct ChannelScore{
//            size_t activity =0;
//            size_t error =0;
//            size_t sinceChangeCount = 0;
//            size_t inactiveCount = 0;
//            bool bActive = false;
            
//            void reset(){
//                activity =0;
//                error =0;
//                bActive = false;
//            }
            
        //    size_t numFramesSinceLastActive;
            size_t numFramesSinceChange;
            bool bActive = false;
            bool bActiveFiltered = false;
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
        //                if(!active){
        //                    numFramesSinceLastActive = 0;
        //                }else{
        //                    numFramesSinceLastActive
        //                }
        //
                        ret = (active?OFX_VAD_CHANGE_TO_ACTIVE:OFX_VAD_CHANGE_TO_INACTIVE);
                    }
                    
                    bActiveFiltered = active;
                }
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
//            for(auto& c: channelsScore){
//                c.reset();
//            }
        }
        
        friend std::ostream& operator << (std::ostream& os, const Score& s) {
//            os << "Num Frames: " << s.numFrames<< "\n";
//            for(auto& c: s.channelsScore){
//                os << "   " << c << "\n";
//            }
            return os;
        }
        
    };
    
    
//    vector<ofxVadChannelScoreState>states;
    
    Score getActivityScore();
    
    
    ofParameter<int> vadAggressiveness = {"VAD Aggressiveness", 3, 0, 3};
    ofParameter<int> attack = {"Attack", 20, 0, 100};
    ofParameter<int> release = {"Release", 20, 0, 100};
    ofParameter<int> preRec = {"Pre Recording", 20, 0, 100};
    
    ofParameterGroup parameters = {"ofxWebRTC_VAD", vadAggressiveness, attack, release, preRec};
    
    void updateRecording(size_t channelIndex, ChannelState state);
    
    vector<shared_ptr<ofSoundBuffer>> recordings;
    Score score;
    class RecordingEventArgs{
    public:
        RecordingEventArgs(){}
        RecordingEventArgs(shared_ptr<ofSoundBuffer> _buffer, size_t _startFrame, size_t _endFrame, size_t _channel):
        buffer(_buffer),startFrame(_startFrame), endFrame(_endFrame), channel(_channel)
        {
            isFinished = (endFrame>0);
        }
        shared_ptr<ofSoundBuffer> buffer;
        size_t startFrame, endFrame;
        size_t channel;
        bool isFinished = false;
    };
    
    ofEvent<RecordingEventArgs> newRecordingEvent;
protected:
    
    vector<size_t> lastRecEndFrame;
    vector<size_t> currentRecStartFrame;
    void pushCurrentRecording(size_t channelIndex);
    
private:
    vector<shared_ptr<ofSoundBuffer> > currentRecordings;
    
    ofxCircularSoundBuffer circularBuffer;
    
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
    
    std::mutex recordingsMutex;
    
    
    
};
