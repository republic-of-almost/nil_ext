#include <aspect/sdl_aspect.hpp>
#include <nil/node.hpp>
#include <nil/node_event.hpp>
#include <nil/data/data.hpp>
#include <nil/aspect.hpp>
#include <utilities/utilities.hpp>
#include <math/math.hpp>
#include <stddef.h>

#ifdef __EMSCRIPTEN__
#include <GLES2/gl2.h>
#else
#include <GL/gl3w.h>
#endif

#ifndef NIMGUI_ASPECT
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
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

  Nil::Node_controller mouse_input(Nil::Data::get_type_id(Nil::Data::Mouse{}));
  self->mouse_input_nodes = static_cast<Nil::Node_controller&&>(mouse_input);

  self->sdl_window = nullptr;
  self->sdl_gl_context = nullptr;
  self->window_node = Nil::Node(nullptr);

  aspect.data_types = 0;
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Window{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Mouse{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Keyboard{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Gamepad{});

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
    Nil::Node node = Nil::Event::node(evt);

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

    if(Nil::Data::has_keyboard(node))
    {
      Nil::Data::Keyboard kb_data{};
      Nil::Data::get(node, kb_data);

      kb_data.key_state = (uintptr_t)self->keys;
      kb_data.key_count = Nil::Data::KeyCode::COUNT;

      Nil::Data::set(node, kb_data);
    }

    if(Nil::Data::has_mouse(node))
    {
      Nil::Data::Mouse mouse{};
      Nil::Data::get(node, mouse);

      if(self->mouse_captured != mouse.capture)
      {
        self->mouse_captured = mouse.capture;

        SDL_SetRelativeMouseMode(mouse.capture ? SDL_TRUE : SDL_FALSE);
      }
    }
  }

  self->mouse_input_nodes.process(event_list);
}


#ifdef IMGUI_DEVELOPER_SUPPORT

// ----------------------------------------------------- [ SDL Aspect IMGUI ] --

namespace {

#ifndef NIMGUI_ASPECT
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
      ImGui::BeginChild("keyboard_info", ImVec2(0, 80), false);

      ImGui::BeginChild("keyboard_data_names", ImVec2(50, 0), false);

      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Text("Key:");
      ImGui::Text("Status:");

      ImGui::EndChild();

      ImGui::SameLine();

      ImGui::BeginChild("scrolling_keys", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);

      const size_t key_count = Nil::Data::KeyCode::COUNT;

      for(size_t i = 0; i < key_count; ++i)
      {
        char key_data[16];
        memset(key_data, 0, sizeof(key_data));
        sprintf(key_data, "key_state##%zu", i);

        ImGui::BeginChild(key_data, ImVec2(50,0), true);

        ImGui::Text("%s", Nil::Data::keycode_to_string(i));
        ImGui::Text("%d", self->keys[i]);

        ImGui::EndChild();

        ImGui::SameLine();
      }

      ImGui::EndChild();
      ImGui::EndChild();
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
#endif


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
        #ifdef __EMSCRIPTEN__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        #endif

        #ifdef __EMSCRIPTEN__
        const Uint32 init_flags = SDL_INIT_VIDEO;
        #else
        const Uint32 init_flags = SDL_INIT_EVERYTHING;
        #endif

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

        #ifndef __EMSCRIPTEN__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        #endif
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

          #ifndef __EMSCRIPTEN__
          gl3wInit();
          #endif
        }
      }

      Nil::Data::Graphics gfx{};
      gfx.type = Nil::Data::Graphics::OGL;

      #ifndef __EMSCRIPTEN__
      glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&gfx.major);
      glGetIntegerv(GL_MINOR_VERSION, (GLint*)&gfx.minor);
      #else
      gfx.major = 2.0;
      gfx.minor = 1.0;
      #endif

      Nil::Data::set(self->window_node, gfx);

      #ifndef NIMGUI_ASPECT
      ImGui_ImplSdlGL3_Init(window);
      ImGui_ImplSdlGL3_NewFrame(window);
      ImGuizmo::BeginFrame();
      #endif

      #ifndef NIMGUI_ASPECT
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

  // Update nodes
  /*
    Timing bug here, if you try setting capture before this aspect, it gets overwritten
    We need to do something about this or we shall get more of these timing things.
  */
  {
    LOG_TODO_ONCE("Data last set thing");

    Nil::Node_list input_nodes = self->mouse_input_nodes.all();

    for(Nil::Node &node : input_nodes)
    {
      if(Nil::Data::has_mouse(node))
      {
        Nil::Data::Mouse data;
        Nil::Data::get(node, data);

        data.delta[0] = self->mouse_delta[0];
        data.delta[1] = self->mouse_delta[1];

        data.position[0] = self->mouse[0];
        data.position[1] = self->mouse[1];

        data.capture = self->mouse_captured;

        Nil::Data::set(node, data);
      }
    }
  }
}


namespace {


size_t
sdl_key_to_nil(const size_t sdl_key)
{
  switch(sdl_key)
  {
    // Alpha
    case(SDL_SCANCODE_A): return Nil::Data::KeyCode::A;
    case(SDL_SCANCODE_B): return Nil::Data::KeyCode::B;
    case(SDL_SCANCODE_C): return Nil::Data::KeyCode::C;
    case(SDL_SCANCODE_D): return Nil::Data::KeyCode::D;
    case(SDL_SCANCODE_E): return Nil::Data::KeyCode::E;
    case(SDL_SCANCODE_F): return Nil::Data::KeyCode::F;
    case(SDL_SCANCODE_G): return Nil::Data::KeyCode::G;
    case(SDL_SCANCODE_H): return Nil::Data::KeyCode::H;
    case(SDL_SCANCODE_I): return Nil::Data::KeyCode::I;
    case(SDL_SCANCODE_J): return Nil::Data::KeyCode::J;
    case(SDL_SCANCODE_K): return Nil::Data::KeyCode::K;
    case(SDL_SCANCODE_L): return Nil::Data::KeyCode::L;
    case(SDL_SCANCODE_M): return Nil::Data::KeyCode::M;
    case(SDL_SCANCODE_N): return Nil::Data::KeyCode::N;
    case(SDL_SCANCODE_O): return Nil::Data::KeyCode::O;
    case(SDL_SCANCODE_P): return Nil::Data::KeyCode::P;
    case(SDL_SCANCODE_Q): return Nil::Data::KeyCode::Q;
    case(SDL_SCANCODE_R): return Nil::Data::KeyCode::R;
    case(SDL_SCANCODE_S): return Nil::Data::KeyCode::S;
    case(SDL_SCANCODE_T): return Nil::Data::KeyCode::T;
    case(SDL_SCANCODE_U): return Nil::Data::KeyCode::U;
    case(SDL_SCANCODE_V): return Nil::Data::KeyCode::V;
    case(SDL_SCANCODE_W): return Nil::Data::KeyCode::W;
    case(SDL_SCANCODE_X): return Nil::Data::KeyCode::X;
    case(SDL_SCANCODE_Y): return Nil::Data::KeyCode::Y;
    case(SDL_SCANCODE_Z): return Nil::Data::KeyCode::Z;

    // Numbers
    case(SDL_SCANCODE_0): return Nil::Data::KeyCode::ZERO;
    case(SDL_SCANCODE_1): return Nil::Data::KeyCode::ONE;
    case(SDL_SCANCODE_2): return Nil::Data::KeyCode::TWO;
    case(SDL_SCANCODE_3): return Nil::Data::KeyCode::THREE;
    case(SDL_SCANCODE_4): return Nil::Data::KeyCode::FOUR;
    case(SDL_SCANCODE_5): return Nil::Data::KeyCode::FIVE;
    case(SDL_SCANCODE_6): return Nil::Data::KeyCode::SIX;
    case(SDL_SCANCODE_7): return Nil::Data::KeyCode::SEVEN;
    case(SDL_SCANCODE_8): return Nil::Data::KeyCode::EIGHT;
    case(SDL_SCANCODE_9): return Nil::Data::KeyCode::NINE;

    // Arrows
    case(SDL_SCANCODE_UP):    return Nil::Data::KeyCode::UP;
    case(SDL_SCANCODE_DOWN):  return Nil::Data::KeyCode::DOWN;
    case(SDL_SCANCODE_LEFT):  return Nil::Data::KeyCode::LEFT;
    case(SDL_SCANCODE_RIGHT): return Nil::Data::KeyCode::RIGHT;

    // Function Keys
    case(SDL_SCANCODE_F1):  return Nil::Data::KeyCode::F1;
    case(SDL_SCANCODE_F2):  return Nil::Data::KeyCode::F2;
    case(SDL_SCANCODE_F3):  return Nil::Data::KeyCode::F3;
    case(SDL_SCANCODE_F4):  return Nil::Data::KeyCode::F4;
    case(SDL_SCANCODE_F5):  return Nil::Data::KeyCode::F5;
    case(SDL_SCANCODE_F6):  return Nil::Data::KeyCode::F6;
    case(SDL_SCANCODE_F7):  return Nil::Data::KeyCode::F7;
    case(SDL_SCANCODE_F8):  return Nil::Data::KeyCode::F8;
    case(SDL_SCANCODE_F9):  return Nil::Data::KeyCode::F9;
    case(SDL_SCANCODE_F10): return Nil::Data::KeyCode::F10;
    case(SDL_SCANCODE_F11): return Nil::Data::KeyCode::F11;
    case(SDL_SCANCODE_F12): return Nil::Data::KeyCode::F12;
    case(SDL_SCANCODE_F13): return Nil::Data::KeyCode::F13;
    case(SDL_SCANCODE_F14): return Nil::Data::KeyCode::F14;
    case(SDL_SCANCODE_F15): return Nil::Data::KeyCode::F15;
    case(SDL_SCANCODE_F16): return Nil::Data::KeyCode::F16;
    case(SDL_SCANCODE_F17): return Nil::Data::KeyCode::F17;
    case(SDL_SCANCODE_F18): return Nil::Data::KeyCode::F18;
    case(SDL_SCANCODE_F19): return Nil::Data::KeyCode::F19;

    // Modifiers
    case(SDL_SCANCODE_LSHIFT):   return Nil::Data::KeyCode::L_SHIFT;
    case(SDL_SCANCODE_RSHIFT):   return Nil::Data::KeyCode::R_SHIFT;
    case(SDL_SCANCODE_LCTRL):    return Nil::Data::KeyCode::L_CTRL;
    case(SDL_SCANCODE_RCTRL):    return Nil::Data::KeyCode::R_CTRL;
    case(SDL_SCANCODE_CAPSLOCK): return Nil::Data::KeyCode::CAPS_LOCK;

    // Other
    case(SDL_SCANCODE_SPACE):    return Nil::Data::KeyCode::SPACE;
    case(SDL_SCANCODE_KP_ENTER): return Nil::Data::KeyCode::ENTER;
    case(SDL_SCANCODE_RETURN):   return Nil::Data::KeyCode::RETURN;
    case(SDL_SCANCODE_RETURN2):  return Nil::Data::KeyCode::RETURN;
    case(SDL_SCANCODE_ESCAPE):   return Nil::Data::KeyCode::ESCAPE;


    default: return -1;
  }
}
} // anon ns


void
late_think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  // Reset Input
  {
    memset(self->mouse_delta, 0, sizeof(Nil_ext::SDL_Aspect::Data::mouse));
  }

  // Update keyboard
  {
    for(uint32_t &key : self->keys)
    {
      if(key == Nil::Data::KeyState::DOWN_ON_FRAME)
      {
        key = Nil::Data::KeyState::DOWN;
      }
      else if(key == Nil::Data::KeyState::UP_ON_FRAME)
      {
        key = Nil::Data::KeyState::UP;
      }
    }
  }

  if(self->sdl_window)
  {
    // Flip Buffers
    #ifndef NIMGUI_ASPECT
    ImGui::Render();
    #endif
    
    SDL_GL_SwapWindow(self->sdl_window);

    // -- Reset ImGui -- //
    #ifndef NIMGUI_ASPECT
    ImGui_ImplSdlGL3_NewFrame(self->sdl_window);
    ImGuizmo::BeginFrame();
    #endif

    // Pump the messages
    if(self->sdl_window)
    {
//      SDL_PumpEvents();
    
      SDL_Event evt;
      
      while (SDL_PollEvent(&evt))
      {
        #ifndef NIMGUI_ASPECT
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
          } // SDL_MOUSEBUTTONUP

          case(SDL_KEYDOWN):
          {
            if(evt.key.repeat == 0)
            {
              const size_t nil_key = sdl_key_to_nil(evt.key.keysym.scancode);

              if(nil_key < Nil::Data::KeyCode::COUNT)
              {
                self->keys[nil_key] = Nil::Data::KeyState::DOWN_ON_FRAME;
              }
            }

            break;
          } // SDL_KEYDOWN

          case(SDL_KEYUP):
          {
            if(evt.key.repeat == 0)
            {
              const size_t nil_key = sdl_key_to_nil(evt.key.keysym.scancode);

              if(nil_key < Nil::Data::KeyCode::COUNT)
              {
                self->keys[nil_key] = Nil::Data::KeyState::UP_ON_FRAME;
              }
            }

            break;
          } // SDL_KEYUP

        }
      }
    }
  }
}


} // ns
} // ns
