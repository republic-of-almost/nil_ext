#include <aspect/sdl_mixer_aspect.hpp>
#include <utilities/utilities.hpp>
#include <nil/aspect.hpp>
#include <nil/data/data.hpp>
#include <nil/node_event.hpp>
#include <nil/node.hpp>


namespace {



} // ns


// ------------------------------------------------------ [ SDL Aspect Impl ] --


namespace Nil_ext {
namespace SDLMixer_Aspect {


void
start_up(
  Nil::Engine &engine,
  Nil::Aspect &aspect)
{
  LOG_INFO("Startup SDL Mixer Aspect");

  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  // Register for data types.
  {
    aspect.data_types = 0;
    aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Audio{});
    aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Audio_resource{});
    
    // only used to idicate HW is ready.
    aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Graphics{});
  }
  
  // Create Node Controllers
  {
    Nil::Node_controller players(Nil::Data::get_type_id(Nil::Data::Audio{}));
    self->sample_player_nodes = static_cast<Nil::Node_controller&&>(players);
    
    Nil::Node_controller samples(Nil::Data::get_type_id(Nil::Data::Audio_resource{}));
    self->sample_nodes = static_cast<Nil::Node_controller&&>(samples);
  }
}


void
events(
  Nil::Engine &engine,
  Nil::Aspect &aspect,
  Nil::Event_list &event_list)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);
  
  Nil::Event_data evt;
  
  while(event_list.get(evt))
  {
    Nil::Node node = Nil::Event::node(evt);
  
    // Initialize the hardware
    if(Nil::Data::has_graphics(node))
    {
      self->initialized = true;
    
      const int init_mixer = Mix_Init(MIX_INIT_OGG|MIX_INIT_MOD);
      const int open_audio = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
      
      if(init_mixer == 0 || open_audio == 0)
      {
        LOG_FATAL("Failed to initialize Audio Mixer");
        LIB_ASSERT(false);
        
        self->initialized = false;
      }
      
      const int channels = Mix_AllocateChannels(512);
      
      if(channels == 0)
      {
        LOG_FATAL("Failed to allocate Channels");
        LIB_ASSERT(false);
        
        self->initialized = false;
      }
    }
    
    
    self->sample_nodes.process(event_list);
    self->sample_player_nodes.process(event_list);
  } // while event list
}


void
late_think(
  Nil::Engine &engine,
  Nil::Aspect &aspect)
{
  /*
    Remove data
  */
  
  /*
    
  */
}


} // ns
} // ns
