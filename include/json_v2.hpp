#pragma once
#ifdef ELSIE_JSON_LIBRARY_VERSION
static_assert(false, "You can only use either version 1 or version 2 of the JSON library at the same time.");
#endif
#define ELSIE_JSON_LIBRARY_VERSION 2
#include "./json_v2/image.hpp"
#include "./json_v2/parse.hpp"
#include "./json_v2/generate.hpp"

