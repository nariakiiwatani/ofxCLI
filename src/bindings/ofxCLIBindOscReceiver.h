#pragma once

#include "ofxOscReceiver.h"
#include "ofxCLI.h"
#include "Testate.h"

namespace ofx {
namespace cli {
	
namespace {
	std::string getAsString(ofxOscMessage &msg, int index) {
		switch(msg.getArgType(index)) {
			case OFXOSC_TYPE_INT32:			return ofToString(msg.getArgAsInt32(index));
			case OFXOSC_TYPE_INT64:			return ofToString(msg.getArgAsInt64(index));
			case OFXOSC_TYPE_FLOAT:			return ofToString(msg.getArgAsFloat(index));
			case OFXOSC_TYPE_DOUBLE:		return ofToString(msg.getArgAsDouble(index));
			case OFXOSC_TYPE_STRING:		return ofToString(msg.getArgAsString(index));
			case OFXOSC_TYPE_SYMBOL:		return ofToString(msg.getArgAsSymbol(index));
			case OFXOSC_TYPE_CHAR:			return ofToString(msg.getArgAsChar(index));
			case OFXOSC_TYPE_MIDI_MESSAGE:	return ofToString(msg.getArgAsMidiMessage(index));
			case OFXOSC_TYPE_TRUE:			return ofToString(msg.getArgAsBool(index));
			case OFXOSC_TYPE_FALSE:			return ofToString(msg.getArgAsBool(index));
			case OFXOSC_TYPE_NONE:			return ofToString(msg.getArgAsNone(index));
			case OFXOSC_TYPE_TRIGGER:		return ofToString(msg.getArgAsTrigger(index));
			case OFXOSC_TYPE_TIMETAG:		return ofToString(msg.getArgAsTimetag(index));
			case OFXOSC_TYPE_BLOB:			return ofToString(msg.getArgAsBlob(index));
			case OFXOSC_TYPE_RGBA_COLOR:	return ofToString(msg.getArgAsRgbaColor(index));
		}
		return "";
	}
}

class BindOscReceiver
{
public:
	struct Settings {
		Settings(){}
		int port=6000;
		bool ignore_first_slash=true;
	};
	void bind(Prompt &p, const Settings &s=Settings()) {
		prompt_ = &p;
		osc_receiver_.setup(s.port);
		ignore_first_slash_ = s.ignore_first_slash;
		ofAddListener(ofEvents().update, this, &BindOscReceiver::update);
		testate_.leaveWill([this]() {
			ofRemoveListener(ofEvents().update, this, &BindOscReceiver::update);
		});
	}
protected:
	void update(ofEventArgs&) {
		using namespace std;
		while(osc_receiver_.hasWaitingMessages()) {
			ofxOscMessage msg;
			osc_receiver_.getNextMessage(msg);
			auto program = msg.getAddress();
			if(ignore_first_slash_ && !program.empty() && program[0] == '/') {
				program = program.substr(1);
			}
			vector<string> args(msg.getNumArgs());
			for(int i = 0; i < msg.getNumArgs(); ++i) {
				args[i] = getAsString(msg,i);
			}
			
			prompt_->proc(program, args);
		}
	}
	Prompt *prompt_=nullptr;
	ofxOscReceiver osc_receiver_;
	bool ignore_first_slash_=true;
	
	Testate testate_;
};
}}
