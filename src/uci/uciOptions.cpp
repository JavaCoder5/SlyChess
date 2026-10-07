/*
	Copyright (C) 2026 JavaCoder5

	SlyChess is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SlyChess is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "uciOptions.h"

std::vector<UciOption> uciOptions;

static std::string optionTypeToString(UciOptionType type)
{
	std::string output;
	switch (type)
	{
	case UCI_OPTION_SPIN: output += "spin"; break;
	case UCI_OPTION_CHECK: output += "check"; break;
	case UCI_OPTION_COMBO: output += "combo"; break;
	case UCI_OPTION_BUTTON: output += "button"; break;
	case UCI_OPTION_STRING: output += "string"; break;
	}
	return output;
}

void printUciOptions()
{
	for (const auto& option : uciOptions) {
		std::cout << "option name " << option.name << " type " << optionTypeToString(option.type);
		if (option.type == UCI_OPTION_SPIN) {
			std::cout << " default " << option.defaultValue << " min " << option.minValue << " max " << option.maxValue;
		}
		else if (option.type == UCI_OPTION_CHECK) {
			std::cout << " default " << (option.defaultValue ? "true" : "false");
		}
		else if (option.type == UCI_OPTION_COMBO) {
			for (const auto& choice : option.choices) {
				std::cout << " var " << choice;
			}
		}
		else if (option.type == UCI_OPTION_STRING) {
			std::cout << " default " << option.defaultValue;
		}
		else if (option.type == UCI_OPTION_BUTTON) {
			// Buttons don't have default values or ranges
		}
		std::cout << "\n";
	}
}

void addUciOption(
	const std::string& name, 
	UciOptionType type, 
	int defaultValue, 
	int minValue, 
	int maxValue, 
	const std::vector<std::string>& choices
)
{
	uciOptions.push_back({ name, type, defaultValue, minValue, maxValue, choices, defaultValue });
}

std::string getUciOptionValue(const std::string& name)
{
	for (const auto& option : uciOptions) {
		if (option.name == name) {
			if (option.type == UCI_OPTION_STRING)
				return option.currentStringValue;
			else
				return std::to_string(option.currentValue);
		}
	}
	return ""; // Return empty string if the option is not found
}

void setUciOptionValue(const std::string& name, int value)
{
	for (auto& option : uciOptions) {
		if (option.name == name) {
			option.currentValue = value;
			return;
		}
	}
}

void setUciOptionValue(const std::string& name, const std::string& value)
{
	for (auto& option : uciOptions) {
		if (option.name == name) {
			option.currentStringValue = value;
			return;
		}
	}
}

UciOptionType getUciOptionType(const std::string& name)
{
	for (const auto& option : uciOptions) {
		if (option.name == name) {
			return option.type;
		}
	}
	return UCI_OPTION_UNKNOWN; // Return UCI_OPTION_UNKNOWN if the option is not found
}