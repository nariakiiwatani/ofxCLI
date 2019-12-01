#pragma once

#include "ofEvents.h"
#include "Testate.h"
#include <unordered_map>
#include <map>
#include "json.hpp"

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
	using SubscriberIdentifier = std::size_t;
	template<typename... Args>
	SubscriberIdentifier subscribe(const std::string &command, std::function<void(Args...)> callback);
	bool unsubscribe(SubscriberIdentifier identifier);
protected:
	void keyPressed(ofKeyEventArgs &key);
	void keyReleased(ofKeyEventArgs &key);
	void proc(const std::string &command);
	LineEditor editor_;
	std::deque<std::string> history_;
	std::unordered_multimap<std::string, SubscriberIdentifier> identifier_;
	std::map<SubscriberIdentifier, std::function<void(std::vector<std::string>)>> callback_;
	
	struct SpecialKeys {
		bool shift=false;
		bool alt=false;
		bool control=false;
		bool command=false;
	} special_keys_;
	int select_length_=0;
	
	Testate testate_;
	
	SubscriberIdentifier next_identifier_;
};
}}

namespace {
	template<typename Tuple, typename F, std::size_t ...I>
	void apply_impl(F &&f, const std::vector<std::string> &arr, nlohmann::detail::index_sequence<I...>) {
		f(ofFromString<typename std::tuple_element<I,Tuple>::type>(arr[I])...);
	}
	template<typename... Args>
	void apply(const std::function<void(Args...)> &f, const std::vector<std::string> &arr) {
		apply_impl<std::tuple<Args...>>(f, arr, nlohmann::detail::make_index_sequence<sizeof...(Args)>{});
	}
}



template<typename... Args>
inline ofx::cli::Prompt::SubscriberIdentifier ofx::cli::Prompt::subscribe(const std::string &command, std::function<void(Args...)> callback)
{
	auto ret = next_identifier_++;
	identifier_.insert(std::make_pair(command, ret));
	auto func = [callback](std::vector<std::string> args) {
		args.resize(sizeof...(Args));
		apply<Args...>(callback, args);
	};
	callback_.insert(make_pair(ret, func));
	return ret;
}
