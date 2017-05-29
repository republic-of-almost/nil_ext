#include <aspect/aspects.hpp>


#define IMGUI_DEVELOPER_SUPPORT
//#define IMGIZMO_DEVELOPER_SUPPORT

#include <nil/aspect.hpp>
#include <nil/nil.hpp>

#include <aspect/sdl_aspect.hpp>

#ifndef NIMGUI_ASPECT
#include <aspect/imgui_aspect.hpp>
#endif

#include <aspect/renderer_aspect.hpp>
#include <aspect/logic_aspect.hpp>

#include <assert.h>
#include <stddef.h>


namespace Nil_ext {


void
load_aspects(Nil::Engine &nil_engine)
{
  /*
    SDL Aspect
    --
    Handles window/input and GL init.
  */
  static Nil_ext::SDL_Aspect::Data sdl;
  {
    Nil::Aspect sdl_aspect{};
    sdl_aspect.start_up_fn    = Nil_ext::SDL_Aspect::start_up;
    sdl_aspect.events_fn      = Nil_ext::SDL_Aspect::events;
    sdl_aspect.early_think_fn = Nil_ext::SDL_Aspect::early_think;
    sdl_aspect.late_think_fn  = Nil_ext::SDL_Aspect::late_think;
    sdl_aspect.user_data      = (uintptr_t)&sdl;

    nil_engine.add_aspect(sdl_aspect);
  }

  /*
    ROV Aspect
    --
    Renderer
  */
  static Nil_ext::ROV_Aspect::Data rov;
  {
    Nil::Aspect rov_aspect{};

    rov_aspect.start_up_fn    = Nil_ext::ROV_Aspect::start_up;
    rov_aspect.events_fn      = Nil_ext::ROV_Aspect::events;
    rov_aspect.early_think_fn = Nil_ext::ROV_Aspect::early_think;
    rov_aspect.think_fn       = Nil_ext::ROV_Aspect::think;
    rov_aspect.user_data      = (uintptr_t)&rov;

    nil_engine.add_aspect(rov_aspect);
  }

  /*
    Logic Aspect
  */
  static Nil_ext::Logic_aspect::Data logic;
  {
    Nil::Aspect logic_aspect{};

    logic_aspect.start_up_fn    = Nil_ext::Logic_aspect::start_up;
    logic_aspect.events_fn      = Nil_ext::Logic_aspect::events;
    logic_aspect.early_think_fn = Nil_ext::Logic_aspect::early_think;
    logic_aspect.think_fn       = Nil_ext::Logic_aspect::think;
    logic_aspect.user_data      = (uintptr_t)&logic;

    nil_engine.add_aspect(logic_aspect);
  }

  /*
    ImGui Aspect
    --
    Debugging UI
  */
  #ifndef NIMGUI_ASPECT
  static Nil_ext::ImGui_Aspect::Data imgui;
  {
    Nil::Aspect imgui_aspect{};

    imgui_aspect.start_up_fn = Nil_ext::ImGui_Aspect::start_up;
    imgui_aspect.events_fn   = Nil_ext::ImGui_Aspect::events;
    imgui_aspect.think_fn    = Nil_ext::ImGui_Aspect::think;
    imgui_aspect.user_data   = (uintptr_t)&imgui;

    nil_engine.add_aspect(imgui_aspect);
  }
  #endif
}


} // ns
