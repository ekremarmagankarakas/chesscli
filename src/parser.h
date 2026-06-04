#pragma once

#include <string_view>

#include "command.h"

Command Parse(std::string_view input);
