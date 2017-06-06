#ifndef NIMGUI

#include <aspect/imgui_aspect.hpp>
#include <nil/aspect.hpp>
#include <nil/data/data.hpp>
#include <nil/node_event.hpp>
#include <nil/nil.hpp>
#include <nil/data/window.hpp>
#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>
#include <string.h>
#include <lib/utilities.hpp>


namespace Nil_ext {
namespace ImGui_Aspect {


// --------------------------------------------- [ ImGUI Aspect Impl Statup ] --


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  self->inspector_node    = Nil::Node(nullptr);
  self->show_graph        = true;
  self->show_raw_graph    = false;
  self->show_data         = false;
  self->show_node_events  = false;
  self->show_menu         = true;

  // Aspects can hook into UI callbacks with developer data.
  aspect.data_types = 0;
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Developer{});
}


// --------------------------------------------- [ ImGUI Aspect Impl Events ] --


void
events(Nil::Engine &engine, Nil::Aspect &aspect, Nil::Event_list &event_list)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  Nil::Event_data evt{};

  while(event_list.get(evt))
  {
    const Nil::Node evt_node(evt.node_id);

    if(!Nil::Event::node_removed(evt))
    {
      if(Nil::Data::has_developer(evt_node))
      {
        Nil::Data::Developer data{};
        Nil::Data::get(evt_node, data);

        if(data.type_id == 1)
        {
          bool exists = false;

          for(size_t i = 0; i < self->dev_nodes.size(); ++i)
          {
            const Nil::Node &node = self->dev_nodes[i];

            if(node.get_id() == evt.node_id)
            {
              exists = true;
              self->dev_data[i] = data;
            }
          }

          if(!exists)
          {
            self->dev_nodes.emplace_back(evt_node);
            self->dev_data.emplace_back(data);

            LIB_ASSERT(self->dev_nodes.size() == self->dev_data.size());
          }
        }
      }
    } // if add / update

    else
    {
      for(size_t i = 0; i < self->dev_nodes.size(); ++i)
      {
        const Nil::Node &node = self->dev_nodes[i];

        if(node == evt_node)
        {
          self->dev_data.erase(std::begin(self->dev_data) + i);
          self->dev_nodes.erase(std::begin(self->dev_nodes) + i);
        }
      }
    } // else remove
  } // while events
}


// ---------------------------------------------- [ ImGUI Aspect Impl Think ] --


namespace {


void
render_node(const Nil::Node &node, Nil::Node &inspect)
{
  if(node.is_valid())
  {
    char name[16];
    memset(name, 0, sizeof(name));
    sprintf(name, "%s##%d", node.get_name(), node.get_id());

    const bool show_tree = ImGui::TreeNode(name);

    ImGui::SameLine(0.f);
    ImGui::Spacing();
    ImGui::SameLine(0.f);

    char insp_button[16];
    memset(insp_button, 0, sizeof(insp_button));
    sprintf(insp_button, "»##%d", node.get_id());

    const bool select_node = ImGui::SmallButton(insp_button);

    if(select_node)
    {
      inspect = node;
    }

    if(show_tree)
    {
      for(size_t i = 0; i < node.get_child_count(); ++i)
      {
        render_node(node.get_child(i), inspect);
      }

      ImGui::TreePop();
    }
  }
};


} // anon ns


void
think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  Nil::Node root = Nil::Node(0, false);

  // ----------------------------------------------------------- [ Settings ] --

  /*
    Render Settings
  */
  if(self->show_node_events)
  {
    ImGui::Begin("Node Events", &self->show_node_events);

    Nil::Engine_settings set{};
    engine.get_settings(set);

    Nil::Engine_state stat;
    engine.get_state(stat);

    bool update_settings = false;

    if(ImGui::Checkbox("Pause Node Events", &set.pause_node_events)) { update_settings = true; }

    if(update_settings)
    {
      engine.set_settings(set);
    }

    ImGui::Spacing();

    ImGui::Text("Node Events");
    ImGui::Columns(5, "pending_events"); // 4-ways, with border
    ImGui::Separator();
    ImGui::Text("Node ID");   ImGui::NextColumn();
    ImGui::Text("Added");     ImGui::NextColumn();
    ImGui::Text("Updated");   ImGui::NextColumn();
    ImGui::Text("Moved");     ImGui::NextColumn();
    ImGui::Text("Removed");   ImGui::NextColumn();

    ImGui::Separator();

    int selected = -1;

    for (int i = 0; i < stat.node_event_count; i++)
    {
      Nil::Node event_node = Nil::Node(stat.node_events[i].node_id, false);

      char label[32];
      sprintf(label, "%04d", event_node.get_id());

      if (ImGui::Selectable(label, selected == event_node.get_id(), ImGuiSelectableFlags_SpanAllColumns))
          selected = i;

      ImGui::NextColumn();

      ImGui::Text(Nil::Event::node_added(stat.node_events[i]) ? "YES" : "NO");    ImGui::NextColumn();
      ImGui::Text(Nil::Event::node_updated(stat.node_events[i]) ? "YES" : "NO");  ImGui::NextColumn();
      ImGui::Text(Nil::Event::node_moved(stat.node_events[i]) ? "YES" : "NO");    ImGui::NextColumn();
      ImGui::Text(Nil::Event::node_removed(stat.node_events[i]) ? "YES" : "NO");  ImGui::NextColumn();
    }
    ImGui::Columns(1);

    if(selected > -1)
    {
      self->inspector_node = Nil::Node(stat.node_events[selected].node_id);
    }

    ImGui::End();
  }

  // --------------------------------------------------------------- [ Data ] --

  /*
    Render Data store
  */
  if(self->show_data)
  {
    ImGui::Begin("Data", &self->show_data);

    ImGui::Text("Data stored in the graph");

    Nil::Engine_state stat;
    engine.get_state(stat);

    const float count = stat.node_count;

    char buf[16];
    sprintf(buf, "%zu", stat.node_count);

    ImGui::ProgressBar(1.f, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Node");

    // --

    sprintf(buf, "%zu", stat.bounding_box_count);

    ImGui::ProgressBar(stat.bounding_box_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Bounding_box");

    // --

    sprintf(buf, "%zu", stat.camera_count);

    ImGui::ProgressBar(stat.camera_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Camera");

    // --

    sprintf(buf, "%zu", stat.collider_count);

    ImGui::ProgressBar(stat.collider_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Collider");

    // --

    sprintf(buf, "%zu", stat.developer_count);

    ImGui::ProgressBar(stat.developer_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Developer");

    // --

    sprintf(buf, "%zu", stat.gamepad_count);

    ImGui::ProgressBar(stat.gamepad_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Gamepad");

    // --

    sprintf(buf, "%zu", stat.graphics_count);

    ImGui::ProgressBar(stat.graphics_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Graphics");

    // --

    sprintf(buf, "%zu", stat.keyboard_count);

    ImGui::ProgressBar(stat.keyboard_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Keyboards");

    // --

    sprintf(buf, "%zu", stat.light_count);

    ImGui::ProgressBar(stat.light_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Light");

    // --

    sprintf(buf, "%zu", stat.logic_count);

    ImGui::ProgressBar(stat.logic_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Logic");

    // --

    sprintf(buf, "%zu", stat.material_count);

    ImGui::ProgressBar(stat.material_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Material");

    // --

    sprintf(buf, "%zu", stat.mesh_count);

    ImGui::ProgressBar(stat.mesh_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Mesh");

    // --

    sprintf(buf, "%zu", stat.mesh_resource_count);

    ImGui::ProgressBar(stat.mesh_resource_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Mesh Resource");

    // --

    sprintf(buf, "%zu", stat.mouse_count);

    ImGui::ProgressBar(stat.mouse_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Mouse");

    // --

    sprintf(buf, "%zu", stat.resouce_count);

    ImGui::ProgressBar(stat.resouce_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Resource");

    // --

    sprintf(buf, "%zu", stat.rigidbody_count);

    ImGui::ProgressBar(stat.rigidbody_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Rigidbody");

    // --

    sprintf(buf, "%zu", stat.texture_count);

    ImGui::ProgressBar(stat.texture_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Texture");

    // --

    sprintf(buf, "%zu", stat.transform_count);

    ImGui::ProgressBar(stat.transform_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Transform");

    // --

    sprintf(buf, "%zu", stat.window_count);

    ImGui::ProgressBar(stat.window_count / count, ImVec2(0.0f,0.0f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Window");


    ImGui::End();
  }


  // ---------------------------------------------------------- [ Raw Graph ] --


  /*
    Render Raw graph data
  */
  if(self->show_raw_graph)
  {
    ImGui::Begin("Raw Graph Data", &self->show_raw_graph);

    ImGui::BeginChild("graph_data_container", ImVec2(0, 180), false);

    ImGui::BeginChild("graph_data_names", ImVec2(110, 0), false);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("ID (Parent ID):");
    ImGui::Text("Depth:");
    ImGui::Separator();
    ImGui::Text("Local Pos:");
    ImGui::Text("Local Scale:");
    ImGui::Text("Local Rot:");
    ImGui::Separator();
    ImGui::Text("World Pos:");
    ImGui::Text("World Scale:");
    ImGui::Text("World Rot:");

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);

    const size_t graph_count = engine.graph_data_count();

    for(size_t i = 0; i < graph_count; ++i)
    {
      char graph_data[16];
      memset(graph_data, 0, sizeof(graph_data));
      sprintf(graph_data, "graph##%zu", i);

      ImGui::BeginChild(graph_data, ImVec2(150,0), true);

      ImGui::Text("%d(%d)", engine.graph_data_get_ids()[i], lib::bits::upper32(engine.graph_data_details()[i]));
      ImGui::Text("%d", lib::bits::lower32(engine.graph_data_details()[i]));

      ImGui::Separator();

      math::transform l_trans = engine.graph_data_local_transforms()[i];

      ImGui::Text("%.01f, %.01f, %.1f", l_trans.position.data[0], l_trans.position.data[1], l_trans.position.data[2]);
      ImGui::Text("%.01f, %.01f, %.1f", l_trans.scale.data[0], l_trans.scale.data[1], l_trans.scale.data[2]);
      ImGui::Text("%.01f, %.01f, %.1f, %.1f", l_trans.rotation.data[0], l_trans.rotation.data[1], l_trans.rotation.data[2], l_trans.rotation.data[3]);

      ImGui::Separator();

      math::transform w_trans = engine.graph_data_world_transforms()[i];

      ImGui::Text("%.01f, %.01f, %.1f", w_trans.position.data[0], w_trans.position.data[1], w_trans.position.data[2]);
      ImGui::Text("%.01f, %.01f, %.1f", w_trans.scale.data[0], w_trans.scale.data[1], w_trans.scale.data[2]);
      ImGui::Text("%.01f, %.01f, %.1f, %.1f", w_trans.rotation.data[0], w_trans.rotation.data[1], w_trans.rotation.data[2], w_trans.rotation.data[3]);

      ImGui::EndChild();

      ImGui::SameLine();
    }

    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::End();
  }


  // -------------------------------------------------------------- [ Graph ] --

  /*
    Render Graph
  */
  if(self->show_graph)
  {
    ImGui::Begin("Graph", &self->show_graph);

    for(size_t i = 0; i < root.get_child_count(); ++i)
    {
      render_node(root.get_child(i), self->inspector_node);
    }

    static std::vector<Nil::Node> add_nodes;

    ImGui::Spacing();
    ImGui::Spacing();
    if(ImGui::SmallButton("+"))
    {
      add_nodes.emplace_back(Nil::Node());
    }

    ImGui::End();
  }

  Nil::Node next_inspector_node = self->inspector_node;


  // ---------------------------------------------------------- [ Inspector ] --


  if(self->inspector_node.is_valid())
  {
    bool inspector_open = true;
    ImGui::Begin("Inspector", &inspector_open);

    ImGui::Text("Node Information");

    char name_buf[16]{0};
    strcat(name_buf, self->inspector_node.get_name());

    if(ImGui::InputText("Name##Node", name_buf, 16))
    {
      self->inspector_node.set_name(name_buf);
    }

    uint32_t node_id = self->inspector_node.get_id();
    ImGui::InputInt("ID", (int*)&node_id, 0, 0, ImGuiInputTextFlags_ReadOnly);

    /*
      Relationships
    */
    {
      Nil::Node parent_node = self->inspector_node.get_parent();

      if(parent_node.is_valid())
      {
        char parent_name[32]{0};
        strcat(parent_name, self->inspector_node.get_parent().get_name());
        strcat(parent_name, "##Node");

        ImGui::Text("Parent:");
        ImGui::SameLine();

        if(ImGui::SmallButton(parent_name))
        {
          next_inspector_node = parent_node;
        }
      }

      const size_t child_count = self->inspector_node.get_child_count();

      if(child_count)
      {
        ImGui::Text("Children:");

        for(size_t i = 0; i < child_count; ++i)
        {
          Nil::Node child_node = self->inspector_node.get_child(i);

          ImGui::SameLine();

          char child_name[32]{0};
          strcat(child_name, child_node.get_name());
          strcat(child_name, "##Node");

          if(ImGui::SmallButton(child_name))
          {
            next_inspector_node = child_node;
          }
        }
      }
    }


    // ------------------------------------------- [ Default Inspector Data ] --

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Default Node Data");

    /*
      Transform Data
    */
    if(Nil::Data::has_transform(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Transform"))
      {
        Nil::Data::Transform trans{};
        Nil::Data::get(self->inspector_node, trans);

        bool update_transform = false;
        if(ImGui::DragFloat3("Position##Tra", trans.position, 0.1f)) { update_transform = true; }
        if(ImGui::DragFloat3("Scale##Tra",    trans.scale, 0.1f))    { update_transform = true; }
        if(ImGui::DragFloat4("Rotation##Tra", trans.rotation, 0.1f)) { update_transform = true; }

        if(update_transform)
        {
          Nil::Data::set(self->inspector_node, trans);
        }
      }
    }


    /*
      Bounding Box
    */
    if(Nil::Data::has_bounding_box(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Bounding Box"))
      {
        Nil::Data::Bounding_box box{};
        Nil::Data::get(self->inspector_node, box);

        bool update_bounding_box = false;
        if(ImGui::DragFloat3("Min##BB", box.min, 0.1f)) { update_bounding_box = true; }
        if(ImGui::DragFloat3("Max##BB", box.max, 0.1f)) { update_bounding_box = true; }

        if(update_bounding_box)
        {
          Nil::Data::set(self->inspector_node, box);
        }
      }
    }


    // --------------------------------------------- [ Other Inspector Data ] --


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Other Node Data");

    /*
      Camera
    */
    if(Nil::Data::has_camera(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Camera"))
      {
        Nil::Data::Camera cam{};
        Nil::Data::get(self->inspector_node, cam);

        const char *proj[]
        {
          "Perpective",
          "Orthographic",
        };

        bool update_cam = false;
        if(ImGui::Combo("Projection##Cam", (int*)&cam.type, proj, 2))           { update_cam = true; }
        if(ImGui::DragInt("Priority##Cam",  (int*)&cam.priority))               { update_cam = true; }
        if(ImGui::DragFloat("Width##Cam",  &cam.width, 0.01f))                  { update_cam = true; }
        if(ImGui::DragFloat("Height##Cam", &cam.height, 0.01f))                 { update_cam = true; }
        if(ImGui::DragFloat("FOV##Cam",  &cam.fov, 0.01f))                      { update_cam = true; }
        if(ImGui::DragFloat("Near Plane##Cam",  &cam.near_plane, 0.1f))         { update_cam = true; }
        if(ImGui::DragFloat("Far Plane##Cam",  &cam.far_plane, 0.1f))           { update_cam = true; }
        if(ImGui::Checkbox("Clear Color Buffer##Cam", &cam.clear_color_buffer)) { update_cam = true; }
        if(ImGui::Checkbox("Clear Depth Buffer##Cam", &cam.clear_depth_buffer)) { update_cam = true; }

        if(update_cam)
        {
          Nil::Data::set(self->inspector_node, cam);
        }
      }
    }

    /*
      Audio
    */
    if(Nil::Data::has_audio(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Audio"))
      {
        Nil::Data::Audio audio{};
        Nil::Data::get(self->inspector_node, audio);
        
        bool update_audio = false;
        if(ImGui::InputInt("ID##AudioPlayer", (int*)&audio.audio_id)) { update_audio = true; }
        
        const char *request[] {
          "NONE",
          "PLAY",
          "STOP",
        };
        
        if(ImGui::Combo("Request##AudioPlayer", (int*)&audio.request_state, request, sizeof(request) / sizeof(char*)))
        {
          update_audio = true;
        }
        
        const char *state[] {
          "NONE",
          "PLAYING",
          "STOPPED",
        };

        if(ImGui::Combo("Current State##AudioPlayer", (int*)&audio.current_state, state, sizeof(state) / sizeof(char*)))
        {
          update_audio = true;
        }
        
        if(ImGui::SliderFloat("Volume##AudioPlayer", &audio.volume, 0.f, 1.f)) { update_audio = true; }
        if(ImGui::Checkbox("Looping##AudioPlayer", &audio.loop))               { update_audio = true; }
        
        // Update the Data //
        if(update_audio)
        {
          Nil::Data::set(self->inspector_node, audio);
        }
      }
    }


    /*
      Audio Resource
    */
    if(Nil::Data::has_audio_resource(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Audio Resource"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Collider
    */
    if(Nil::Data::has_collider(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Collider"))
      {
        ImGui::Text("No UI Impl");
      }
    }

    /*
      Developer Data
    */
    if(Nil::Data::has_developer(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Developer"))
      {
        Nil::Data::Developer dev{};
        Nil::Data::get(self->inspector_node, dev);

        bool update_developer = false;
        if(ImGui::InputInt("AUX0", (int*)&dev.aux_01)) { update_developer = true; }
        if(ImGui::InputInt("AUX1", (int*)&dev.aux_02)) { update_developer = true; }
        if(ImGui::InputInt("AUX2", (int*)&dev.aux_03)) { update_developer = true; }
        if(ImGui::InputInt("AUX3", (int*)&dev.aux_04)) { update_developer = true; }

        if(update_developer)
        {
          Nil::Data::set(self->inspector_node, dev);
        }
      }
    }


    /*
      Gamepad
    */
    if(Nil::Data::has_gamepad(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Gamepad"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Graphics
    */
    if(Nil::Data::has_graphics(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Graphics"))
      {
        Nil::Data::Graphics gfx{};
        Nil::Data::get(self->inspector_node, gfx);

        bool update_gfx = false;

        const char *types []
        {
          "OpenGL",
          "DirectX",
        };

        if(ImGui::Combo("API", (int*)&gfx.type, types, 2))  { update_gfx = true; }
        if(ImGui::InputInt("Major", (int*)&gfx.major))      { update_gfx = true; }
        if(ImGui::InputInt("Minor", (int*)&gfx.minor))      { update_gfx = true; }

        if(update_gfx)
        {
          Nil::Data::set(self->inspector_node, gfx);
        }
      }
    }


    /*
      Keyboard
    */
    if(Nil::Data::has_keyboard(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Keyboard"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Light
    */
    if(Nil::Data::has_light(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Light"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Logic
    */
    if(Nil::Data::has_logic(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Logic"))
      {
        Nil::Data::Logic logic{};
        Nil::Data::get(self->inspector_node, logic);

        bool update_logic = false;

//        if(ImGui::InputInt("Type",  (int*)&logic.type))       { update_logic = true; }
//        if(ImGui::InputInt("ID",    (int*)&logic.logic_id))   { update_logic = true; }
//        if(ImGui::InputInt("Major", (int*)&logic.aux_01))     { update_logic = true; }
//        if(ImGui::InputInt("Minor", (int*)&logic.aux_02))     { update_logic = true; }

        if(update_logic)
        {
          Nil::Data::set(self->inspector_node, logic);
        }
      }
    }


    /*
      Material
    */
    if(Nil::Data::has_material(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Material"))
      {
        Nil::Data::Material mat{};
        Nil::Data::get(self->inspector_node, mat);

        bool update_mat = false;

        const char *shaders[] {
          "FULLBRIGHT",
          "LIT",
          "DIR_LIGHT",
        };

        if(ImGui::Combo("Shader", (int*)&mat.shader, shaders, sizeof(shaders) / sizeof (char*))) { update_mat = true; }

        if(ImGui::ColorEdit4("Color", mat.color))                { update_mat = true; }
        if(ImGui::InputInt("Texture 01", (int*)&mat.texture_01)) { update_mat = true; }
        if(ImGui::InputInt("Texture 02", (int*)&mat.texture_02)) { update_mat = true; }
        if(ImGui::InputInt("Texture 03", (int*)&mat.texture_03)) { update_mat = true; }
        if(ImGui::DragFloat2("UV Scale", mat.scale))             { update_mat = true; }
        if(ImGui::DragFloat2("UV Offset", mat.offset))           { update_mat = true; }

        if(update_mat)
        {
          Nil::Data::set(self->inspector_node, mat);
        }
      }
    }


    /*
      Mesh
    */
    if(Nil::Data::has_mesh(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Mesh"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Mesh Resource
    */
    if(Nil::Data::has_mesh_resource(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Mesh_resource"))
      {
        Nil::Data::Mesh_resource mesh_resource{};
        Nil::Data::get(self->inspector_node, mesh_resource);

        float columns = 3; // put position as default.

        if(mesh_resource.normal_vec3) { columns += 3.f; }
        if(mesh_resource.texture_coords_vec2) { columns += 2.f; }
        if(mesh_resource.color_vec4) { columns += 4.f; }


        float col_ratio = (1.f / columns) * 0.9f;

        ImGui::Text("Vertex Count: %zu", mesh_resource.count);

        // Positions
        if(mesh_resource.position_vec3)
        {
          ImGui::BeginGroup();
          ImGui::Text("Positions");
          ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)0), ImVec2(ImGui::GetWindowWidth() * col_ratio * 3, 200.0f), true);
          for (int line = 0; line < mesh_resource.count; line++)
          {
            int index = line * 3;
            char pos[16];
            memset(pos, 0, sizeof(pos));
            sprintf(pos, "%.2f,%.2f,%.2f", mesh_resource.position_vec3[index + 0], mesh_resource.position_vec3[index + 1], mesh_resource.position_vec3[index + 2]);
            ImGui::Text("%s", pos);
          }
          ImGui::EndChild();
          ImGui::EndGroup();
        }

        ImGui::SameLine();

        // Normals
        if(mesh_resource.normal_vec3)
        {
          ImGui::BeginGroup();
          ImGui::Text("Normals");
          ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)1), ImVec2(ImGui::GetWindowWidth() * col_ratio * 3, 200.0f), true);
          for (int line = 0; line < mesh_resource.count; line++)
          {
            int index = line * 3;
            char pos[16];
            memset(pos, 0, sizeof(pos));
            sprintf(pos, "%.2f,%.2f,%.2f", mesh_resource.normal_vec3[index + 0], mesh_resource.normal_vec3[index + 1], mesh_resource.normal_vec3[index + 2]);
            ImGui::Text("%s", pos);
          }
          ImGui::EndChild();
          ImGui::EndGroup();
        }

        ImGui::SameLine();

        // UV
        if(mesh_resource.texture_coords_vec2)
        {
          ImGui::BeginGroup();
          ImGui::Text("UVs");
          ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)2), ImVec2(ImGui::GetWindowWidth() * col_ratio * 2, 200.0f), true);
          for (int line = 0; line < mesh_resource.count; line++)
          {
            int index = line * 2;
            char pos[16];
            memset(pos, 0, sizeof(pos));
            sprintf(pos, "%.2f,%.2f", mesh_resource.texture_coords_vec2[index + 0], mesh_resource.texture_coords_vec2[index + 1]);
            ImGui::Text("%s", pos);
          }
          ImGui::EndChild();
          ImGui::EndGroup();
        }

        ImGui::SameLine();

        // Colors
        if(mesh_resource.color_vec4)
        {
          ImGui::BeginGroup();
          ImGui::Text("Colors");
          ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)3), ImVec2(ImGui::GetWindowWidth() * col_ratio * 4, 200.0f), true);
          for (int line = 0; line < mesh_resource.count; line++)
          {
            int index = line * 4;
            char pos[16];
            memset(pos, 0, sizeof(pos));
            sprintf(pos, "%.2f,%.2f,%.2f,%.2f", mesh_resource.color_vec4[index + 0], mesh_resource.color_vec4[index + 1], mesh_resource.color_vec4[index + 2], mesh_resource.color_vec4[index + 3]);
            ImGui::Text("%s", pos);
          }
          ImGui::EndChild();
          ImGui::EndGroup();
          ImGui::EndChild();
          ImGui::EndGroup();
        }
      }
    }


    /*
      Mouse
    */
    if(Nil::Data::has_mouse(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Mouse"))
      {
        Nil::Data::Mouse mouse{};
        Nil::Data::get(self->inspector_node, mouse);

        constexpr ImGuiInputTextFlags_ flags = ImGuiInputTextFlags_ReadOnly;

        bool update_input = false;

        ImGui::InputInt2("Pos",   (int*)mouse.position, flags);
        ImGui::InputInt2("Delta", (int*)mouse.delta,    flags);
        if(ImGui::Checkbox("Capture", &mouse.capture)) { update_input = true; }

        if(update_input)
        {
          Nil::Data::set(self->inspector_node, mouse);
        }
      }
    }


    /*
      Resource
    */
    if(Nil::Data::has_resource(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Resource"))
      {
        Nil::Data::Resource resource{};
        Nil::Data::get(self->inspector_node, resource);

        ImGui::Text("Resources are readonly atm");

        constexpr ImGuiInputTextFlags_ flags = ImGuiInputTextFlags_ReadOnly;

        ImGui::InputInt("Type", (int*)&resource.type, 0, 0, flags);
        ImGui::InputText("Name", &resource.name[0], 64, flags);
        ImGui::InputInt("Ptr", (int*)&resource.data, 0, 0, flags);
      }
    }


    /*
      Rigidbody
    */
    if(Nil::Data::has_rigidbody(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Rigidbody"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Texture
    */
    if(Nil::Data::has_texture(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Texture"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Window Data
    */
    if(Nil::Data::has_window(self->inspector_node))
    {
      if(ImGui::CollapsingHeader("Window"))
      {
        Nil::Data::Window window{};
        Nil::Data::get(self->inspector_node, window);

        bool update_window = false;
        if(ImGui::InputText("Window Title##Win", window.title, 32))          { update_window = true; }
        if(ImGui::DragInt("Width##Win", (int*)&window.width, 1, 0, 0xFFFF))  { update_window = true; }
        if(ImGui::DragInt("Height##Win", (int*)&window.height,1, 0, 0xFFFF)) { update_window = true; }
        if(ImGui::Checkbox("Fullscreen##Win", &window.fullscreen))           { update_window = true; }

        if(update_window)
        {
          Nil::Data::set(self->inspector_node, window);
        }
      }
    }

    // --------------------------------------------------------- [ Add Data ] --

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Add Other Data");

    const size_t item_count = 18;

    const char *items[item_count] {
      "Select Data",
      "Audio",          // 1
      "AudioResource",  // 2
      "Camera",         // 3
      "Collider",       // 4
      "Developer",      // 5
      "Gamepad",        // 6
      "Graphics",       // 7
      "Keyboard",       // 8
      "Light",          // 9
      "Logic",          // 10
      "Material",       // 11
      "Mesh",           // 12
      "Mouse",          // 13
      "Resource",       // 14
      "Rigidbody",      // 15
      "Texture",        // 16
      "Window",         // 17
    };

    int item = 0;
    if(ImGui::Combo("Data##New", &item, items, item_count))
    {
      switch(item)
      {
        case(1):
        {
          if(!Nil::Data::has_audio(self->inspector_node))
          {
            Nil::Data::Audio data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(2):
        {
          if(!Nil::Data::has_audio_resource(self->inspector_node))
          {
            Nil::Data::Audio_resource data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(3):
        {
          if(!Nil::Data::has_camera(self->inspector_node))
          {
            Nil::Data::Camera data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(4):
        {
          if(!Nil::Data::has_collider(self->inspector_node))
          {
            Nil::Data::Collider data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(5):
        {
          if(!Nil::Data::has_developer(self->inspector_node))
          {
            Nil::Data::Developer data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(6):
        {
          if(!Nil::Data::has_gamepad(self->inspector_node))
          {
            Nil::Data::Gamepad data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(7):
        {
          if(!Nil::Data::has_graphics(self->inspector_node))
          {
            Nil::Data::Graphics data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(8):
        {
          if(!Nil::Data::has_keyboard(self->inspector_node))
          {
            Nil::Data::Keyboard data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(9):
        {
          if(!Nil::Data::has_light(self->inspector_node))
          {
            Nil::Data::Light data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(10):
        {
          if(!Nil::Data::has_logic(self->inspector_node))
          {
            Nil::Data::Logic data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(11):
        {
          if(!Nil::Data::has_material(self->inspector_node))
          {
            Nil::Data::Material data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(12):
        {
          if(!Nil::Data::has_mesh(self->inspector_node))
          {
            Nil::Data::Mesh data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(13):
        {
          if(!Nil::Data::has_mouse(self->inspector_node))
          {
            Nil::Data::Mouse data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(14):
        {
          if(!Nil::Data::has_resource(self->inspector_node))
          {
            Nil::Data::Resource data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(15):
        {
          if(!Nil::Data::has_rigidbody(self->inspector_node))
          {
            Nil::Data::Rigidbody data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(16):
        {
          if(!Nil::Data::has_texture(self->inspector_node))
          {
            Nil::Data::Texture data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
        case(17):
        {
          if(!Nil::Data::has_window(self->inspector_node))
          {
            Nil::Data::Window data{};
            Nil::Data::set(self->inspector_node, data);
          }
          break;
        }
      }
    }

    // ------------------------------------------------------ [ Delete Node ] --

    ImGui::Spacing();
    ImGui::Separator();

    // Delete the node?
    if(ImGui::Button("Delete Node"))
    {
      ImGui::OpenPopup("Delete");
    }

    // Destroy Node Confirmation.
    if (ImGui::BeginPopupModal("Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
      ImGui::Text("Destroy Node?\n");
      ImGui::Separator();

      if (ImGui::Button("OK", ImVec2(120,0)))
      {
        self->inspector_node.destroy();
        ImGui::CloseCurrentPopup();
      }

      ImGui::SameLine();

      if (ImGui::Button("Cancel", ImVec2(120,0)))
      {
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }


    ImGui::End();

    self->inspector_node = next_inspector_node;

    if(!inspector_open)
    {
      self->inspector_node = Nil::Node(nullptr);
    }
  }


  // ---------------------------------------------------------- [ Main Menu ] --


  if(self->show_menu)
  {
    ImGui::BeginMainMenuBar();

    if(ImGui::BeginMenu("Nil"))
    {
      ImGui::MenuItem("Graph", nullptr, &self->show_graph);
      ImGui::MenuItem("Graph-Raw", nullptr, &self->show_raw_graph);
      ImGui::MenuItem("Data", nullptr, &self->show_data);
      ImGui::MenuItem("Node Events", nullptr, &self->show_node_events);

      ImGui::Separator();

      if(ImGui::MenuItem("Quit"))
      {
        aspect.want_to_quit = true;
      }

      ImGui::EndMenu();
    }

    for(auto &menu : self->dev_data)
    {
      using fn = void(*)(uintptr_t user_data);

      ((fn)menu.aux_01)(menu.aux_02);
    }

    ImGui::EndMainMenuBar();
  }


  // --------------------------------------------------------- [ Other Menu ] --

  for(auto &menu : self->dev_data)
  {
    using fn = void(*)(uintptr_t user_data);
    
    if(menu.aux_03)
    {
      ((fn)menu.aux_03)(menu.aux_04);
    }
  }

}


} // ns
} // ns


#endif
