#pragma once

#include "ofMain.h"

#include "waveformDraw.h"

#include "ofxSoundPlayerObject.h"
#include "ofxGui.h"
#include "ofxWebRTC_VAD.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    ofSoundStream stream;
    ofxSoundOutput output;
    // these are all subclasses of ofSoundObject
//    ofxSoundPlayerObject player;
    waveformDraw wave;

    ofxPanel gui;
    ofParameter<float> pan, speed;
    void speedChanged(float&);
    void panChanged(float&f);

    
//    vector<ofMesh> waveforms;
    
    vector<waveformDraw> fullFileWaveform;
    
        
    
    ofxWebRTC_VAD vad;
    
    ofParameter<int> vadAggressiveness = {"VAD Aggressiveness", 0, 0, 3};
    ofEventListeners listeners;
    
    vector<ofxWebRTC_VAD::Score> scores;
    
    int bufferSize;
    
    void drawVadScore();
};
