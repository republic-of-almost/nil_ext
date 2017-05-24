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

  virtual void on_start()                     {}
  virtual void on_early_think(const float dt) {}
  virtual void on_think(const float dt)       {}
  virtual void on_late_think(const float dt)  {}
  virtual void on_end()                       {}
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
