#pragma once

#include <functional>

class Testate
{
public:
	using Will = std::function<void()>;
	void leaveWill(Will lastword) {
		lastwords_.push_back(lastword);
	}
	~Testate() {
		for(auto &&lastword : lastwords_) {
			lastword();
		}
	}
private:
	std::vector<Will> lastwords_;
};

