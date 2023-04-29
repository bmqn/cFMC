#pragma once

#include <string>
#include <optional>

#include "Config.hpp"
#include "Term.hpp"
#include "Machine.hpp"

bool isReservedLoc(const Loc_t& loc);
std::optional<Loc_t> getReservedLocFromId(const std::string_view &id);
std::optional<std::string> getIdFromReservedLoc(const Loc_t &loc);

std::string stringifyTerm(TermHandle_t term, bool omitNil = true);
std::string stringifyClosure(Closure_t closure, bool omitNil = true);