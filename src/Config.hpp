#pragma once

#include <string>
#include <string_view>
#include <cinttypes>

using Var_t = std::string;
using LocVar_t = std::string;

using Loc_t = std::string;
using Prim_t = uint8_t;

const Loc_t k_DefaultLoc = "dfl";
const Loc_t k_NewLoc     = "new";
const Loc_t k_InputLoc   = "in";
const Loc_t k_OutputLoc  = "out";
const Loc_t k_NullLoc    = "null";