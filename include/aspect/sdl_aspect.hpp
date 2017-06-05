/*
  SDL_Asepct create a window.
  and OpenGL context.
*/
#ifndef SDL_ASPECT_INCLUDED_F108C491_0859_490B_A447_A11A6BF860AC
#define SDL_ASPECT_INCLUDED_F108C491_0859_490B_A447_A11A6BF860AC


#include <nil/fwd.hpp>
#include <nil/node.hpp>
#include <nil/data/data.hpp>
#include <nil/node_controller.hpp>
#include <SDL2/SDL.h>
#include <vector>


// ---------------------------------------------------- [ SDL Aspect Config ] --


#define SDL_ASPECT_IMGUI_SUPPORT
#define SDL_ASPECT_IMGUIZMO_SUPPORT
#define IMGUI_DEVELOPER_SUPPORT


namespace Nil_ext {
namespace SDL_Aspect {


// ------------------------------------------------------ [ SDL Aspect Data ] --


struct Data
{
  // Window / GL State //

  SDL_Window *sdl_window;
  SDL_GLContext sdl_gl_context;
  
  Nil::Node window_node;
  
  // Input State //
  
  int32_t mouse_delta[2];
  int32_t mouse[2];
  bool mouse_captured = false;
  
  std::vector<Nil::Node> keyboard_input_nodes;
  
  Nil::Node_controller mouse_input_nodes;
  
  uint32_t keys[Nil::Data::KeyCode::COUNT];
  
  // Debug UI //
  
  #ifdef IMGUI_DEVELOPER_SUPPORT
  bool sdl_show_info;
  bool sdl_show_raw_input;
  bool sdl_show_settings;
  #endif
};


// ------------------------------------------------- [ SDL Aspect Interface ] --


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect);


void
events(Nil::Engine &engine, Nil::Aspect &aspect, Nil::Event_list &event_list);


void
early_think(Nil::Engine &engine, Nil::Aspect &aspect);


void
late_think(Nil::Engine &engine, Nil::Aspect &aspect);


} // ns
} // ns


#endif // inc guard 
