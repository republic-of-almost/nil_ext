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
        if(ImGui::DragFloat3("Position##Tra", trans.position)) { update_transform = true; }
        if(ImGui::DragFloat3("Scale##Tra",    trans.scale))    { update_transform = true; }
        if(ImGui::DragFloat4("Rotation##Tra", trans.rotation)) { update_transform = true; }

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
        if(ImGui::DragFloat3("Min##BB", box.min)) { update_bounding_box = true; }
        if(ImGui::DragFloat3("Max##BB", box.max)) { update_bounding_box = true; }
        
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
        if(ImGui::DragFloat("FOV##Cam",  &cam.fov))                   { update_cam = true; }
        if(ImGui::DragFloat("Near Plane##Cam",  &cam.near_plane))     { update_cam = true; }
        if(ImGui::DragFloat("Far Plane##Cam",  &cam.far_plane))       { update_cam = true; }
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
        ImGui::Text("No UI Impl");
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
        ImGui::Text("No UI Impl");
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
