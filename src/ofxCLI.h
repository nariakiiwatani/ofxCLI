#pragma once

#include "ofEvents.h"
#include "Testate.h"

namespace ofx {
namespace cli {
class LineEditor
{
public:
	std::string get() const { return buffer_; }
	std::size_t getCursorPos() const { return cursor_pos_; }
	
	bool insert(char ch);
	bool insert(const std::string &str);
	
	bool deleteL(int amount=1);
	bool deleteR(int amount=1);
	bool clear();
	
	bool moveCursorL(int amount=1);
	bool moveCursorR(int amount=1);
	bool moveCursor(int amount);
	bool moveCursorWordL();
	bool moveCursorWordR();
	bool moveCursorHome();
	bool moveCursorEnd();
private:
	std::string buffer_;
	std::size_t cursor_pos_=0;
};
class Prompt {
public:
	struct Settings {
		Settings(){}
		bool enable_key_event = true;
	};
	Prompt(const Settings &settings=Settings());
	void draw() const;
protected:
	void keyPressed(ofKeyEventArgs &key);
	void keyReleased(ofKeyEventArgs &key);
	LineEditor editor_;
	std::deque<std::string> history_;
	
	struct SpecialKeys {
		bool shift=false;
		bool alt=false;
		bool control=false;
		bool command=false;
	} special_keys_;
	int select_length_=0;
	
	Testate testate_;
};
}}
