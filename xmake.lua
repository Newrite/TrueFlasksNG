-- set minimum xmake version
set_xmakever("3.0.5")

-- includes
includes(os.getenv("CommonLibSSE-NG"))
add_requires("glaze")

-- set project
set_project("TrueFlasksNG")
set_version("1.2.6")
set_license("GPL-3.0")

-- set defaults
set_languages("c++latest")
set_warnings("allextra", "error")
set_defaultmode("releasedbg")

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set policies
set_policy("package.requires_lock", true)
set_policy("check.auto_ignore_flags", false)

-- set configs
set_config("skyrim_vr", true)
set_config("skyrim_ae", true)
set_config("skyrim_se", true)
set_config("skse_xbyak", true)

rule("prisma_ui_resources")
    set_extensions(".html", ".css", ".js", ".svg", ".ttf")


-- targets
target("TrueFlasksNG")
    add_packages("glaze")

    -- add dependencies to target
    add_deps("commonlibsse-ng")


    -- add commonlibsse-ng plugin
    add_rules("commonlibsse-ng.plugin", {
        name = "True Flasks NG",
        author = "magink && newrite && zodiak",
        description = "True Flasks NG is a plugin for Skyrim SE that adds new flasks with dark souls mechanics."
    })

add_rules("prisma_ui_resources")

set_policy("build.c++.modules", true)

-- add src files
add_includedirs("src")
set_pcxxheader("src/pch.h")
add_headerfiles("src/**.h", "src/**.hpp", "src/**.html", "src/**.js", "src/**.css", "src/**.svg", "src/**.ttf")
add_files("src/**.cpp")
