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

    src_files = {
      make.add_headers(make.get_proj_root()),
      make.add_src(make.get_proj_root()),
      make.add_doc(make.get_proj_root()),
    },

    inc_dirs = {
      make.get_proj_root() .. "./",
    },

    project_dependencies = {
      "Nil",
      "Utilities",
      "External",
      "ROV",
      "Math",
    },
  }
)
