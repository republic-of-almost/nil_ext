#include <aspect/sdl_mixer_aspect.hpp>
#include <nil/aspect.hpp>
#include <nil/data/data.hpp>
#include <nil/node_event.hpp>
#include <nil/node.hpp>
#include <lib/key.hpp>
#include <lib/utilities.hpp>
#include <lib/array.hpp>
#include <lib/entity.hpp>

#ifndef NIMGUI
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
#include <imguizmo/ImGuizmo.h>
#endif

#include <stdint.h>


namespace {


/*
  Callback and Data for when a channel has finished.
  Seems SDLMixer doesn't let me pass user data so need to keep this list
  global.
*/
constexpr uint32_t max_channels = 512;
lib::array<int, 32> channels_finished;


void
finished_chans(int channel)
{
  channels_finished.emplace_back(channel);
}


} // ns


// ----------------------------------------------- [ SDL Mixer Aspect IMGUI ] --


#ifndef NIMGUI


namespace {


inline void
sdl_mixer_aspect_debug_menu(uintptr_t user_data)
{
  Nil_ext::SDLMixer_Aspect::Data *self = reinterpret_cast<Nil_ext::SDLMixer_Aspect::Data*>(user_data);
  LIB_ASSERT(self);

  if(ImGui::BeginMenu("SDL_MIX"))
  {
    ImGui::MenuItem("Info", nullptr, &self->sdl_show_info);
  
    ImGui::EndMenu();
  }
}


inline void
sdl_aspect_debug_window(uintptr_t user_data)
{
  Nil_ext::SDLMixer_Aspect::Data *self = reinterpret_cast<Nil_ext::SDLMixer_Aspect::Data*>(user_data);
  LIB_ASSERT(self);

  if(self->sdl_show_info)
  {
    ImGui::Begin("Mixer Info", &self->sdl_show_info);
    
    if(ImGui::CollapsingHeader("Active Channels"))
    {
      const size_t count = self->channels.size();
      
      for(uint32_t i = 0; i < count; ++i)
      {
        const uint32_t entity = self->channels[i];
        
        if(entity)
        {
          ImGui::Text("Channel %i\n", i);
        }
      }
    }
    
    ImGui::End();
  }
}


} // ns


#endif


// ------------------------------------------------ [ SDL Mixer Aspect Impl ] --


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
  
  #ifndef NIMGUI
  {
    Nil::Data::Developer dev{};
    dev.type_id = 1;
    dev.aux_01 = (uintptr_t)sdl_mixer_aspect_debug_menu;
    dev.aux_02 = (uintptr_t)self;

    dev.aux_03 = (uintptr_t)sdl_aspect_debug_window;
    dev.aux_04 = (uintptr_t)self;

    Nil::Data::set(self->dev_menu_node, dev);
  }
  #endif
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
      
      if(init_mixer < 0 || open_audio < 0)
      {
        LOG_FATAL("Failed to initialize Audio Mixer");
        LIB_ASSERT(false);
        
        self->initialized = false;
      }
      
      const int channels = Mix_AllocateChannels(max_channels);
      
      if(channels < 0)
      {
        LOG_FATAL("Failed to allocate Channels");
        LIB_ASSERT(false);
        
        self->initialized = false;
      }
      
      self->channels.resize(1024);
      memset(&self->channels[0], 0, sizeof(uint32_t) * self->channels.size());
      
      Mix_ChannelFinished(finished_chans);
      
//      // Debugging
      Mix_Chunk *sample = Mix_LoadWAV("/Users/PhilCK/Desktop/Lua/audio/walk.wav");
      self->samples.emplace_back(sample);
      self->sample_keys.emplace_back(uint32_t{0});
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
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);
  
  /*
    Load any samples
  */
  for(auto &samp : self->sample_nodes.added())
  {
    {
      Nil::Node node = Nil::Node(samp.get_id());
    
      Nil::Data::Audio_resource data{};
      
      Nil::Data::get(node, data);
    
      Mix_Chunk *chunk = Mix_LoadWAV(data.filename);
      
      if(!chunk)
      {
        LOG_ERROR("Failed to load audio file");
        break;
      }
    
      self->samples.emplace_back(chunk);
      self->sample_keys.emplace_back(samp.get_id());
      
      data.id = samp.get_id();
      Nil::Data::set(node, data);
    }
  }
  
  /*
    Any channels that have finished playing we can clear
  */
  for(auto &chan : channels_finished)
  {
    const uint32_t chan_details = self->channels[chan];
    self->channels[chan] = 0;
    
    Nil::Node node(lib::entity::instance(chan_details));
    Nil::Data::Audio data{};
    Nil::Data::get(node, data);
    
    data.channel_id = 0;
    data.current_state = Nil::Data::Audio::STOPPED;
    Nil::Data::set(node, data);
  }
  
  channels_finished.clear();
  
  /*
    Remove data
  */
  {
    Nil::Node_list remove_players = self->sample_player_nodes.removed();
  }
  
  /*
    Update or insert new data
  */
  {
    Nil::Node_list players = self->sample_player_nodes.updated_and_added();
    
    for(auto &node : players)
    {
      LIB_ASSERT(Nil::Data::has_audio(node));
    
      Nil::Data::Audio audio{};
      Nil::Data::get(node, audio);
      
      const uint32_t curr_state = audio.current_state;
      const uint32_t req_state  = audio.request_state;
      
      if(req_state  == Nil::Data::Audio::PLAY &&
         curr_state != Nil::Data::Audio::PLAYING)
      {
        /*
          Search for an available channel
        */
        for(uint32_t i = 0; i < self->channels.size(); ++i)
        {
          if(self->channels[i] == 0)
          {
            // -- Find Sample -- //
            size_t sample = 0;
            {
              const uint32_t sample_key = audio.audio_id;
              
              if(!lib::key::linear_search(
                sample_key,
                self->sample_keys.data(),
                self->sample_keys.size(),
                &sample))
              {
                LOG_WARNING("Failed to find audio");
              }
            }
          
            // -- Volume -- //
            {
              const float volume = (float)MIX_MAX_VOLUME * audio.volume;
              Mix_Volume(i, (int)volume);
            }
            
            // -- Play Channel -- //
            {
              const int loop = audio.loop ? -1 : 0;
            
              Mix_Chunk *chunk = self->samples[sample];
              Mix_PlayChannel(i, chunk, loop);
              
              audio.channel_id = i + 1;
            }
            
            // Set the channel data.
            self->channels[i] = lib::entity::create(1, node.get_id());
            audio.current_state = Nil::Data::Audio::PLAYING;
            
            break;
          }
        }
        
        audio.request_state = Nil::Data::Audio::NO_REQ_STATE;
        Nil::Data::set(node, audio);
      }
      else if(audio.channel_id)
      {
        const float volume = (float)MIX_MAX_VOLUME * audio.volume;
        Mix_Volume(audio.channel_id - 1, volume);
      }
    }
  }
  
  /*
    Update or insert new data
  */
  {
    Nil::Node_list audio = self->sample_nodes.updated_and_added();
    
    for(auto &node : audio)
    {
      LIB_ASSERT(Nil::Data::has_audio_resource(node));
    
    }
  }
}


} // ns
} // ns
