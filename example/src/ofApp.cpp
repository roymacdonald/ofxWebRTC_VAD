#include "ofApp.h"

/// Uncomment the following line if you want to use a load dialog instead of a fixed file path to open
#define USE_LOAD_DIALOG

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);
    //----- Loading sound player begin -------.
#ifdef USE_LOAD_DIALOG
    ofFileDialogResult result = ofSystemLoadDialog();
    if (result.bSuccess) {
        player.load(result.getPath());
    }
#else
    

    player.load( ofToDataPath("../../../../../examples/sound/soundPlayerExample/bin/data/sounds/beat.wav",true),
                //set the following to true if you want to stream the audio data from the disk on demand instead of
                //reading the whole file into memory. Default is false
                false);
#endif
    //----- Loading sound player end -------.

    //----- Sound stream setup begin -------.
    // the sound stream is in charge of dealing with your computers audio device.
    // lets print to the console the sound devices that can output sound.
    ofxSoundUtils::printOutputSoundDevices();
    
    auto outDevices = ofxSoundUtils::getOutputSoundDevices();
    
    // IMPORTANT!!!
    // The following line of code is where you set which audio interface to use.
    // the index is the number printed in the console inside [ ] before the interface name
    // You can use a different input and output device.
    
    int outDeviceIndex = 0;
    
    cout << ofxSoundUtils::getSoundDeviceString(outDevices[outDeviceIndex], false, true) << endl;
    
    
    bufferSize = 256*2;
    
    ofSoundStreamSettings soundSettings;
    soundSettings.numInputChannels = 0;
    soundSettings.numOutputChannels = 2;
    soundSettings.sampleRate = player.getSoundFile().getSampleRate();
    soundSettings.bufferSize = bufferSize;
    soundSettings.numBuffers = 1;
    
    stream.setup(soundSettings);
    
    
    // it is important to set up which object is going to deliver the audio data to the sound stream.
    // thus, we need to set the stream's output. The output object is going to be the last one of the audio signal chain, which is set up further down
    stream.setOutput(output);

    //-------Sound stream setup end -------.
    

    // ------- GUI setup begin -------
    gui.setup();
    gui.add(pan.set("PAN", 0, -1,1));
    gui.add(player.volume);
    gui.add(speed.set("Speed", 1, 0, 10));
    
    gui.add(vadAggressiveness);
    //----- gui listeners
    pan.addListener(this, &ofApp::panChanged);
    speed.addListener(this, &ofApp::speedChanged);
    
    listeners.push(vadAggressiveness.newListener([&](int& a){
        vad.setAggressiveness((Vad::Aggressiveness)a);
    }));
    
    
    // ------- GUI setup end -------

    
    // ------ waveforms ---------
    // the waveformDraw class setup receives the rectangle where it is going to be drawn
    // you can skip this and pass this while drawing if you are changing where this is going to be drawn.
    // As well, the waveformDraw class inherits from ofRectangle so you can access the functions of the latter.
    
    float x = gui.getShape().getMaxX()+10;
    fullFileWaveform.setup( x, 0, ofGetWidth() - x, ofGetHeight()/3);
    wave.setup(0, fullFileWaveform.getMaxY(), ofGetWidth(), ofGetHeight() - fullFileWaveform.getMaxY());
    // the fullFileWaveform object will have a static waveform for the whole audio file data. This is only created once as it will read the whole sound file data.
    // For such, we need to get the sound buffer from the sound file in order to get the whole file's data.
    // calling player.getBuffer(), which actually is a function, will return the players current buffer, the one that is being sent to the sound device, so it will not work for what we are trying to achieve.
    // the waveformDraw's makeMeshFromBuffer(ofBuffer&) function will create a waveform from the buffer passed
    
    fullFileWaveform.makeMeshFromBuffer( player.getSoundFile().getBuffer());
    

    // wave object will be part of the signal chain and will update on real time as the audio passes to the output
    
    // --------- Audio signal chain setup.-------
    // Each of our objects need to connect to each other in order to create a signal chain, which ends with the output; the object that we set as the sound stream output.

    player.connectTo(vad).connectTo(wave).connectTo(output);

    
    player.play();
    
    // set if you want to either have the player looping (playing over and over again) or not (stop once it reaches the its end).
    player.setLoop(true);
    
    if(!player.isLooping()){
        // if the player is not looping you can register  to the end event, which will get triggered when the player reaches the end of the file.
        playerEndListener = player.endEvent.newListener(this, &ofApp::playerEnded);
    }
}
//--------------------------------------------------------------
void ofApp::playerEnded(size_t & id){
    // This function gets called when the player ends. You can do whatever you need to here.
    // This event happens in the main thread, not in the audio thread.
    cout << "the player's instance " << id << "finished playing" << endl;
    
}
//--------------------------------------------------------------
void ofApp::speedChanged(float&){
    player.setSpeed(speed);
}
//--------------------------------------------------------------
void ofApp::panChanged(float&f){
    
    player.setPan(pan);
}
//--------------------------------------------------------------
void ofApp::exit(){
    stream.close();
}
//--------------------------------------------------------------
void ofApp::update(){
}
//--------------------------------------------------------------
void ofApp::drawVadScore(){
    scores.push_back(vad.getActivityScore());
    float frameDrawSize = ofMap(bufferSize, 0, player.getNumFrames(), 0, fullFileWaveform.width);
    float y0 = fullFileWaveform.getMinY();

    ofPushStyle();
    ofSetColor(ofColor::magenta);
    float x = fullFileWaveform.getMinX();

    for(auto& s: scores){
        float w = s.numFrames*frameDrawSize;
        for(size_t i = 0; i < s.channelsScore.size(); i++){
            float h = fullFileWaveform.height / s.channelsScore.size();
            if(s.channelsScore[i].activity > 0){
                ofDrawRectangle(x, y0 + (h*i), w, h);
            }
        }
        x+= w;
    }

    ofPopStyle();

}
//--------------------------------------------------------------
void ofApp::draw(){
    drawVadScore();

    ofSetColor(ofColor::white);

    fullFileWaveform.draw();
    
    ofSetColor(ofColor::red);
    float playhead = ofMap(player.getPosition(), 0,1, fullFileWaveform.getMinX(),fullFileWaveform.getMaxX());
    ofDrawLine(playhead, 0, playhead, fullFileWaveform.getMaxY());
    
    if(fullFileWaveform.inside(ofGetMouseX(), ofGetMouseY())){
        ofSetColor(ofColor::cyan);
        ofDrawLine(ofGetMouseX(), 0, ofGetMouseX(), fullFileWaveform.getMaxY());
    }
    ofSetColor(ofColor::white);
    wave.draw();

    
    ofSetColor(ofColor::yellow);
    player.drawDebug(20, gui.getShape().getMaxY() + 20);
    
    
    if(scores.size()) ofDrawBitmapStringHighlight( ofToString(scores.back()), ofGetWidth() - 400, ofGetHeight() - 200);
    
    gui.draw();
}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    // if you clic and release you'll move the player to the position relative to the mouse
    if(fullFileWaveform.inside(x, y)){
    player.setPositionMS(ofMap(x, fullFileWaveform.getMinX(), fullFileWaveform.getMaxX(), 0, player.getDurationMS()));
    }
}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){}
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){}
