#ifndef SDL_MIXER_ASPECT_INCLUDED_DB71B449_E4F1_458B_975D_C36A318BBAAE
#define SDL_MIXER_ASPECT_INCLUDED_DB71B449_E4F1_458B_975D_C36A318BBAAE


#include <SDL2/SDL_mixer.h>
#include <utilities/array.hpp>
#include <nil/node_controller.hpp>


namespace Nil_ext {
namespace SDLMixer_Aspect {


// ----------------------------------------------------- [ SDL Mixer Aspect ] --


struct Data
{
  lib::array<Mix_Chunk*, 64>  samples;
  lib::array<uint32_t, 64>    channels;
  
  Nil::Node_controller sample_nodes;
  Nil::Node_controller sample_player_nodes;
  
  bool initialized = false;
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
