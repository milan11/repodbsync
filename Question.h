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
	using Options = std::vector<Option>;

public:
	using OptionIndex = Options::size_type;

	void setText(std::string text);
	OptionIndex addOption(std::string key, std::string description);
	OptionIndex addOption(Option option);

	OptionIndex show();

private:
	std::string text;

	Options options;

};
