#include "ofxCLI.h"
#include "ofGraphics.h"
#include <regex>

using namespace std;
using namespace ofx::cli;

bool LineEditor::insert(char ch)
{
	buffer_.insert(cursor_pos_++, ofToString(ch));
	return true;
}
bool LineEditor::insert(const std::string &str)
{
	buffer_.insert(cursor_pos_, str);
	cursor_pos_ += str.size();
	return true;
}

bool LineEditor::deleteL(int amount)
{
	if(cursor_pos_ < amount) {
		return false;
	}
	cursor_pos_ -= amount;
	deleteR(amount);
	return true;
}
bool LineEditor::deleteR(int amount)
{
	if(cursor_pos_ > buffer_.size()-amount) {
		return false;
	}
	buffer_.erase(cursor_pos_, amount);
	return true;
}

bool LineEditor::clear()
{
	buffer_.clear();
	cursor_pos_ = 0;
}

bool LineEditor::moveCursorL(int amount)
{
	return cursor_pos_ >= amount && ((cursor_pos_-=amount) || true);
}
bool LineEditor::moveCursorR(int amount)
{
	return buffer_.size()-cursor_pos_ >= amount && ((cursor_pos_+=amount) || true);
}
bool LineEditor::moveCursor(int amount)
{
	return (amount > 0 && moveCursorR(amount))
	|| (amount < 0 && moveCursorL(-amount))
	|| amount == 0;
}
namespace {
	std::string delimiters = " \t\n";
}
bool LineEditor::moveCursorWordL()
{
	if(cursor_pos_ != 0 && delimiters.find(buffer_[cursor_pos_-1]) != string::npos) {
		moveCursorL();
		moveCursorWordL();
		return true;
	}
	string::reverse_iterator start = buffer_.rbegin()+(buffer_.size()-cursor_pos_); 
	auto found = std::find_first_of(start, buffer_.rend(), begin(delimiters), end(delimiters));
	auto newpos = found != buffer_.rend() ? buffer_.size() - std::distance(buffer_.rbegin(), found) : string::npos;
	return (newpos != string::npos && moveCursorL(cursor_pos_-newpos)) || moveCursorHome();
}
bool LineEditor::moveCursorWordR()
{
	if(cursor_pos_ != buffer_.size() && delimiters.find(buffer_[cursor_pos_]) != string::npos) {
		moveCursorR();
		moveCursorWordR();
		return true;
	}
	auto newpos = buffer_.find_first_of(delimiters, cursor_pos_);
	return (newpos != string::npos && moveCursorR(newpos-cursor_pos_)) || moveCursorEnd();
}

bool LineEditor::moveCursorHome()
{
	return cursor_pos_ > 0 && moveCursorL(cursor_pos_);
}
bool LineEditor::moveCursorEnd()
{
	return cursor_pos_ < buffer_.size() && moveCursorR(buffer_.size()-cursor_pos_);
}


Prompt::Prompt(const Settings &settings)
{
	if(settings.enable_key_event) {
		ofAddListener(ofEvents().keyPressed, this, &Prompt::keyPressed);
		ofAddListener(ofEvents().keyReleased, this, &Prompt::keyReleased);
		testate_.leaveWill([this]() {
			ofRemoveListener(ofEvents().keyPressed, this, &Prompt::keyPressed);
			ofRemoveListener(ofEvents().keyReleased, this, &Prompt::keyReleased);
		});
	}
}

void Prompt::keyPressed(ofKeyEventArgs &key)
{
	auto deleteSelected = [this]() {
		return select_length_ > 0 ? editor_.deleteR(select_length_) : editor_.deleteL(-select_length_)
		, select_length_ != 0;
	};
	bool clear_select = false;
	auto cursor_prev = editor_.getCursorPos();
	switch(key.key) {
		case OF_KEY_SHIFT:	special_keys_.shift = true;	break;
		case OF_KEY_ALT:	special_keys_.alt = true;	break;
		case OF_KEY_CONTROL:	special_keys_.control = true;	break;
		case OF_KEY_COMMAND:	special_keys_.command = true;	break;
		case OF_KEY_LEFT: {
			if(special_keys_.alt) {
				editor_.moveCursorWordL();
			}
			else if(special_keys_.command) {
				editor_.moveCursorHome();
			}
			else {
				editor_.moveCursorL();
			}
			clear_select = !special_keys_.shift;
		}	break;
		case OF_KEY_RIGHT:
			if(special_keys_.alt) {
				editor_.moveCursorWordR();
			}
			else if(special_keys_.command) {
				editor_.moveCursorEnd();
			}
			else {
				editor_.moveCursorR();
			}
			clear_select = !special_keys_.shift;
			break;
		case OF_KEY_UP:
			if(history_header_ != begin(history_)) {
				editor_.clear();
				editor_.insert(*--history_header_);
				editor_.moveCursorEnd();
			}
			clear_select = true;
			break;
		case OF_KEY_DOWN:
			if(history_header_ != end(history_)) {
				editor_.clear();
				if(++history_header_ != end(history_)) {
					editor_.insert(*history_header_);
				}
				editor_.moveCursorEnd();
			}
			clear_select = true;
			break;
		case OF_KEY_DEL:
			deleteSelected() || editor_.deleteR();
			clear_select = true;
			break;
		case OF_KEY_BACKSPACE:
			deleteSelected() || editor_.deleteL();
			clear_select = true;
			break;
		case OF_KEY_RETURN: {
			if(!editor_.get().empty()) {
				auto &&str = editor_.get();
				proc(str);
				if(history_.empty() || history_header_ != end(history_)-1 || *history_header_ != str) {
					history_.push_back(str);
				}
				history_header_ = end(history_);
				editor_.clear();
			}
			clear_select = true;
		}	break;
		default:
			if(0x20 <= key.key && key.key <= 0x7e) {
				clear_select = true;
				editor_.insert(key.key);
			}
			break;
	}
	if(clear_select) {
		select_length_ = 0;
	}
	else if(special_keys_.shift) {
		select_length_ += cursor_prev - editor_.getCursorPos();
	}
}

void Prompt::keyReleased(ofKeyEventArgs &key)
{
	switch(key.key) {
		case OF_KEY_SHIFT:	special_keys_.shift = false;	break;
		case OF_KEY_ALT:	special_keys_.alt = false;	break;
		case OF_KEY_CONTROL:	special_keys_.control = false;	break;
		case OF_KEY_COMMAND:	special_keys_.command = false;	break;
	}
}

bool Prompt::unsubscribe(Prompt::SubscriberIdentifier identifier)
{
	callback_.erase(identifier);
}

namespace {
	std::string regexEscaped(std::string str) {
		std::string escaper = "\\*+.?{}()[]-|$";
		for(auto &&ch : escaper) {
			ofStringReplace(str, ofToString(ch), "\\"+ofToString(ch));
		}
		return str;
	}
	std::string trimming(std::string src, std::string str) {
		str = regexEscaped(str);
		auto pos = src.find(str);
		if(pos != std::string::npos) {
			src = src.substr(str.length());
		}
		pos = src.rfind(str);
		if(pos != std::string::npos) {
			src = src.substr(0,pos);
		}
		return src;
	}
	std::vector<std::string> split(std::string src, std::vector<std::string> delimiters={" ", ","}, std::vector<std::string> quotes={"\"", "'"}) {
		auto dels = delimiters;
		dels.insert(std::end(dels), std::begin(quotes), std::end(quotes));
		for(int i = 0; i < dels.size(); ++i) {
			dels[i] = regexEscaped(dels[i]);
		}
		std::vector<std::string> patterns = {"((?!"+ofJoinString(dels,"|")+").)+"};
		for(int i = 0; i < quotes.size(); ++i) {
			auto src = regexEscaped(quotes[i]);
			patterns.push_back(src + "((?!" + src + ").)*" + src);
		}
		std::regex re(ofJoinString(patterns, "|"));
		std::vector<std::string> ret = {};
		for (std::sregex_iterator it{std::begin(src), std::end(src), re}, last{}; it != last; ++it) {
			std::string str = (*it).str();
			for(auto &&q : quotes) {
				str = trimming(str, q);
			}
			ret.push_back(str);
		}
		return ret;
	}
}
void Prompt::proc(const std::string &command)
{
	auto args = split(command);
	if(args.empty()) {
		return;
	}
	auto program = args[0];
	args.erase(begin(args));
	auto identifiers = identifier_.equal_range(program);
	for(auto it = identifiers.first; it != identifiers.second; ++it) {
		auto funcs = callback_.equal_range(it->second);
		for(auto it2 = funcs.first; it2 != funcs.second; ++it2) {
			it2->second(args);
		}
	}
}

void Prompt::draw() const
{
	int y = 10;
	for(auto &&h : history_) {
		ofDrawBitmapString(h, 0, y);
		y += 10;
	}
	ofPushStyle();
	ofSetColor(ofColor::gray);
	ofDrawRectangle(editor_.getCursorPos()*8, y-4, select_length_*8, 6);
	ofSetColor(ofColor::black);
	ofDrawRectangle(editor_.getCursorPos()*8, y-10, 8, 12);
	ofPopStyle();
	ofDrawBitmapString(editor_.get(), 0, y);
}
