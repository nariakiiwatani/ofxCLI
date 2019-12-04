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

bool LineEditor::deleteSelected()
{
	if(selection_length_ == 0) {
		return false;
	}
	selection_length_ > 0 ? deleteR(selection_length_) : deleteL(-selection_length_);
	selection_length_ = 0;
	return true;
};

bool LineEditor::clear()
{
	buffer_.clear();
	cursor_pos_ = 0;
	selection_length_ = 0;
}

bool LineEditor::moveCursorL()
{
	switch(move_mode_) {
		case CHAR:	return moveCursorCharL(1);
		case WORD:	return moveCursorWordL();
		case WHOLE:	return moveCursorHome();
	}
	return false;
}

bool LineEditor::moveCursorR()
{
	switch(move_mode_) {
		case CHAR:	return moveCursorCharR(1);
		case WORD:	return moveCursorWordR();
		case WHOLE:	return moveCursorEnd();
	}
	return false;
}

bool LineEditor::moveCursorCharL(int amount)
{
	if(cursor_pos_ < amount) {
		return false;
	}
	if(selection_mode_) {
		selection_length_ += amount;
	}
	cursor_pos_-=amount;
	return true;
}
bool LineEditor::moveCursorCharR(int amount)
{
	if(buffer_.size()-cursor_pos_ < amount) {
		return false;
	}
	if(selection_mode_) {
		selection_length_ -= amount;
	}
	cursor_pos_+=amount;
	return true;
}
bool LineEditor::moveCursorChar(int amount)
{
	return (amount > 0 && moveCursorCharR(amount))
	|| (amount < 0 && moveCursorCharL(-amount))
	|| amount == 0;
}
namespace {
	std::string delimiters = " \t\n";
}
bool LineEditor::moveCursorWordL()
{
	if(cursor_pos_ != 0 && delimiters.find(buffer_[cursor_pos_-1]) != string::npos) {
		moveCursorCharL();
		moveCursorWordL();
		return true;
	}
	string::reverse_iterator start = buffer_.rbegin()+(buffer_.size()-cursor_pos_); 
	auto found = std::find_first_of(start, buffer_.rend(), begin(delimiters), end(delimiters));
	auto newpos = found != buffer_.rend() ? buffer_.size() - std::distance(buffer_.rbegin(), found) : string::npos;
	return (newpos != string::npos && moveCursorCharL(cursor_pos_-newpos)) || moveCursorHome();
}
bool LineEditor::moveCursorWordR()
{
	if(cursor_pos_ != buffer_.size() && delimiters.find(buffer_[cursor_pos_]) != string::npos) {
		moveCursorCharR();
		moveCursorWordR();
		return true;
	}
	auto newpos = buffer_.find_first_of(delimiters, cursor_pos_);
	return (newpos != string::npos && moveCursorCharR(newpos-cursor_pos_)) || moveCursorEnd();
}

bool LineEditor::moveCursorHome()
{
	return cursor_pos_ > 0 && moveCursorCharL(cursor_pos_);
}
bool LineEditor::moveCursorEnd()
{
	return cursor_pos_ < buffer_.size() && moveCursorCharR(buffer_.size()-cursor_pos_);
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
	switch(key.key) {
		case OF_KEY_SHIFT:	special_keys_.shift = true;	break;
		case OF_KEY_ALT:	special_keys_.alt = true;	break;
		case OF_KEY_CONTROL:	special_keys_.control = true;	break;
		case OF_KEY_COMMAND:	special_keys_.command = true;	break;
	}
	if(special_keys_.shift) {
		editor_.enterSelectionMode();
	}
	editor_.setMoveMode(special_keys_.command ? LineEditor::WHOLE : (special_keys_.alt ? LineEditor::WORD : LineEditor::CHAR));
	switch(key.key) {
		case OF_KEY_LEFT:
			editor_.moveCursorL();
			clear_select = !special_keys_.shift;
			break;
		case OF_KEY_RIGHT:
			editor_.moveCursorR();
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
			editor_.deleteSelected() || editor_.deleteR();
			clear_select = true;
			break;
		case OF_KEY_BACKSPACE:
			editor_.deleteSelected() || editor_.deleteL();
			clear_select = true;
			break;
		case OF_KEY_TAB:
			if(!is_suggesting_) {
				vector<string> candidates;
				for(auto &&i : subscribed_) {
					candidates.push_back(i.first);
				}
				suggest_.updateCandidates(candidates, editor_.getText());
				is_suggesting_ = true;
			}
			if(special_keys_.shift ? suggest_.hasPrev() : suggest_.hasNext()) {
				string str = (special_keys_.shift ? suggest_.prev() : suggest_.next()) + " ";
				editor_.clear();
				editor_.insert(str);
			}
			break;
		case OF_KEY_RETURN: {
			auto &&str = editor_.getText();
			if(!str.empty()) {
				proc();
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
		editor_.clearSelection();
		is_suggesting_ = false;
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
	if(!special_keys_.shift) {
		editor_.leaveSelectionMode();
	}
	editor_.setMoveMode(special_keys_.command ? LineEditor::WHOLE : (special_keys_.alt ? LineEditor::WORD : LineEditor::CHAR));
}

bool Prompt::unsubscribe(Prompt::SubscriberIdentifier identifier)
{
	auto result = find_if(begin(subscribed_), end(subscribed_), [identifier](const pair<std::string, Subscriber> &p) {
		return p.second.identifier == identifier;
	});
	if(result == end(subscribed_)) {
		return false;
	}
	subscribed_.erase(result);
	return true;
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
void Prompt::proc()
{
	proc(editor_.getText());
}
void Prompt::proc(const std::string &command)
{
	auto args = split(command);
	if(args.empty()) {
		return;
	}
	auto program = args[0];
	args.erase(begin(args));
	proc(program, args);
}
void Prompt::proc(const std::string &program, const std::vector<std::string> &args)
{
	Testate if_unsubscribed([this,program,args]() {
		ofJson json = {{"program",program},{"args",args}};
		ofNotifyEvent(UNSUBSCRIBED, json, this);
	});
	auto identifiers = subscribed_.equal_range(program);
	for(auto it = identifiers.first; it != identifiers.second; ++it) {
		auto deduced_args = it->second.func(args);
		ofJson json = {{"program",program},{"args",deduced_args}};
		ofNotifyEvent(SUBSCRIBED, json, this);
		if_unsubscribed.expire();
	}
}

vector<vector<string>> Prompt::getTips() const
{
	auto args = split(editor_.getText());
	if(args.empty()) {
		return {};
	}
	vector<vector<string>> ret;
	auto identifiers = subscribed_.equal_range(args[0]);
	for(auto it = identifiers.first; it != identifiers.second; ++it) {
		ret.push_back(it->second.tip);
	}
	return ret;
}

void Prompt::draw(float x, float y) const
{
	ofPushMatrix();
	ofTranslate(x,y);
	ofPushStyle();
	ofSetColor(ofColor::gray);
	ofDrawRectangle(editor_.getCursorPos()*8, -4, editor_.getSelectionLength()*8, 6);
	ofSetColor(ofColor::black);
	ofDrawRectangle(editor_.getCursorPos()*8, -10, 8, 12);
	ofPopStyle();
	ofDrawBitmapString(editor_.getText(), 0, 0);
	ofPopMatrix();
}
void Prompt::drawDebug(float x, float y) const
{
	ofPushMatrix();
	ofTranslate(x,0);
	for(auto &&h : history_) {
		ofDrawBitmapString(h, 0, y);
		y += 10;
	}
	ofPopMatrix();
	draw(x,y);
	y += 20;
	
	ofPushMatrix();
	auto &&tips = getTips();
	for(auto &tip : tips) {
		ofDrawBitmapStringHighlight(ofJoinString(tip, " "), x, y, ofFloatColor(1,1,0,0.5f), ofFloatColor(0,0,0,1));
		y += 20;
	}
	ofPopMatrix();
}
