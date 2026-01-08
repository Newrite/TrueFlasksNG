#pragma once

#pragma execution_character_set("utf-8")

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <SKSE/API.h>
#include <ranges>

namespace logger = SKSE::log;
namespace stl = SKSE::stl;
using namespace std::literals;

// Просто "статик" класс для удобного доступа элементов друг к другу (полезо при хуках)
#define STATIC_STRUCT(struct_name) \
struct struct_name final \
{ \
struct_name() = delete; \
~struct_name() = delete; \
struct_name(const struct_name& other) = delete; \
struct_name(struct_name&& other) noexcept = delete; \
struct_name& operator=(const struct_name& other) = delete; \
struct_name& operator=(struct_name&& other) noexcept = delete;

#define DLLEXPORT __declspec(dllexport)

#ifdef SKYRIM_AE
#define RELOCATION_OFFSET(se, ae) ae
#else
#define RELOCATION_OFFSET(se, ae) se
#endif
