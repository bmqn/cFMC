#pragma once

#include <string>
#include <string_view>
#include <cinttypes>

using Var_t = std::string;
using Val_t = uint8_t;
using Loc_t = uint8_t;

constexpr const std::string_view k_DefaultLocId = "dfl";
constexpr const std::string_view k_NewLocId     = "new";
constexpr const std::string_view k_InputLocId   = "in";
constexpr const std::string_view k_OutputLocId  = "out";

constexpr const Loc_t k_DefaultLoc = 0;
constexpr const Loc_t k_NewLoc     = 1;
constexpr const Loc_t k_InputLoc   = 2;
constexpr const Loc_t k_OutputLoc  = 3;

constexpr const size_t k_NumReservedLocs = 4;
constexpr const size_t k_NumNewLocs      = 256 - k_NumReservedLocs;
constexpr const Loc_t  k_FirstNewLoc     = k_NumReservedLocs;