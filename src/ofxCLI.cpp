#include "ofxCLI.h"
#include "ofGraphics.h"

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

bool LineEditor::moveCursor(int amount)
{
	auto newpos = cursor_pos_+amount;
	return (0 <= newpos && newpos <= buffer_.size()) && ((cursor_pos_ = newpos) || true);
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
	return (cursor_pos_ > 0) && ((cursor_pos_ = 0) || true);
}
bool LineEditor::moveCursorEnd()
{
	return (cursor_pos_ < buffer_.size()) && ((cursor_pos_ = buffer_.size()) || true);
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
		case OF_KEY_DEL:
			if(select_length_ > 0) {
				editor_.deleteR(select_length_);
			}
			else if(select_length_ < 0) {
				editor_.deleteL(-select_length_);
			}
			else {
				editor_.deleteR();
			}
			clear_select = true;
			break;
		case OF_KEY_BACKSPACE:
			if(select_length_ > 0) {
				editor_.deleteR(select_length_);
			}
			else if(select_length_ < 0) {
				editor_.deleteL(-select_length_);
			}
			else {
				editor_.deleteL();
			}
			clear_select = true;
			break;
		case OF_KEY_RETURN: {
			clear_select = true;
			if(!editor_.get().empty()) {
				history_.push_back(editor_.get());
				editor_.clear();
			}
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
