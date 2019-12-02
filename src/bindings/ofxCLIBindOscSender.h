#pragma once

#include "ofxCLI.h"
#include "Testate.h"
#include "ofxOscSender.h"
#include "ofJson.h"

namespace ofx {
namespace cli {
namespace {
	void addArg(const ofJson &json, ofxOscMessage &msg) {
		using namespace nlohmann::detail;
		switch(json.type()) {
			case value_t::null:				msg.addNoneArg();	break;
			case value_t::boolean:			msg.addBoolArg(json);	break;
			case value_t::string:			msg.addStringArg(json);	break;
			case value_t::number_integer:	msg.addIntArg(json);	break;
			case value_t::number_unsigned:	msg.addIntArg(json);	break;
			case value_t::number_float:		msg.addFloatArg(json);	break;
			case value_t::object:			
			case value_t::array:
			case value_t::discarded:		break;
		}
	}
}
class BindOscSender
{
public:
	struct Settings {
		Settings(){}
		std::string host="localhost";
		std::int16_t port=6000;
		bool add_leading_slash=true;
		bool use_subscribed=true;
		bool use_unsubscribed=true;
	};
	void bind(Prompt &p, const Settings &s=Settings()) {
		prompt_ = &p;
		osc_sender_.setup(s.host, s.port);
		add_leading_slash_ = s.add_leading_slash;
		if(s.use_subscribed) {
			ofAddListener(prompt_->SUBSCRIBED, this, &BindOscSender::callback);
			testate_.leaveWill([this]() {
				ofRemoveListener(prompt_->SUBSCRIBED, this, &BindOscSender::callback);
			});
		}
		if(s.use_unsubscribed) {
			ofAddListener(prompt_->UNSUBSCRIBED, this, &BindOscSender::callback);
			testate_.leaveWill([this]() {
				ofRemoveListener(prompt_->UNSUBSCRIBED, this, &BindOscSender::callback);
			});
		}
	}
protected:
	Prompt *prompt_=nullptr;
	ofxOscSender osc_sender_;
	void callback(ofJson &json) {
		ofxOscMessage msg;
		std::string program = json["program"];
		if(add_leading_slash_ && (program.empty() || program[0] != '/')) {
			program = "/" + program;
		}
		msg.setAddress(program);
		for(auto &&v : json["args"]) {
			addArg(v, msg);
		}
		osc_sender_.sendMessage(msg);
	}
	
	bool add_leading_slash_=true;
	Testate testate_;
};
}}
