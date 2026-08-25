#pragma once

#pragma execution_character_set("utf-8")

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <SKSE/API.h>
#include <ranges>
#include <glaze/glaze.hpp>

namespace logger = SKSE::log;
namespace stl = SKSE::stl;
using namespace std::literals;

// Static-only helper for grouping related items without allowing instantiation.
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

// Picks the runtime-appropriate offset. This used to be an #ifdef SKYRIM_AE, but that
// macro is never defined -- the build sets ENABLE_SKYRIM_AE -- so the se branch was
// silently taken on every runtime. A single NG dll loads on SE, AE and VR alike, so
// the choice has to be made at runtime. REL::Relocate expands to a switch on
// Module::GetRuntime() in a multi-runtime build, and to a constexpr return in an
// exclusive one.
//
// VR takes the se value, which is exactly what happened before. The one place VR
// really diverges is dispatched explicitly through REL::Module::IsVR().
//
// The result is no longer constexpr in a multi-runtime build, so only use it once
// REL::Module is up -- never during static initialization.
#define RELOCATION_OFFSET(se, ae) ::REL::Relocate(se, ae, se)
