/*
  SDL_Asepct create a window.
  and OpenGL context.
*/
#ifndef IMGUI_ASPECT_INCLUDED_76A60511_5EA9_453A_A2BF_6A7EC1E35B08
#define IMGUI_ASPECT_INCLUDED_76A60511_5EA9_453A_A2BF_6A7EC1E35B08


#include <nil/fwd.hpp>
#include <nil/node.hpp>
#include <nil/aspect.hpp>
#include <nil/data/data.hpp>


// ----------------------------------------------- [ ImGUI Aspect Interface ] --


namespace Nil_ext {


struct ImGUI_Aspect : public Nil::Aspect
{
  explicit
  ImGUI_Aspect();
  
  
  ~ImGUI_Aspect();
  
  void
  think(const float dt) override;
    
  
  // -------------------------------------------------------- [ Member Vars ] --
  
  Nil::Node m_inspector_node;
  bool m_show_graph;
  bool m_show_data;
  bool m_show_node_events;
  bool m_show_menu;
  
};


} // ns


#endif // inc guard 

/*
  Implimentation.
*/
#ifdef IMGUI_ASPECT_IMPL

#ifndef IMGUI_ASPECT_IMPL_C344ADC3_7EB9_4B5E_9B3F_D1E3627E42A7
#define IMGUI_ASPECT_IMPL_C344ADC3_7EB9_4B5E_9B3F_D1E3627E42A7


#include <nil/node_event.hpp>
#include <nil/data/window.hpp>
#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>
#include <string.h>


namespace {


  
} // anon ns


namespace Nil_ext {


ImGUI_Aspect::ImGUI_Aspect()
: Nil::Aspect()
, m_inspector_node(nullptr)
, m_show_graph(true)
, m_show_data(false)
, m_show_node_events(false)
, m_show_menu(true)
{
}


ImGUI_Aspect::~ImGUI_Aspect()
{
  
}


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


}


void
ImGUI_Aspect::think(const float dt)
{
  Nil::Node &root = get_root_node();
  
  // ----------------------------------------------------------- [ Settings ] --
  
  /*
    Render Settings
  */
  if(m_show_node_events)
  {
    ImGui::Begin("Node Events", &m_show_node_events);
    
    Nil::Engine_settings set;
    get_engine()->get_settings(set);
    
    Nil::Engine_state stat;
    get_engine()->get_state(stat);
    
    bool update_settings = false;
    
    if(ImGui::Checkbox("Pause Node Events", &set.pause_node_events)) { update_settings = true; }
    
    if(update_settings)
    {
      get_engine()->set_settings(set);
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
      Nil::Node event_node = stat.node_events[i].node;
    
      char label[32];
      sprintf(label, "%04d", event_node.get_id());
      
      if (ImGui::Selectable(label, selected == event_node.get_id(), ImGuiSelectableFlags_SpanAllColumns))
          selected = i;
      
      ImGui::NextColumn();
      
      ImGui::Text(Nil::Event::node_added(stat.node_events[i]) ? "YES" : "NO");  ImGui::NextColumn();
      ImGui::Text(Nil::Event::node_updated(stat.node_events[i]) ? "YES" : "NO");  ImGui::NextColumn();
      ImGui::Text(Nil::Event::node_moved(stat.node_events[i]) ? "YES" : "NO"); ImGui::NextColumn();
      ImGui::Text(Nil::Event::node_removed(stat.node_events[i]) ? "YES" : "NO"); ImGui::NextColumn();
    }
    ImGui::Columns(1);
    
    if(selected > -1)
    {
      m_inspector_node = stat.node_events[selected].node;
    }
    
    ImGui::End();
  }
  
  // --------------------------------------------------------------- [ Data ] --
  
  /*
    Render Data store
  */
  if(m_show_data)
  {
    ImGui::Begin("Data", &m_show_data);
    
    ImGui::Text("Data stored in the graph");
    
    Nil::Engine_state stat;
    get_engine()->get_state(stat);
    
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
  
  // -------------------------------------------------------------- [ Graph ] --
  
  /*
    Render Graph
  */
  if(m_show_graph)
  {
    ImGui::Begin("Graph", &m_show_graph);
  
    for(size_t i = 0; i < root.get_child_count(); ++i)
    {
      render_node(root.get_child(i), m_inspector_node);
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
  
  Nil::Node next_inspector_node = m_inspector_node;
  
  
  // ---------------------------------------------------------- [ Inspector ] --
  
  
  if(m_inspector_node.is_valid())
  {
    bool inspector_open = true;
    ImGui::Begin("Inspector", &inspector_open);
    
    ImGui::Text("Node Information");
    
    char name_buf[16]{0};
    strcat(name_buf, m_inspector_node.get_name());
    
    if(ImGui::InputText("Name##Node", name_buf, 16))
    {
      m_inspector_node.set_name(name_buf);
    }
    
    uint32_t node_id = m_inspector_node.get_id();
    ImGui::InputInt("ID", (int*)&node_id, 0, 0, ImGuiInputTextFlags_ReadOnly);
    
    /*
      Relationships
    */
    {
      Nil::Node parent_node = m_inspector_node.get_parent();
      
      if(parent_node.is_valid())
      {
        char parent_name[32]{0};
        strcat(parent_name, m_inspector_node.get_name());
        strcat(parent_name, "##Node");
      
        ImGui::Text("Parent:");
        ImGui::SameLine();
      
        if(ImGui::SmallButton(parent_name))
        {
          next_inspector_node = parent_node;
        }
      }
      
      const size_t child_count = m_inspector_node.get_child_count();
      
      if(child_count)
      {
        ImGui::Text("Children:");
        
        for(size_t i = 0; i < child_count; ++i)
        {
          Nil::Node child_node = m_inspector_node.get_child(i);
        
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
    if(Nil::Data::has_transform(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Transform"))
      {
        Nil::Data::Transform trans{};
        Nil::Data::get(m_inspector_node, trans);
        
        bool update_transform = false;
        if(ImGui::DragFloat3("Position##Tra", trans.position, 0.1f)) { update_transform = true; }
        if(ImGui::DragFloat3("Scale##Tra",    trans.scale, 0.1f))    { update_transform = true; }
        if(ImGui::DragFloat4("Rotation##Tra", trans.rotation, 0.1f)) { update_transform = true; }

        if(update_transform)
        {
          Nil::Data::set(m_inspector_node, trans);
        }
      }
    }
    
    
    /*
      Bounding Box
    */
    if(Nil::Data::has_bounding_box(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Bounding Box"))
      {
        Nil::Data::Bounding_box box{};
        Nil::Data::get(m_inspector_node, box);
        
        bool update_bounding_box = false;
        if(ImGui::DragFloat3("Min##BB", box.min, 0.1f)) { update_bounding_box = true; }
        if(ImGui::DragFloat3("Max##BB", box.max, 0.1f)) { update_bounding_box = true; }
        
        if(update_bounding_box)
        {
          Nil::Data::set(m_inspector_node, box);
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
    if(Nil::Data::has_camera(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Camera"))
      {
        Nil::Data::Camera cam{};
        Nil::Data::get(m_inspector_node, cam);
        
        const char *proj[]
        {
          "Perpective",
          "Orthographic",
        };
        
        bool update_cam = false;
        if(ImGui::Combo("Projection##Cam", (int*)&cam.type, proj, 2)) { update_cam = true; }
        if(ImGui::DragInt("Priority##Cam",  (int*)&cam.priority))     { update_cam = true; }
        if(ImGui::DragInt("Width##Cam",  (int*)&cam.width))           { update_cam = true; }
        if(ImGui::DragInt("Height##Cam", (int*)&cam.height))          { update_cam = true; }
        if(ImGui::DragFloat("FOV##Cam",  &cam.fov, 0.01f))                   { update_cam = true; }
        if(ImGui::DragFloat("Near Plane##Cam",  &cam.near_plane, 0.1f))     { update_cam = true; }
        if(ImGui::DragFloat("Far Plane##Cam",  &cam.far_plane, 0.1f))       { update_cam = true; }
        if(ImGui::Checkbox("Clear Color Buffer##Cam", &cam.clear_color_buffer)) { update_cam = true; }
        if(ImGui::Checkbox("Clear Depth Buffer##Cam", &cam.clear_depth_buffer)) { update_cam = true; }
        
        if(update_cam)
        {
          Nil::Data::set(m_inspector_node, cam);
        }
      }
    }
    
    
    /*
      Collider
    */
    if(Nil::Data::has_collider(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Collider"))
      {
        ImGui::Text("No UI Impl");
      }
    }
    
    /*
      Developer Data
    */
    if(Nil::Data::has_developer(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Developer"))
      {
        Nil::Data::Developer dev{};
        Nil::Data::get(m_inspector_node, dev);
        
        bool update_developer = false;
        if(ImGui::InputInt("AUX0", (int*)&dev.aux_01)) { update_developer = true; }
        if(ImGui::InputInt("AUX1", (int*)&dev.aux_02)) { update_developer = true; }
        if(ImGui::InputInt("AUX2", (int*)&dev.aux_03)) { update_developer = true; }
        if(ImGui::InputInt("AUX3", (int*)&dev.aux_04)) { update_developer = true; }
        
        if(update_developer)
        {
          Nil::Data::set(m_inspector_node, dev);
        }
      }
    }
    
    
    /*
      Gamepad
    */
    if(Nil::Data::has_gamepad(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Gamepad"))
      {
        ImGui::Text("No UI Impl");
      }
    }
    
    
    /*
      Graphics
    */
    if(Nil::Data::has_graphics(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Graphics"))
      {
        Nil::Data::Graphics gfx{};
        Nil::Data::get(m_inspector_node, gfx);
      
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
          Nil::Data::set(m_inspector_node, gfx);
        }
      }
    }
    
    
    /*
      Keyboard
    */
    if(Nil::Data::has_keyboard(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Keyboard"))
      {
        ImGui::Text("No UI Impl");
      }
    }
    
    
    /*
      Light
    */
    if(Nil::Data::has_light(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Light"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Logic
    */
    if(Nil::Data::has_logic(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Logic"))
      {
        Nil::Data::Logic logic{};
        Nil::Data::get(m_inspector_node, logic);
      
        bool update_logic = false;
        
        if(ImGui::InputInt("Type",  (int*)&logic.type))       { update_logic = true; }
        if(ImGui::InputInt("ID",    (int*)&logic.logic_id))   { update_logic = true; }
        if(ImGui::InputInt("Major", (int*)&logic.aux_01))     { update_logic = true; }
        if(ImGui::InputInt("Minor", (int*)&logic.aux_02))     { update_logic = true; }

        if(update_logic)
        {
          Nil::Data::set(m_inspector_node, logic);
        }
      }
    }
    
    
    /*
      Material
    */
    if(Nil::Data::has_material(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Material"))
      {
        Nil::Data::Material mat{};
        Nil::Data::get(m_inspector_node, mat);
        
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
          Nil::Data::set(m_inspector_node, mat);
        }
      }
    }
    
    
    /*
      Mesh
    */
    if(Nil::Data::has_mesh(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Mesh"))
      {
        ImGui::Text("No UI Impl");
      }
    }
    
    
    /*
      Mesh Resource
    */
    if(Nil::Data::has_mesh_resource(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Mesh_resource"))
      {
        Nil::Data::Mesh_resource mesh_resource{};
        Nil::Data::get(m_inspector_node, mesh_resource);
        
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
    if(Nil::Data::has_mouse(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Mouse"))
      {
        ImGui::Text("No UI Impl");
      }
    }
    
    
    /*
      Resource
    */
    if(Nil::Data::has_resource(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Resource"))
      {
        Nil::Data::Resource resource{};
        Nil::Data::get(m_inspector_node, resource);
      
        ImGui::Text("Resources are readonly atm");
        
        ImGui::InputInt("Type", (int*)&resource.type, 0, 0, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputText("Name", &resource.name[0], 64, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Ptr", (int*)&resource.data, 0, 0, ImGuiInputTextFlags_ReadOnly);
      }
    }
    
    
    /*
      Rigidbody
    */
    if(Nil::Data::has_rigidbody(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Rigidbody"))
      {
        ImGui::Text("No UI Impl");
      }
    }


    /*
      Texture
    */
    if(Nil::Data::has_texture(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Texture"))
      {
        ImGui::Text("No UI Impl");
      }
    }
    
  
    /*
      Window Data
    */
    if(Nil::Data::has_window(m_inspector_node))
    {
      if(ImGui::CollapsingHeader("Window"))
      {
        Nil::Data::Window window{};
        Nil::Data::get(m_inspector_node, window);
        
        bool update_window = false;
        if(ImGui::InputText("Window Title##Win", window.title, 32))          { update_window = true; }
        if(ImGui::DragInt("Width##Win", (int*)&window.width, 1, 0, 0xFFFF))  { update_window = true; }
        if(ImGui::DragInt("Height##Win", (int*)&window.height,1, 0, 0xFFFF)) { update_window = true; }
        if(ImGui::Checkbox("Fullscreen##Win", &window.fullscreen))           { update_window = true; }
        
        if(update_window)
        {
          Nil::Data::set(m_inspector_node, window);
        }
      }
    }
    
    // --------------------------------------------------------- [ Add Data ] --
    
    ImGui::Spacing();
    ImGui::Separator();
    
    ImGui::Text("Add Other Data");
    
    const size_t item_count = 16;
    
    const char *items[item_count] {
      "Select Data",
      "Camera",     // 1
      "Collider",   // 2
      "Developer",  // 3
      "Gamepad",    // 4
      "Graphics",   // 5
      "Keyboard",   // 6
      "Light",      // 7
      "Logic",      // 8
      "Material",   // 9
      "Mesh",       // 10
      "Mouse",      // 11
      "Resource",   // 12
      "Rigidbody",  // 13
      "Texture",    // 14
      "Window",     // 15
    };
    
    int item = 0;
    if(ImGui::Combo("Data##New", &item, items, item_count))
    {
      switch(item)
      {
        case(1):
        {
          if(!Nil::Data::has_camera(m_inspector_node))
          {
            Nil::Data::Camera data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(2):
        {
          if(!Nil::Data::has_collider(m_inspector_node))
          {
            Nil::Data::Collider data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(3):
        {
          if(!Nil::Data::has_developer(m_inspector_node))
          {
            Nil::Data::Developer data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(4):
        {
          if(!Nil::Data::has_gamepad(m_inspector_node))
          {
            Nil::Data::Gamepad data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(5):
        {
          if(!Nil::Data::has_graphics(m_inspector_node))
          {
            Nil::Data::Graphics data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(6):
        {
          if(!Nil::Data::has_keyboard(m_inspector_node))
          {
            Nil::Data::Keyboard data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(7):
        {
          if(!Nil::Data::has_light(m_inspector_node))
          {
            Nil::Data::Light data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(8):
        {
          if(!Nil::Data::has_logic(m_inspector_node))
          {
            Nil::Data::Logic data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(9):
        {
          if(!Nil::Data::has_material(m_inspector_node))
          {
            Nil::Data::Material data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(10):
        {
          if(!Nil::Data::has_mesh(m_inspector_node))
          {
            Nil::Data::Mesh data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(11):
        {
          if(!Nil::Data::has_mouse(m_inspector_node))
          {
            Nil::Data::Mouse data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(12):
        {
          if(!Nil::Data::has_resource(m_inspector_node))
          {
            Nil::Data::Resource data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(13):
        {
          if(!Nil::Data::has_rigidbody(m_inspector_node))
          {
            Nil::Data::Rigidbody data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(14):
        {
          if(!Nil::Data::has_texture(m_inspector_node))
          {
            Nil::Data::Texture data{};
            Nil::Data::set(m_inspector_node, data);
          }
          break;
        }
        case(15):
        {
          if(!Nil::Data::has_window(m_inspector_node))
          {
            Nil::Data::Window data{};
            Nil::Data::set(m_inspector_node, data);
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
        m_inspector_node.destroy();
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
    
    m_inspector_node = next_inspector_node;
    
    if(!inspector_open)
    {
      m_inspector_node = Nil::Node(nullptr);
    }
  }
  
  
  // ---------------------------------------------------------- [ Main Menu ] --
  
  
  if(m_show_menu)
  {
    ImGui::BeginMainMenuBar();
    
    if(ImGui::BeginMenu("Nil"))
    {
      ImGui::MenuItem("Graph", nullptr, &m_show_graph);
      ImGui::MenuItem("Data", nullptr, &m_show_data);
      ImGui::MenuItem("Node Events", nullptr, &m_show_node_events);
      
      ImGui::Separator();
      
      if(ImGui::MenuItem("Quit"))
      {
        set_quit_signal();
      }
      
      ImGui::EndMenu();
    }
    
    
    ImGui::EndMainMenuBar();
  }
}


} // ns


#endif // impl guard
#endif // impl request
