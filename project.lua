#!lua

--[[
  Aspect Project Description
]]--

table.insert(
  projects,
  {
    name     = "Aspects",
    kind     = "StaticLib",
    language = "C++",
    location = make.get_proj_root(),
    no_link  = true,

    src_files = {
      -- Headers
      make.get_proj_root() .. "**.hpp",
      make.get_proj_root() .. "**.h",

      -- Doc
      make.get_proj_root() .. "**.md",
      make.get_proj_root() .. "**.txt",
    },

    inc_dirs = {
      make.get_proj_root() .. "./",
    },
  }
)
