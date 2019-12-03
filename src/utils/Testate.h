#pragma once

#include <functional>

class Testate
{
public:
	using Will = std::function<void()>;
	Testate(){}
	Testate(Will lastword) {
		leaveWill(lastword);
	}
	void leaveWill(Will lastword) {
		lastwords_.push_back(lastword);
	}
	void expire() { is_expired_ = true; }
	~Testate() {
		if(is_expired_) return;
		for(auto &&lastword : lastwords_) {
			lastword();
		}
	}
private:
	std::vector<Will> lastwords_;
	bool is_expired_=false;
};

