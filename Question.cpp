#include "Question.h"

#include <iostream>

Question::Option::Option(std::string key, std::string description)
:
	key(std::move(key)),
	description(std::move(description))
{
}

void Question::setText(std::string text) {
	Question::text = std::move(text);
}

Question::OptionIndex Question::addOption(std::string key, std::string description) {
	return addOption(Option(key, description));
}

Question::OptionIndex Question::addOption(Option option) {
	options.push_back(std::move(option));
	return options.size() - 1;
}

Question::OptionIndex Question::show() {
	std::cout << text << std::endl;

	for (const Option &option : options) {
		std::cout << "  " << option.key << " - " << option.description << std::endl;
	}

	while (true) {
		std::string line;
		std::getline(std::cin, line);

		for (size_t i = 0; i < options.size(); ++i) {
			if (options[i].key == line) {
				return i;
			}
		}

		std::cout << "Invalid option: " << line << std::endl;
	}
}
