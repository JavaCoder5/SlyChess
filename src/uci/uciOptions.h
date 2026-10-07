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

#pragma once
#include <vector>
#include <string>
#include <iostream>

enum UciOptionType {
    UCI_OPTION_CHECK,
    UCI_OPTION_SPIN,
    UCI_OPTION_COMBO,
    UCI_OPTION_BUTTON,
    UCI_OPTION_STRING,
    UCI_OPTION_UNKNOWN
};

struct UciOption
{
    std::string name;
    UciOptionType type;
    int defaultValue;
    int minValue;
    int maxValue;
    std::vector<std::string> choices; // Used for combo type
    int currentValue; // Current value of the option
                      // For check type, 0 = false, anything else = true
    std::string currentStringValue; // For string type
};

void printUciOptions();

void addUciOption(
    const std::string& name, 
    UciOptionType type, 
    int defaultValue = 0, 
    int minValue = 0, 
    int maxValue = 0, 
    const std::vector<std::string>& choices = {}
);

std::string getUciOptionValue(const std::string& name);

void setUciOptionValue(const std::string& name, int value);

void setUciOptionValue(const std::string& name, const std::string& value);

UciOptionType getUciOptionType(const std::string& name);