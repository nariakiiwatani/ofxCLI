#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	// lambda function (with or without capture)
	prompt_.subscribe("basic", []() { cout << "basic event occured" << endl; });
	// you can set function with arguments
	prompt_.subscribe("int2", [](int _0, int _1) { cout << _0 << " " << _1 << endl; });
	// you can specify default argument(s)
	prompt_.subscribe("str", [](std::string s) { cout << s << endl; }, {"hoge"});
	// many parameters...
	prompt_.subscribe("many", std::function<void(int,int,int,int,int,int,int,int)>([](int _0, int _1, int _2, int _3, int _4, int _5, int _6, int _7) {
		cout << _0 << " " << _1 << " " << _2 << " " << _3 << " " << _4 << " " << _5 << " " << _6 << " " << _7 << endl;
	}), {0,1,2,3,4,5,6,7});
	// member function
	prompt_.subscribe("some", this, &ofApp::someFunction);
	// member function with default arguments
	prompt_.subscribe("some2", this, &ofApp::someFunction, {100, 50.3f, "some string"});
}

bool ofApp::someFunction(int a, float b, std::string c)
{
	cout << a << "," << b << "," << c << endl;
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
