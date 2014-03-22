#include "TextDiff.h"

#include <string>
#include <boost/optional.hpp>
#include "LinesReader.h"

TextDiff::TextDiff(const boost::filesystem::path &a, const boost::filesystem::path &b) {
	LinesReader aReader(a);
	LinesReader bReader(b);

	boost::optional<std::string> aLine;
	boost::optional<std::string> bLine;
	while (aLine = aReader.readLine(), bLine = bReader.readLine(), aLine || bLine) {
		if (aLine != bLine) {
			equal = false;
			return;
		}
	}

	equal = true;
}

bool TextDiff::areEqual() const {
	return equal;
}
