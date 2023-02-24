#pragma once

#include <string>
#include <optional>

#include "Config.hpp"
#include "Term.hpp"

std::optional<Loc_t> getLocFromId(const std::string_view &id);

std::string stringifyTerm(const Term &term, bool omitNil = true);