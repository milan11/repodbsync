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

private:
	typedef std::vector<Option> Options;

public:
	typedef Options::size_type OptionIndex;

	void setText(std::string text);
	OptionIndex addOption(std::string key, std::string description);
	OptionIndex addOption(Option option);

	OptionIndex show();

private:
	std::string text;

	Options options;

};
