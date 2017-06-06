#include <aspect/logic_aspect.hpp>
#include <utilities/utilities.hpp>
#include <nil/aspect.hpp>
#include <nil/data/data.hpp>
#include <nil/node_event.hpp>


#ifdef IMGUI_DEVELOPER_SUPPORT


// --------------------------------------------------- [ Logic Aspect IMGUI ] --


namespace {


#ifndef NIMGUI
inline void
logic_aspect_debug_menu(uintptr_t user_data)
{
  Nil_ext::ROV_Aspect::Data *self(
    reinterpret_cast<Nil_ext::ROV_Aspect::Data*>(user_data)
  );

  if(ImGui::BeginMenu("Logic"))
  {
    ImGui::EndMenu();
  }

  LIB_ASSERT(self);
}


inline void
logic_aspect_debug_window(uintptr_t user_data)
{
  Nil_ext::ROV_Aspect::Data *self(
    reinterpret_cast<Nil_ext::ROV_Aspect::Data*>(user_data)
  );
  LIB_ASSERT(self);
}
#endif

} // anon ns
#endif



namespace Nil_ext {
namespace Logic_aspect {


// -------------------------------------------- [ Logic Aspect Impl Startup ] --


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  aspect.data_types = 0;
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Logic{});

//  #ifdef IMGUI_DEVELOPER_SUPPORT
//  self->dev_node.set_name("Logic Dev");
//
//  Nil::Data::Developer dev{};
//  dev.type_id = 1;
//  dev.aux_01 = (uintptr_t)logic_aspect_debug_menu;
//  dev.aux_02 = (uintptr_t)self;
//
//  dev.aux_03 = (uintptr_t)logic_aspect_debug_window;
//  dev.aux_04 = (uintptr_t)self;
//
//  Nil::Data::set(self->dev_node, dev);
//  #endif
}


// --------------------------------------------- [ Logic Aspect Impl Events ] --


void
events(Nil::Engine &engine, Nil::Aspect &aspect, Nil::Event_list &event_list)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  Nil::Event_data evt;

  while(event_list.get(evt))
  {
    Nil::Node node(evt.node_id);

    if(Nil::Event::node_added(evt))
    {
      if(Nil::Data::has_logic(node))
      {
        Nil::Data::Logic data{};
        Nil::Data::get(node, data);

        if(data.logic_id == 1)
        {
          bool exists = false;

          for(auto n : self->logic_nodes)
          {
            if(n == node)
            {
              exists = true;
            }
          }

          if(!exists)
          {
            self->logic_nodes.emplace_back(node);
            self->update.emplace_back(data.think_01);
            self->user_data.emplace_back(data.user_data);
//            self->logic.emplace_back((Logic*)data.aux_01);
//            self->logic.back()->on_start();
          }
        }
      }
    }
    else if(Nil::Event::node_removed(evt))
    {
    }


  } // while event list
}


// ---------------------------------------- [ Logic Aspect Impl Early Think ] --


void
early_think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  for(size_t i = 0; i < self->logic_nodes.size(); ++i)
  {
    self->update[i](self->logic_nodes[i], self->user_data[i]);
  }
}


// ----------------------------------------- [ Logic Aspect Impl Late Think ] --


void
think(Nil::Engine &engine, Nil::Aspect &aspect)
{
//  Data *self = reinterpret_cast<Data*>(aspect.user_data);
//  LIB_ASSERT(self);
//
//  for(auto &log : self->logic)
//  {
//    log->on_think(0.16f);
//  }
}


} // ns
} // ns
