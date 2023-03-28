#pragma once

#include <string>
#include <string_view>
#include <cinttypes>

using Var_t = std::string;
using LocVar_t = std::string;

using Loc_t = std::string;
using Prim_t = uint8_t;

constexpr const std::string_view k_DefaultLoc = "dfl";
constexpr const std::string_view k_NewLoc     = "new";
constexpr const std::string_view k_InputLoc   = "in";
constexpr const std::string_view k_OutputLoc  = "out";
constexpr const std::string_view k_NullLoc    = "null";

constexpr const size_t k_NumReservedLocs = 5;
constexpr const size_t k_NumNewLocs      = 256 - k_NumReservedLocs;