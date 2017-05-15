#ifndef LOGIC_ASPECT_INCLUDED_6E5D2235_11A4_41DE_B659_4567CD7DBFD1
#define LOGIC_ASPECT_INCLUDED_6E5D2235_11A4_41DE_B659_4567CD7DBFD1


#include <nil/fwd.hpp>
#include <nil/node.hpp>
#include <math/fwd.hpp>
#include <vector>


namespace Nil_ext {

class Logic
{
public:

  explicit Logic() {}
  virtual ~Logic() {}

  virtual void on_start() {}
  
  virtual void on_early_think(const float dt) {}
  virtual void on_think(const float dt) {}
  virtual void on_late_think(const float dt) {}
  
  virtual void on_end() {}


};


namespace Logic_aspect {


// ---------------------------------------------------- [ Logic Aspect Data ] --


struct Data
{
  std::vector<Nil::Node> pending_removals;
  std::vector<Nil::Node> logic_nodes;

  std::vector<Logic*> logic;

  #ifdef IMGUI_DEVELOPER_SUPPORT
  Nil::Node dev_node;
  #endif
};


// ----------------------------------------------- [ Logic Aspect Interface ] --


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect);


void
events(Nil::Engine &engine, Nil::Aspect &aspect, Nil::Event_list &event_list);


void
early_think(Nil::Engine &engine, Nil::Aspect &aspect);


void
think(Nil::Engine &engine, Nil::Aspect &aspect);


} // ns
} // ns


#endif // inc guard


/*
  Implimentation.
*/
#ifdef LOGIC_ASPECT_IMPL

#ifndef LOGIC_ASPECT_IMPL_8975A214_6883_4A2A_96A9_A251AA16BE82
#define LOGIC_ASPECT_IMPL_8975A214_6883_4A2A_96A9_A251AA16BE82


#include <nil/data/data.hpp>


#ifdef IMGUI_DEVELOPER_SUPPORT


// --------------------------------------------------- [ Logic Aspect IMGUI ] --


namespace {


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

  #ifdef IMGUI_DEVELOPER_SUPPORT
  self->dev_node.set_name("Logic Dev");

  Nil::Data::Developer dev{};
  dev.type_id = 1;
  dev.aux_01 = (uintptr_t)logic_aspect_debug_menu;
  dev.aux_02 = (uintptr_t)self;
  
  dev.aux_03 = (uintptr_t)logic_aspect_debug_window;
  dev.aux_04 = (uintptr_t)self;

  Nil::Data::set(self->dev_node, dev);
  #endif
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
            self->logic.emplace_back((Logic*)data.aux_01);
            self->logic.back()->on_start();
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
  
  for(auto &log : self->logic)
  {
    log->on_early_think(0.16f);
  }
}


// ----------------------------------------- [ Logic Aspect Impl Late Think ] --


void
think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);
  
  for(auto &log : self->logic)
  {
    log->on_think(0.16f);
  }
}


} // ns
} // ns


#endif // impl inc
#endif // impl guard
