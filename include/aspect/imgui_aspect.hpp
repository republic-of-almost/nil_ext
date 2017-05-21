/*
  SDL_Asepct create a window.
  and OpenGL context.
*/
#ifndef IMGUI_ASPECT_INCLUDED_76A60511_5EA9_453A_A2BF_6A7EC1E35B08
#define IMGUI_ASPECT_INCLUDED_76A60511_5EA9_453A_A2BF_6A7EC1E35B08


#include <nil/fwd.hpp>
#include <nil/node.hpp>
#include <nil/data/data.hpp>
#include <vector>

namespace Nil_ext {
namespace ImGui_Aspect {


// -------------------------------------------------- [ ImGUI Aspect Config ] --


/*
  To hook onto UI add developer data with this type id.
  Put callbacks for Imgui
  AUX 01 uintptr_t For Aspects Debug Menu.
  AUX 02 uintptr_t For Aspects Window Render.
*/
constexpr uint32_t developer_type_id = 1;


// ---------------------------------------------------- [ ImGUI Aspect Data ] --


struct Data
{
  Nil::Node inspector_node;
  bool show_graph;
  bool show_raw_graph;
  bool show_data;
  bool show_node_events;
  bool show_menu;
  
  std::vector<Nil::Node> dev_nodes;
  std::vector<Nil::Data::Developer> dev_data;
};


// ----------------------------------------------- [ ImGUI Aspect Interface ] --


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect);


void
events(Nil::Engine &engine, Nil::Aspect &aspect, Nil::Event_list &event_list);


void
think(Nil::Engine &engine, Nil::Aspect &aspect);


} // ns
} // ns


#endif // inc guard
