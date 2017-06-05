#ifndef SDL_MIXER_ASPECT_INCLUDED_DB71B449_E4F1_458B_975D_C36A318BBAAE
#define SDL_MIXER_ASPECT_INCLUDED_DB71B449_E4F1_458B_975D_C36A318BBAAE


#ifndef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#else
#include <SDL/SDL_mixer.h>
#endif

#include <utilities/array.hpp>
#include <nil/node_controller.hpp>
#include <nil/node.hpp>
#include <vector>


namespace Nil_ext {
namespace SDLMixer_Aspect {


// ----------------------------------------------------- [ SDL Mixer Aspect ] --


struct Data
{
  lib::array<uint32_t> sample_keys;
  lib::array<Mix_Chunk*, 64>  samples;
  lib::array<uint32_t>    channels;
  
  Nil::Node_controller sample_nodes;
  Nil::Node_controller sample_player_nodes;
  
  bool initialized = false;
  
  // Debug UI //
  
  #ifndef NIMGUI
  Nil::Node dev_menu_node;
  
  bool sdl_show_info = false;
  #endif
};


// ------------------------------------------- [ SDL Mixer Aspect Interface ] --


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect);


void
events(Nil::Engine &engine, Nil::Aspect &aspect, Nil::Event_list &event_list);


void
late_think(Nil::Engine &engine, Nil::Aspect &aspect);


} // ns
} // ns


#endif // inc guard
