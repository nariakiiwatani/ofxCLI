#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	prompt_.subscribe("str", std::function<void(std::string)>([](std::string s) {
		cout << s << endl;
	}), {"hoge"});
	prompt_.subscribe("i2", std::function<void(int,int)>([](int _0, int _1) {
		cout << _0 << " " << _1 << endl;
	}));
	prompt_.subscribe("many", std::function<void(int,int,int,int,int,int,int,int)>([](int _0, int _1, int _2, int _3, int _4, int _5, int _6, int _7) {
		cout << _0 << " " << _1 << " " << _2 << " " << _3 << " " << _4 << " " << _5 << " " << _6 << " " << _7 << endl;
	}), {0,1,2,3,4,5,6,7});
	
	osc_sender_.bind(prompt_);
}

//--------------------------------------------------------------
void ofApp::update(){
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	prompt_.drawDebug(0,10);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
	
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
	
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
	
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
	
}
