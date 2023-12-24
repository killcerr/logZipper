add_rules("mode.debug", "mode.release")

target("logZipper")
    set_kind("shared")
    set_languages("c++23")
    set_symbols("debug")
    if is_os("android") then 
        add_cxflags("-static")
    end
    add_files("dllmain.cpp")
    add_files("deflate/lib/**.c")
