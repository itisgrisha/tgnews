#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "udpipe.h"

namespace udpipe = ufal::udpipe;
using pModel = std::unique_ptr<udpipe::model>;
using pReader = std::unique_ptr<udpipe::input_format>;
using BOWDict = std::unordered_map<std::string, size_t>;
