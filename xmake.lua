-- set minimum xmake version
set_xmakever("3.0.5")

-- includes
includes(os.getenv("CommonLibSSE-NG"))
add_requires("glaze")

-- set project
set_project("TrueFlasksNG")
set_version("1.0.0")
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
set_config("skyrim_ae", false)
set_config("skyrim_se", false)
set_config("skse_xbyak", true)

rule("prisma_ui_resources")
    set_extensions(".html", ".css", ".js", ".svg")
    

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
add_headerfiles("src/**.h", "src/**.hpp", "src/**.html", "src/**.js", "src/**.css", "src/**.svg")
add_files("src/**.cpp")

after_build(function(target)
    local copy = function(env_path, ext)
        -- env_path может содержать несколько путей через ";"
        for _, path_item in pairs(env_path:split(";")) do
            if os.exists(path_item) then
                local plugins = path.join(path_item, ext, "SKSE/Plugins")
                os.mkdir(plugins)
                os.trycp(target:targetfile(), plugins)
                os.trycp(target:symbolfile(), plugins)
            end
        end
    end

    -- Формируем относительный путь к папке dist внутри папки TrueFlasksNG
    local dist_path = path.join(os.projectdir(), "dist")

    -- 1. Очищаем папку dist
    if os.exists(dist_path) then
        os.rm(path.join(dist_path, "*")) -- Удаляем содержимое
    else
        os.mkdir(dist_path)
    end

    -- 2. Копируем скомпилированные файлы (dll/pdb) в корень dist
    copy(dist_path, "")

    -- 3. Копируем папку с UI
    local prisma_ui_views_path = path.join(dist_path, "PrismaUI", "views")
    os.mkdir(prisma_ui_views_path)

    -- Путь "src/UI/views" считается автоматически от корня проекта (projectdir)
    os.trycp(path.join(os.projectdir(), "src/UI/views/*"), prisma_ui_views_path)
end)
