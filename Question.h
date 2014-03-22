#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Question {
public:
	struct Option {
		Option(std::string key, std::string description);

		std::string key;
		std::string description;
	};

	typedef uint32_t OptionIndex;

	void setText(std::string text);
	OptionIndex addOption(std::string key, std::string description);
	OptionIndex addOption(Option option);

	OptionIndex show();

private:
	std::string text;

	typedef std::vector<Option> Options;
	Options options;

};
