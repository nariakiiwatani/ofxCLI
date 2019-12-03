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
	
	ofEvent<ofJson> SUBSCRIBED, UNSUBSCRIBED;

	LineEditor& getCurrent() { return editor_; }
	const std::vector<std::string>& getHistory() const { history_; }
protected:
	void keyPressed(ofKeyEventArgs &key);
	void keyReleased(ofKeyEventArgs &key);
	LineEditor editor_;
	std::deque<std::string> history_;
	std::deque<std::string>::iterator history_header_;
	std::unordered_multimap<std::string, SubscriberIdentifier> identifier_;
	std::map<SubscriberIdentifier, std::function<ofJson(std::vector<std::string>)>> callback_;
	
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
	auto ret = next_identifier_++;
	identifier_.insert(std::make_pair(command, ret));
	auto func = [callback,default_args](std::vector<std::string> argv) {
		auto args = detail::make_tuple_from_vector(argv, default_args);
		detail::apply(callback, args);
		return args;
	};
	callback_.insert(make_pair(ret, func));
	return ret;
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
