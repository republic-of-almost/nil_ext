/*
  SDL_Asepct create a window.
  and OpenGL context.
*/
#ifndef SDL_ASPECT_INCLUDED_F108C491_0859_490B_A447_A11A6BF860AC
#define SDL_ASPECT_INCLUDED_F108C491_0859_490B_A447_A11A6BF860AC


#include <nil/fwd.hpp>
#include <nil/node.hpp>
#include <SDL2/SDL.h>


// ---------------------------------------------------- [ SDL Aspect Config ] --


#define SDL_ASPECT_IMGUI_SUPPORT
#define SDL_ASPECT_IMGUIZMO_SUPPORT
#define IMGUI_DEVELOPER_SUPPORT


namespace Nil_ext {
namespace SDL_Aspect {


// ------------------------------------------------------ [ SDL Aspect Data ] --


struct Data
{
  // Window GL State

  SDL_Window *sdl_window;
  SDL_GLContext sdl_gl_context;
  
  Nil::Node window_node;
  
  // Input State
  
  int32_t mouse_delta[2];
  int32_t mouse[2];
  
  // Debug UI
  
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

/*
  Implimentation.
*/
#ifdef SDL_ASPECT_IMPL

#ifndef SDL_ASPECT_IMPL_5D76166D_3BF0_4692_B623_E8DFA5EB8599
#define SDL_ASPECT_IMPL_5D76166D_3BF0_4692_B623_E8DFA5EB8599


#include <nil/node.hpp>
#include <nil/node_event.hpp>
#include <nil/data/data.hpp>
#include <nil/aspect.hpp>
#include <utilities/utilities.hpp>
#include <math/math.hpp>
#include <GL/gl3w.h>
#include <stddef.h>

#ifdef SDL_ASPECT_IMGUI_SUPPORT
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
#endif

#ifdef SDL_ASPECT_IMGUIZMO_SUPPORT
#include <imguizmo/ImGuizmo.h>
#endif


namespace {

constexpr char err_msg_context_setup_fail[] = "SDL Context Failed to initialize";
constexpr char err_msg_opengl_setup_fail[] = "OGL Context Failed to initialize";

// Settings //
constexpr Uint32 fullscreen_mode  = SDL_WINDOW_FULLSCREEN;
  
} // anon ns


namespace Nil_ext {
namespace SDL_Aspect {


// ------------------------------------------------------ [ SDL Aspect Impl ] --


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect)
{
  LOG_INFO("Startup SDL Aspect");

  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);
  
  self->sdl_window = nullptr;
  self->sdl_gl_context = nullptr;
  self->window_node = Nil::Node(nullptr);
  
  aspect.data_types = 0;
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Window{});
  
  #ifdef IMGUI_DEVELOPER_SUPPORT
  self->sdl_show_info = false;
  self->sdl_show_settings = false;
  self->sdl_show_raw_input = false;
  #endif
}


void
events(Nil::Engine &engine, Nil::Aspect &aspect, Nil::Event_list &event_list)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  Nil::Event_data evt;

  while(event_list.get(evt))
  {
    const Nil::Node node = Nil::Event::node(evt);
  
    if(Nil::Data::has_window(node))
    {
      const bool valid_window_node = self->window_node;
      const bool updating_current  = self->window_node == node;
      
      if(!valid_window_node || updating_current)
      {
        self->window_node = node;
      }
      else
      {
        LOG_ERROR("SDL Aspect only supports one node with a window.")
      }
    }
  }
}


#ifdef IMGUI_DEVELOPER_SUPPORT

// ----------------------------------------------------- [ SDL Aspect IMGUI ] --

namespace {


inline void
sdl_aspect_debug_menu(uintptr_t user_data)
{
  Nil_ext::SDL_Aspect::Data *self = reinterpret_cast<Nil_ext::SDL_Aspect::Data*>(user_data);
  LIB_ASSERT(self);

  if(ImGui::BeginMenu("SDL"))
  {
    ImGui::MenuItem("Info", nullptr, &self->sdl_show_info);
    ImGui::MenuItem("Settings", nullptr, &self->sdl_show_settings);
    ImGui::MenuItem("Raw Input", nullptr, &self->sdl_show_raw_input);
    
    ImGui::EndMenu();
  }
}


inline void
sdl_aspect_debug_window(uintptr_t user_data)
{
  Nil_ext::SDL_Aspect::Data *self = reinterpret_cast<Nil_ext::SDL_Aspect::Data*>(user_data);
  LIB_ASSERT(self);

  if(self->sdl_show_info)
  {
    ImGui::Begin("SDL Info", &self->sdl_show_info);
    
    ImGui::Text("Platform: %s", SDL_GetPlatform());
    SDL_version ver;
    SDL_GetVersion(&ver);
    
    ImGui::Text("SDL Linked Version: %d.%d.%d", ver.major, ver.minor, ver.patch);

    SDL_VERSION(&ver);
    
    ImGui::Text("SDL Compiled Version: %d.%d.%d", ver.major, ver.minor, ver.patch);
    
    if(ImGui::CollapsingHeader("CPU Info"))
    {
      ImGui::Text("Cache Line (Bytes): %d", SDL_GetCPUCacheLineSize());
      ImGui::Text("CPU Count: %d",          SDL_GetCPUCount());
      ImGui::Text("RAM (MB): %d",           SDL_GetSystemRAM());
    
      ImGui::Text("SSE: %s",    SDL_HasSSE()   ? "YES" : "NO");
      ImGui::Text("SSE2: %s",   SDL_HasSSE2()  ? "YES" : "NO");
      ImGui::Text("SSE3: %s",   SDL_HasSSE3()  ? "YES" : "NO");
      ImGui::Text("SSE41: %s",  SDL_HasSSE41() ? "YES" : "NO");
      ImGui::Text("SSE42: %s",  SDL_HasSSE42() ? "YES" : "NO");
    }
    
    if(ImGui::CollapsingHeader("GPU Info"))
    {
      ImGui::Text("Curr Driver: %s", SDL_GetCurrentVideoDriver());
      
      const int drivers = SDL_GetNumVideoDrivers();
      
      for(int i = 0; i < drivers; ++i)
      {
        ImGui::Text("%d : %s", i + 1, SDL_GetVideoDriver(i));
      }
    }
    
    ImGui::End();
  }
  
  if(self->sdl_show_settings)
  {
    ImGui::Begin("SDL Settings", &self->sdl_show_settings);
    
    bool v_sync = !!SDL_GL_GetSwapInterval();
    if(ImGui::Checkbox("GL VSync", &v_sync))
    {
      SDL_GL_SetSwapInterval(v_sync ? 1 : 0);
    }
    
    ImGui::End();
  }
  
  if(self->sdl_show_raw_input)
  {
    ImGui::Begin("SDL Input", &self->sdl_show_raw_input);
    
    if(ImGui::CollapsingHeader("Keyboard"))
    {
      
    }
    
    if(ImGui::CollapsingHeader("Mouse"))
    {
      ImGui::InputInt2("Position", self->mouse, ImGuiInputTextFlags_ReadOnly);
      ImGui::InputInt2("Delta", self->mouse_delta, ImGuiInputTextFlags_ReadOnly);
    }
    
    if(ImGui::CollapsingHeader("Gamepads"))
    {
      
    }
    
    ImGui::End();
  }
}


} // anon ns
#endif


void
early_think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);
  
  if(self->window_node.is_valid())
  {
    Nil::Data::Window window_data;
    Nil::Data::get(self->window_node, window_data);
    
    // If we need to create the window.
    if(self->sdl_window == nullptr)
    {
      const int32_t monitor_preference = 1;
      const char *title = strlen(window_data.title) ? window_data.title : "Nil";
      const bool v_sync = true;
      
      SDL_Window *window = nullptr;
      SDL_GLContext gl_context = nullptr;
      
      // Setup hints (Must happen before creation of window)
      {
        #ifdef __APPLE__
        // https://forums.libsdl.org/viewtopic.php?p=51127&sid=570a47d0f562cc0b1d4c91b7712c663f
        SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");
        #endif
      }

      // Create Window
      {
        const Uint32 init_flags = SDL_INIT_EVERYTHING;

        if(SDL_Init(init_flags) != 0)
        {
          LIB_ASSERT(false);
          LOG_FATAL(err_msg_context_setup_fail);
        }
        
        constexpr bool high_dpi_support   = false;

        const Uint32 window_high_dpi      = high_dpi_support ? SDL_WINDOW_ALLOW_HIGHDPI : 0;
        const Uint32 default_window_flags = window_high_dpi | SDL_WINDOW_OPENGL;
        const Uint32 window_flags         = window_data.fullscreen ? default_window_flags | fullscreen_mode : default_window_flags;
        const Uint32 display_startup      = math::min((int32_t)monitor_preference, (int32_t)SDL_GetNumVideoDisplays() - 1);
        
        SDL_DisplayMode display;
        SDL_GetCurrentDisplayMode(display_startup, &display);
        
       
        // If size == 0 we use 2/3rds of the screen.
        {
          if(window_data.width == 0)
          {
            // 2/3 rds width.
            window_data.width = 2 * (display.w / 3);
          }
          
          if(window_data.height == 0)
          {
          // 2/3 rds height.
            window_data.height = 2 * (display.h / 3);
          }
        }
        
        const Uint32 context_width  = window_data.width;
        const Uint32 context_height = window_data.height;

        window = SDL_CreateWindow(
          title,
          SDL_WINDOWPOS_CENTERED_DISPLAY(display_startup),
          SDL_WINDOWPOS_CENTERED_DISPLAY(display_startup),
          context_width,
          context_height,
          window_flags
        );

        if(!window)
        {
          LOG_FATAL(err_msg_context_setup_fail);
          LIB_ASSERT(false);
        }
        
        /*
          These values might have been filled in if letf blank
        */
        window_data.width  = context_width;
        window_data.height = context_height;
        strcpy(window_data.title, title);
        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,              8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,            8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,             8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,            8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,            16);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,          8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,          1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,    0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,    0);

        // Create context
        {
          gl_context = SDL_GL_CreateContext(window);

          if(!gl_context)
          {
            LOG_FATAL(err_msg_opengl_setup_fail);
            LIB_ASSERT(false);
          }

          SDL_GL_MakeCurrent(window, gl_context);
          SDL_GL_SetSwapInterval(v_sync ? 1 : 0); // Vsync
          
          self->sdl_window = window;
          self->sdl_gl_context = gl_context;
          
          Nil::Data::set(self->window_node, window_data);
        }
      }
      
      gl3wInit();
      
      Nil::Data::Graphics gfx{};
      gfx.type = Nil::Data::Graphics::OGL;
      
      glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&gfx.major);
      glGetIntegerv(GL_MINOR_VERSION, (GLint*)&gfx.minor);
      
      Nil::Data::set(self->window_node, gfx);
      
      #ifdef SDL_ASPECT_IMGUI_SUPPORT
      ImGui_ImplSdlGL3_Init(window);
      ImGui_ImplSdlGL3_NewFrame(window);
      #endif
      
      #ifdef SDL_ASPECT_IMGUIZMO_SUPPORT
      ImGuizmo::BeginFrame();
      #endif
      
      #ifdef IMGUI_DEVELOPER_SUPPORT
      Nil::Data::Developer dev{};
      dev.type_id = 1;
      dev.aux_01 = (uintptr_t)sdl_aspect_debug_menu;
      dev.aux_02 = (uintptr_t)self;
      
      dev.aux_03 = (uintptr_t)sdl_aspect_debug_window;
      dev.aux_04 = (uintptr_t)self;

      Nil::Data::set(self->window_node, dev);
      #endif
    }
    
    // Else resize
    else
    {
      SDL_SetWindowTitle(self->sdl_window, window_data.title);
      SDL_SetWindowSize(self->sdl_window, window_data.width, window_data.height);
      SDL_SetWindowFullscreen(self->sdl_window, window_data.fullscreen ? fullscreen_mode : 0);
    }
  }
}


void
late_think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);
  
  // Reset Input
  {
    memset(self->mouse_delta, 0, sizeof(Nil_ext::SDL_Aspect::Data::mouse));
  }

  if(self->sdl_window)
  {
    // Flip Buffers
    #ifdef SDL_ASPECT_IMGUI_SUPPORT
    ImGui::Render();
    #endif
    SDL_GL_SwapWindow(self->sdl_window);
    
    // -- Reset ImGui -- //
    #ifdef SDL_ASPECT_IMGUI_SUPPORT
    ImGui_ImplSdlGL3_NewFrame(self->sdl_window);
    #endif
    
    #ifdef SDL_ASPECT_IMGUIZMO_SUPPORT
    ImGuizmo::BeginFrame();
    #endif

    // Pump the messages
    if(self->sdl_window)
    {
      SDL_Event evt;
    
      while (SDL_PollEvent(&evt))
      {
        #ifdef SDL_ASPECT_IMGUI_SUPPORT
        if(ImGui_ImplSdlGL3_ProcessEvent(&evt))
        {
          // Need to figure out how to selectivly swallow events.
          //continue;
        }
        #endif
      
        switch(evt.type)
        {
          /*
            Time to quit
          */
          case(SDL_QUIT):
            aspect.want_to_quit = true;
            break;

          /*
            Window Events
          */
          case(SDL_WINDOWEVENT):
          {
            switch(evt.window.event)
            {
              case(SDL_WINDOWEVENT_RESIZED):
              {
                LOG_TODO("This event could be called when transitioning to or from a retina monitor. Do I need to regenerate fbo's?");
                break;
              }
              case(SDL_WINDOWEVENT_FOCUS_LOST):
              {
                LOG_TODO("Should button events be reset? Do We still get other events?");
                break;
              }
              case(SDL_WINDOWEVENT_FOCUS_GAINED):
              {
                LOG_TODO("Do we need to reinitialize anything here?");
                break;
              }
            }
            break;
          }
            
          /*
            Input Mouse Events
          */
          case(SDL_MOUSEMOTION):
          {
            const Uint32 mouse_id = evt.motion.which;
            
            if(mouse_id == 0)
            {
              self->mouse[0] = evt.motion.x;
              self->mouse[1] = evt.motion.y;
              self->mouse_delta[0] = evt.motion.xrel;
              self->mouse_delta[1] = evt.motion.yrel;
            }
          
            break;
          } // SDL_MOUSEMOTION
          
          case(SDL_MOUSEWHEEL):
          {
            break;
          } // SDL_MOUSEWHEEL
          
          case(SDL_MOUSEBUTTONDOWN):
          {
            const uint32_t mouse_id = evt.button.which;
            
            if(mouse_id == 0)
            {
              switch(evt.button.button)
              {
    //            case(SDL_BUTTON_LEFT):
    //              input::detail::mouse_set_button(core::mouse_button_id::left, button_state::down_on_frame);
    //              break;
    //              
    //            case(SDL_BUTTON_MIDDLE):
    //              input::detail::mouse_set_button(core::mouse_button_id::middle, button_state::down_on_frame);
    //              break;
    //              
    //            case(SDL_BUTTON_RIGHT):
    //              input::detail::mouse_set_button(core::mouse_button_id::right, button_state::down_on_frame);
    //              break;
    //              
    //            case(SDL_BUTTON_X1):
    //              input::detail::mouse_set_button(core::mouse_button_id::x1, button_state::down_on_frame);
    //              break;
    //              
    //            case(SDL_BUTTON_X2):
    //              input::detail::mouse_set_button(core::mouse_button_id::x2, button_state::down_on_frame);
    //              break;
              }
            }
            
            break;
          } // SDL_MOUSEBUTTONDOWN
          
          case(SDL_MOUSEBUTTONUP):
          {
            const uint32_t mouse_id = evt.button.which;
            
            if(mouse_id == 0)
            {
              switch(evt.button.button)
              {
    //            case(SDL_BUTTON_LEFT):
    //              input::detail::mouse_set_button(core::mouse_button_id::left, button_state::up_on_frame);
    //              break;
    //              
    //            case(SDL_BUTTON_MIDDLE):
    //              input::detail::mouse_set_button(core::mouse_button_id::middle, button_state::up_on_frame);
    //              break;
    //              
    //            case(SDL_BUTTON_RIGHT):
    //              input::detail::mouse_set_button(core::mouse_button_id::right, button_state::up_on_frame);
    //              break;
    //              
    //            case(SDL_BUTTON_X1):
    //              input::detail::mouse_set_button(core::mouse_button_id::x1, button_state::up_on_frame);
    //              break;
    //              
    //            case(SDL_BUTTON_X2):
    //              input::detail::mouse_set_button(core::mouse_button_id::x2, button_state::up_on_frame);
    //              break;
              }
            }
            
            break;
  //        } // SDL_MOUSEBUTTONUP
          
  //        case(SDL_KEYDOWN):
  //        {
  //          for(auto &keycode : keycodes)
  //          {
  //            if(keycode.sdl_key == evt.key.keysym.scancode)
  //            {
  //  //            input::detail::key_set(keycode.core_key, button_state::down_on_frame);
  //            }
  //          }
  //        
  //          break;
  //        } // SDL_KEYDOWN
  //        
  //        case(SDL_KEYUP):
  //        {
  //          for(auto &keycode : keycodes)
  //          {
  //            if(keycode.sdl_key == evt.key.keysym.scancode)
  //            {
  //  //            input::detail::key_set(keycode.core_key, button_state::up_on_frame);
  //            }
  //          }
          
  //          break;
          } // SDL_KEYUP
          
        }
      }
    }
  }
}


} // ns
} // ns


#endif // impl guard
#endif // impl request
