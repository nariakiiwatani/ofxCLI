#pragma once

#include "ofEvents.h"
#include "Testate.h"
#include <unordered_map>
#include <map>
#include "ofJson.h"
#include "detail/traits.h"

namespace ofx {
namespace cli {

class LineEditor
{
public:
	const std::string& getText() const { return buffer_; }
	std::size_t getCursorPos() const { return cursor_pos_; }
	int getSelectionLength() const { return selection_length_; }
	
	bool insert(char ch);
	bool insert(const std::string &str);
	
	bool deleteL(int amount=1);
	bool deleteR(int amount=1);
	bool deleteSelected();
	bool clear();
	
	bool moveCursorL();
	bool moveCursorR();
	bool moveCursorCharL(int amount=1);
	bool moveCursorCharR(int amount=1);
	bool moveCursorChar(int amount);
	bool moveCursorWordL();
	bool moveCursorWordR();
	bool moveCursorHome();
	bool moveCursorEnd();
	
	void enterSelectionMode() { selection_mode_ = true; }
	void leaveSelectionMode() { selection_mode_ = false; }
	void clearSelection() { selection_length_ = 0; }
	enum MoveMode {
		CHAR,WORD,WHOLE
	};
	void setMoveMode(MoveMode mode) { move_mode_ = mode; }
private:
	std::string buffer_;
	std::size_t cursor_pos_=0;
	int selection_length_=0;
	bool selection_mode_=false;
	MoveMode move_mode_=CHAR;
};
class Suggest
{
public:
	Suggest() {
		prev_ = std::end(candidates_);
	}
	bool setLoop(bool loop) { is_loop_ = loop; }
	void updateCandidates(const std::vector<std::string> &candidates, const std::string &str) {
		candidates_.clear();
		for(auto &&c : candidates) {
			if(str.empty() || c.find(str) == 0) {
				candidates_.push_back(c);
			}
		}
		prev_ = std::begin(candidates_);
	}
	std::string prev() {
		if(prev_ == std::begin(candidates_)) {
			prev_ = std::end(candidates_);
		}
		return *(--prev_);
	}
	std::string next() {
		if(++prev_ == std::end(candidates_) && is_loop_) {
			prev_ = std::begin(candidates_);
		}
		return *prev_;
	}
	bool hasPrev() const { return !candidates_.empty() && (is_loop_ || prev_ != std::begin(candidates_)); }
	bool hasNext() const { return !candidates_.empty() && (is_loop_ || prev_ != std::end(candidates_)); }
private:
	std::vector<std::string> candidates_;
	std::vector<std::string>::iterator prev_;
	bool is_loop_=true;
};
class Prompt {
public:
	struct Settings {
		Settings(){}
		bool enable_key_event = true;
	};
	Prompt(const Settings &settings=Settings());
	void draw(float x, float y) const;
	void drawDebug(float x, float y) const;
	
	using SubscriberIdentifier = std::size_t;
	template<typename F, typename Tuple=decltype(detail::make_tuple_from_args_list(std::declval<F>()))>
	SubscriberIdentifier subscribe(const std::string &command, F callback, const Tuple &default_args={});
	template<typename Listener, typename Ret, typename... Args>
	SubscriberIdentifier subscribe(const std::string &command, Listener *listener, Ret (Listener::*f)(Args...), const std::tuple<Args...> &default_args={});
	template<typename... Args>
	SubscriberIdentifier subscribe(const std::string &command, Args &...args);
	bool unsubscribe(SubscriberIdentifier identifier);
	
	void proc();
	void proc(const std::string &command);
	void proc(const std::string &program, const std::vector<std::string> &args);
	
	std::vector<std::vector<std::string>> getTips() const;
	
	ofEvent<ofJson> SUBSCRIBED, UNSUBSCRIBED;

	LineEditor& getCurrent() { return editor_; }
	const std::vector<std::string>& getHistory() const { history_; }
protected:
	void keyPressed(ofKeyEventArgs &key);
	void keyReleased(ofKeyEventArgs &key);
	LineEditor editor_;
	std::deque<std::string> history_;
	std::deque<std::string>::iterator history_header_;
	struct Subscriber {
		SubscriberIdentifier identifier;
		std::function<ofJson(std::vector<std::string>)> func;
		std::vector<std::string> tip;
	};
	std::unordered_multimap<std::string, Subscriber> subscribed_;
	
	Suggest suggest_;
	bool is_suggesting_=false;
	
	struct SpecialKeys {
		bool shift=false;
		bool alt=false;
		bool control=false;
		bool command=false;
	} special_keys_;
	
	Testate testate_;
	
	SubscriberIdentifier next_identifier_;
};
}}

template<typename F, typename Tuple>
inline ofx::cli::Prompt::SubscriberIdentifier ofx::cli::Prompt::subscribe(const std::string &command, F callback, const Tuple &default_args)
{
	Subscriber subs;
	subs.identifier = next_identifier_++;
	subs.func = [callback,default_args](std::vector<std::string> argv) {
		auto args = detail::make_tuple_from_vector(argv, default_args);
		detail::apply(callback, args);
		return args;
	};
	subs.tip = {detail::get_tuple_element_name(default_args)};
	subs.tip.insert(std::begin(subs.tip), command);
	subscribed_.insert(make_pair(command, subs));
	return subs.identifier;
}

template<typename Listener, typename Ret, typename... Args>
inline ofx::cli::Prompt::SubscriberIdentifier ofx::cli::Prompt::subscribe(const std::string &command, Listener *listener, Ret (Listener::*callback)(Args...), const std::tuple<Args...> &default_args)
{
	return subscribe(command, [listener, callback](Args... args) {
		(listener->*callback)(args...);
	}, default_args);
}

template<typename... Args>
inline ofx::cli::Prompt::SubscriberIdentifier ofx::cli::Prompt::subscribe(const std::string &command, Args &...args)
{
	return subscribe(command, [&args...](Args... argv) {
		std::tie(args...) = std::make_tuple(argv...);
	}, std::make_tuple(args...));
}
