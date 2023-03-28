#pragma once

#include <string>
#include <optional>

#include "Config.hpp"
#include "Term.hpp"

std::optional<Loc_t> getReservedLocFromId(const std::string_view &id);
std::optional<std::string> getIdFromReservedLoc(const Loc_t &loc);

std::string stringifyTerm(const Term &term, bool omitNil = true);