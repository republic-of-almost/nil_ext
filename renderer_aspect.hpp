#ifndef GL_ASPECT_INCLUDED_5D7A0FCB_0B1D_4455_9F83_E01195FBD0D7
#define GL_ASPECT_INCLUDED_5D7A0FCB_0B1D_4455_9F83_E01195FBD0D7


#include <nil/fwd.hpp>
#include <nil/node.hpp>
#include <math/math.hpp>
#include <utilities/utilities.hpp>
#include <vector>


// ----------------------------------------------- [ Renderer Aspect Config ] --


#define IMGUI_DEVELOPER_SUPPORT


namespace Nil_ext {
namespace ROV_Aspect {


// ------------------------------------------------- [ Renderer Aspect Data ] --


struct Data
{
  uint32_t current_viewport[2];

  // Meshes //

  std::vector<uint32_t> external_mesh_ids;
  std::vector<uint32_t> internal_mesh_ids;
  std::vector<Nil::Node> pending_mesh_load;
  
  // Camera //
  
  struct ROV_Camera
  {
    math::mat4 view;
    math::mat4 proj;
    
    uint32_t clear_flags;
  };
  
  std::vector<Nil::Node>  pending_camera_node_updates;
  std::vector<Nil::Node>  pending_camera_node_removals;
  std::vector<Nil::Node>  camera_nodes;
  std::vector<ROV_Camera> rov_camera;
  
  // Draw Calls //
  
  struct ROV_Renderable
  {
    uint8_t shader_type;
    math::mat4 world;
    uint32_t mesh_id;
    
    float color[4];
  };
  
  std::vector<Nil::Node>      pending_renderable_node_updates;
  std::vector<Nil::Node>      pending_renderable_node_removals;
  std::vector<Nil::Node>      renderable_nodes;
  std::vector<ROV_Renderable> renderables;
  
  bool has_initialized = false;
  bool process_pending = true;

  #ifdef IMGUI_DEVELOPER_SUPPORT
  Nil::Node dev_node;
  bool renderer_show_info;
  bool renderer_show_settings;
  #endif
};


// -------------------------------------------- [ Renderer Aspect Interface ] --


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
#ifdef RENDERER_ASPECT_IMPL

#ifndef RENDERER_ASPECT_IMPL_C4901940_F51C_496B_A821_BDE103DB8742
#define RENDERER_ASPECT_IMPL_C4901940_F51C_496B_A821_BDE103DB8742


//#include <GL/gl3w.h>
//
//#define ROV_GL_IMPL
#include <rov/rov.hpp>

#include <nil/node.hpp>
#include <nil/data/window.hpp>
#include <math_nil_extensions.hpp>

#ifdef IMGIZMO_DEVELOPER_SUPPORT
#include <imguizmo/ImGuizmo.h>
#endif

#ifdef IMGUI_DEVELOPER_SUPPORT


// ------------------------------------------------ [ Renderer Aspect IMGUI ] --


namespace {


inline void
renderer_aspect_debug_menu(uintptr_t user_data)
{
  Nil_ext::ROV_Aspect::Data *self = reinterpret_cast<Nil_ext::ROV_Aspect::Data*>(user_data);
  LIB_ASSERT(self);

  if(ImGui::BeginMenu("ROV"))
  {
    ImGui::MenuItem("Info", nullptr, &self->renderer_show_info);
    ImGui::MenuItem("Settings", nullptr, &self->renderer_show_settings);
    
    ImGui::EndMenu();
  }
}


inline void
renderer_aspect_debug_window(uintptr_t user_data)
{
  Nil_ext::ROV_Aspect::Data *self = reinterpret_cast<Nil_ext::ROV_Aspect::Data*>(user_data);
  LIB_ASSERT(self);
  
  if(self->renderer_show_info)
  {
    ImGui::Begin("Renderer Info", &self->renderer_show_info);
    
    if(ImGui::CollapsingHeader("Cameras"))
    {
      ImGui::Columns(3, "cameras");
      ImGui::Separator();
      ImGui::Text("Node ID");     ImGui::NextColumn();
      ImGui::Text("Clear Color"); ImGui::NextColumn();
      ImGui::Text("Clear Depth"); ImGui::NextColumn();
      
      ImGui::Separator();
      
      int selected = -1;
      
      for(size_t i = 0; i < self->camera_nodes.size(); ++i)
      {
        char label[32];
        sprintf(label, "%04d", self->camera_nodes[i].get_id());

        if (ImGui::Selectable(label, selected == self->camera_nodes[i].get_id(), ImGuiSelectableFlags_SpanAllColumns))
        {
          selected = (int)i;
        }
      
        ImGui::NextColumn();

        ImGui::Text("%s", self->rov_camera[i].clear_flags & rovClearFlag_Color ? "YES" : "NO"); ImGui::NextColumn();
        ImGui::Text("%s", self->rov_camera[i].clear_flags & rovClearFlag_Depth ? "YES" : "NO"); ImGui::NextColumn();
      }
      ImGui::Columns(1);
      ImGui::Separator();
      
      ImGui::Spacing();
      
      
      ImGui::Text("Pending Camera Updates: %s", self->pending_camera_node_updates.size() ? "YES" : "NO");
      ImGui::Text("Pending Camera Removals: %s", self->pending_camera_node_removals.size() ? "YES" : "NO");
    }
    
    if(ImGui::CollapsingHeader("Draw Calls"))
    {
      ImGui::Columns(1, "renderables");
      ImGui::Separator();
      ImGui::Text("Node ID");     ImGui::NextColumn();
      
      ImGui::Separator();
      
      int selected = -1;
      
      for(size_t i = 0; i < self->renderables.size(); ++i)
      {
        char label[32];
        sprintf(label, "%04d", self->renderable_nodes[i].get_id());

        if (ImGui::Selectable(label, selected == self->renderable_nodes[i].get_id(), ImGuiSelectableFlags_SpanAllColumns))
        {
          selected = (int)i;
        }
      
        ImGui::NextColumn();
      }
      ImGui::Columns(1);
      ImGui::Separator();
      
      ImGui::Spacing();
    
      ImGui::Text("Pending Renderable Updates: %s", self->pending_renderable_node_updates.size() ? "YES" : "NO");
      ImGui::Text("Pending Renderable Removals: %s", self->pending_renderable_node_removals.size() ? "YES" : "NO");
    }
    
    ImGui::End();
  }
  
  if(self->renderer_show_settings)
  {
    ImGui::Begin("Renderer Settings", &self->renderer_show_settings);
    
    ImGui::Checkbox("Process Pending Changes", &self->process_pending);
    
    ImGui::End();
  }
}


} // anon ns
#endif


namespace Nil_ext {
namespace ROV_Aspect {


// ----------------------------------------- [ Renderer Aspect Impl Startup ] --


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect)
{
  LOG_INFO("Startup Renderer Aspect")

  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);
  
  self->current_viewport[0] = 800;
  self->current_viewport[1] = 600;
  
  aspect.data_types = 0;
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Camera{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Material{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Mesh{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Texture{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Resource{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Graphics{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Mesh_resource{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Window{});
  
  #ifdef IMGUI_DEVELOPER_SUPPORT
  self->renderer_show_info     = false;
  self->renderer_show_settings = false;

  self->dev_node.set_name("Renderer Dev");

  Nil::Data::Developer dev{};
  dev.type_id = 1;
  dev.aux_01 = (uintptr_t)renderer_aspect_debug_menu;
  dev.aux_02 = (uintptr_t)self;
  
  dev.aux_03 = (uintptr_t)renderer_aspect_debug_window;
  dev.aux_04 = (uintptr_t)self;

  Nil::Data::set(self->dev_node, dev);
  #endif
}


// ------------------------------------------ [ Renderer Aspect Impl Events ] --


void
events(Nil::Engine &engine, Nil::Aspect &aspect, Nil::Event_list &event_list)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  Nil::Event_data evt;

  while(event_list.get(evt))
  {
    const Nil::Node &node = Nil::Node(evt.node_id);
    
    /*
      We add all nodes to the pending and deal with later.
    */
    if(!Nil::Event::node_removed(evt))
    {
      if(Nil::Data::has_mesh_resource(node))
      {
        self->pending_mesh_load.emplace_back(node);
      }
      
      if(Nil::Data::has_camera(node))
      {
        self->pending_camera_node_updates.emplace_back(node);
      }
      
      if(Nil::Data::has_material(node))
      {
        self->pending_renderable_node_updates.emplace_back(node);
      }
      
      /*
        Viewport changes mean we need to regenerate projection mats
        so we just regenerate all the cameras.
      */
      if(Nil::Data::has_window(node))
      {
        Nil::Data::Window win{};
        Nil::Data::get(node, win);
        
        self->current_viewport[0] = win.width;
        self->current_viewport[1] = win.height;
        
        for(auto cam : self->camera_nodes)
        {
          self->pending_camera_node_updates.emplace_back(cam);
        }
      }
      
      if(Nil::Data::has_graphics(node))
      {
        if(!self->has_initialized)
        {
          rov_initialize();
        }
      
        self->has_initialized = true;
      }
    }
    else
    {
      if(Nil::Data::has_camera(node))
      {
        self->pending_camera_node_removals.emplace_back(node);
      }
      
      if(Nil::Data::has_material(node))
      {
        self->pending_renderable_node_removals.emplace_back(node);
      }
    }
  }
}


// ------------------------------------- [ Renderer Aspect Impl Early Think ] --


void
early_think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  if(self->has_initialized && self->process_pending)
  {
    /*
      Remove data first
    */
    {
      for(size_t i = 0; i < self->pending_renderable_node_removals.size(); ++i)
      {
        for(size_t j = 0; j < self->renderable_nodes.size(); ++j)
        {
          if(self->renderable_nodes[j] == self->pending_renderable_node_removals[i])
          {
            self->renderable_nodes.erase(self->renderable_nodes.begin() + j);
            self->renderables.erase(self->renderables.begin() + j);
            break;
          }
        }
      }
      
      for(size_t i = 0; i < self->pending_camera_node_removals.size(); ++i)
      {
        for(size_t j = 0; j < self->camera_nodes.size(); ++j)
        {
          if(self->camera_nodes[j] == self->pending_camera_node_removals[i])
          {
            self->camera_nodes.erase(self->camera_nodes.begin() + j);
            self->rov_camera.erase(self->rov_camera.begin() + j);
            break;
          }
        }
      }
      
      self->pending_renderable_node_removals.clear();
      self->pending_camera_node_removals.clear();
    }
    
    /*
      Update or insert new data
    */
    {
      for(auto &node : self->pending_renderable_node_updates)
      {
        Nil::Data::Transform trans{};
        Nil::Data::get(node, trans, true);
      
        Nil::Data::Material mat{};
        Nil::Data::get(node, mat);
        
        Nil::Data::Mesh mesh{};
        
        if(Nil::Data::has_mesh(node))
        {
          Nil::Data::get(node, mesh);
        }
      
        // ROV Mesh
        const uint32_t rov_mesh = mesh.mesh_id;
        
//        for(size_t i = 0; i < self->external_mesh_ids.size(); ++i)
//        {
//          if(self->external_mesh_ids[i] == mesh.mesh_id)
//          {
//            rov_mesh = self->internal_mesh_ids[i];
//          }
//        }
      
        Data::ROV_Renderable rov_render
        {
          (uint8_t)mat.shader,
          math::mat4_from_nil_transform(trans),
          rov_mesh,
        };
        
        memcpy(rov_render.color, mat.color, sizeof(Nil::Data::Material::color));
        /*
          Check to see if we have it already.
        */
        bool insert_new_data = true;
        
        for(size_t j = 0; j < self->renderable_nodes.size(); ++j)
        {
          if(self->renderable_nodes[j] == node)
          {
            self->renderables[j] = rov_render;
            insert_new_data = false;
            break;
          }
        }
        
        if(insert_new_data)
        {
          self->renderable_nodes.emplace_back(node);
          self->renderables.emplace_back(rov_render);
        }
      }
      
      for(auto &node : self->pending_camera_node_updates)
      {
        Nil::Data::Transform trans{};
        Nil::Data::get(node, trans, true);
        
        Nil::Data::Camera cam_data{};
        Nil::Data::get(node, cam_data);
      
        uint32_t clear_flags = 0;
        if(cam_data.clear_color_buffer) { clear_flags |= rovClearFlag_Color; }
        if(cam_data.clear_depth_buffer) { clear_flags |= rovClearFlag_Depth; }

        const Data::ROV_Camera rov_camera
        {
          math::mat4_lookat_from_nil_transform(trans),
          math::mat4_projection_from_nil_camera(cam_data, self->current_viewport),
          clear_flags
        };
        
        /*
          Check to see if we have it already.
        */
        bool insert_new_data = true;
        
        for(size_t j = 0; j < self->camera_nodes.size(); ++j)
        {
          if(self->camera_nodes[j] == node)
          {
            self->rov_camera[j] = rov_camera;
            insert_new_data = false;
            break;
          }
        }
        
        if(insert_new_data)
        {
          self->camera_nodes.emplace_back(node);
          self->rov_camera.emplace_back(rov_camera);
        }
      }
      
      self->pending_renderable_node_updates.clear();
      self->pending_camera_node_updates.clear();
    } // Update or insert data
    
    /*
      Resources
    */
    {
      for(auto &node : self->pending_mesh_load)
      {
        Nil::Data::Mesh_resource mesh_resource{};
        Nil::Data::get(node, mesh_resource);
        
        if(mesh_resource.status == 0)
        {
          const uint32_t mesh = rov_createMesh(mesh_resource.position_vec3, mesh_resource.normal_vec3, mesh_resource.texture_coords_vec2, mesh_resource.count);
          self->internal_mesh_ids.emplace_back(mesh);
          self->external_mesh_ids.emplace_back(mesh_resource.id);
          
          mesh_resource.status = 1;
          Nil::Data::set(node, mesh_resource);
        }
      }
    }
  } // Has inited and process
}


// -------------------------------------- [ Renderer Aspect Impl Late Think ] --


void
think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);
  
  if(self->has_initialized)
  {
    for(auto &cam : self->rov_camera)
    {
      #ifdef IMGIZMO_DEVELOPER_SUPPORT
      float world_mat[16];
      math::mat4_to_array(self->renderables.front().world, world_mat);
    
      ImGuiIO& io = ImGui::GetIO();
      ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
      ImGuizmo::Manipulate(
        math::mat4_get_data(cam.view),
        math::mat4_get_data(cam.proj),
        ImGuizmo::ROTATE,
        ImGuizmo::WORLD,
        world_mat
      );
      #endif
    
      const uint32_t viewport[4] {0,0,self->current_viewport[0], self->current_viewport[1]};
    
      rov_startRenderPass(
        math::mat4_get_data(cam.view),
        math::mat4_get_data(cam.proj),
        viewport,
        cam.clear_flags);
      
      for(auto &render : self->renderables)
      {
        if(self->internal_mesh_ids.size() > render.mesh_id)
        {
          rov_setColor(render.color[0], render.color[1], render.color[2], render.color[3]);
          rov_setShader(render.shader_type);
          rov_setMesh(self->external_mesh_ids[render.mesh_id]);

          rov_submitMeshTransform(math::mat4_get_data(render.world));
        }
      }
    }
        
    rov_execute();
  }
}


} // ns
} // ns


#endif // impl inc
#endif // impl guard


