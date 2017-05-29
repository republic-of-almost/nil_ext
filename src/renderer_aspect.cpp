//#include <GL/gl3w.h>
//
//#define ROV_GL_IMPL
#include <aspect/renderer_aspect.hpp>
#include <rov/rov.hpp>
#include <nil/nil.hpp>
#include <nil/node_event.hpp>
#include <nil/node_controller.hpp>
#include <nil/aspect.hpp>
#include <nil/node.hpp>
#include <nil/data/data.hpp>
#include <nil/data/window.hpp>
#include <aspect/math_nil_extensions.hpp>
#include <utilities/utilities.hpp>

#ifdef IMGIZMO_DEVELOPER_SUPPORT
#include <imguizmo/ImGuizmo.h>
#endif

#ifdef IMGUI_DEVELOPER_SUPPORT

#include <imgui/imgui.h>


// ------------------------------------------------ [ Renderer Aspect IMGUI ] --


namespace {


#ifndef NIMGUI_ASPECT
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

      Nil::Node_list all_cam_nodes = self->camera_nodes.all();

      for(size_t i = 0; i < all_cam_nodes.size(); ++i)
      {
        char label[32];
        sprintf(label, "%04d", all_cam_nodes[i].get_id());

        if (ImGui::Selectable(label, selected == all_cam_nodes[i].get_id(), ImGuiSelectableFlags_SpanAllColumns))
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

      Nil::Node_list pending_cam_updates = self->camera_nodes.updated();
      Nil::Node_list pending_cam_removals = self->camera_nodes.removed();

      ImGui::Text(
        "Pending Camera Updates: %s",
        pending_cam_updates.size() ? "YES" : "NO"
      );

      ImGui::Text(
        "Pending Camera Removals: %s",
        pending_cam_removals.size() ? "YES" : "NO"
      );
    }

    if(ImGui::CollapsingHeader("Draw Calls"))
    {
      ImGui::Columns(1, "renderables");
      ImGui::Separator();
      ImGui::Text("Node ID");     ImGui::NextColumn();

      ImGui::Separator();

      int selected = -1;

      Nil::Node_list render_nodes = self->renderable_nodes.all();

      for(size_t i = 0; i < self->renderables.size(); ++i)
      {
        char label[32];
        sprintf(label, "%04d", render_nodes[i].get_id());

        if (ImGui::Selectable(label, selected == render_nodes[i].get_id(), ImGuiSelectableFlags_SpanAllColumns))
        {
          selected = (int)i;
        }

        ImGui::NextColumn();
      }
      ImGui::Columns(1);
      ImGui::Separator();

      ImGui::Spacing();

      Nil::Node_list updated_render_nodes = self->renderable_nodes.updated();

      ImGui::Text(
        "Pending Renderable Updates: %s",
        updated_render_nodes.size() ? "YES" : "NO"
      );

      Nil::Node_list removed_render_nodes = self->renderable_nodes.removed();

      ImGui::Text(
        "Pending Renderable Removals: %s",
        removed_render_nodes.size() ? "YES" : "NO"
      );
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
#endif


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

  // Camera nodes
  const uint64_t cam_data = Nil::Data::get_type_id(Nil::Data::Camera{});
  Nil::Node_controller cam_nodes(cam_data);
  self->camera_nodes = static_cast<Nil::Node_controller&&>(cam_nodes);

  // Renderable nodes
  uint64_t renderable_data = 0;
  renderable_data |= Nil::Data::get_type_id(Nil::Data::Material{});
  renderable_data |= Nil::Data::get_type_id(Nil::Data::Mesh{});
  renderable_data |= Nil::Data::get_type_id(Nil::Data::Texture{});
  Nil::Node_controller render_nodes(renderable_data);
  self->renderable_nodes = static_cast<Nil::Node_controller&&>(render_nodes);



  aspect.data_types = 0;
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Camera{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Material{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Mesh{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Texture{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Resource{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Graphics{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Mesh_resource{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Window{});
  aspect.data_types |= Nil::Data::get_type_id(Nil::Data::Developer{});


  #ifndef NIMGUI_ASPECT
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

  self->camera_nodes.process(event_list);
  self->renderable_nodes.process(event_list);

  Nil::Event_data evt;

  event_list.reset();

  while(event_list.get(evt))
  {
    const Nil::Node &node = Nil::Node(evt.node_id);

    if(Nil::Data::has_window(node))
    {
      Nil::Data::Window win{};
      Nil::Data::get(node, win);

      self->current_viewport[0] = win.width;
      self->current_viewport[1] = win.height;
    }

    if(Nil::Data::has_graphics(node))
    {
      if(!self->has_initialized)
      {
        rov_initialize();
      }

      self->has_initialized = true;
    }

    if(Nil::Data::has_mesh_resource(node))
    {
      self->pending_mesh_load.emplace_back(node);
    }

    if(Nil::Data::has_developer(node))
    {
      Nil::Data::Developer data{};
      Nil::Data::get(node, data);

      if(data.type_id == 2)
      {
        self->debug_lines = node;
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


  /*
    Remove data first
  */
  {
    Nil::Node_list remove_renderables = self->renderable_nodes.removed();

    for(auto &node : remove_renderables)
    {
      for(size_t i = 0; i < self->renderable_node_ids.size(); ++i)
      {
        if(node.get_id() == self->renderable_node_ids[i])
        {
          self->renderable_node_ids.erase(self->renderable_node_ids.begin() + i);
          self->renderables.erase(self->renderables.begin() + i);
          break;
        }
      }
    }

    Nil::Node_list pending_camera_removals = self->camera_nodes.removed();

    for(auto &node : pending_camera_removals)
    {
      for(size_t i = 0; i < self->camera_node_ids.size(); ++i)
      {
        if(node.get_id() == self->camera_node_ids[i])
        {
          self->camera_node_ids.erase(self->camera_node_ids.begin() + i);
          self->rov_camera.erase(self->rov_camera.begin() + i);
        }
      }
    }
  }

  /*
    Update or insert new data
  */
  {
    Nil::Node_list update_renderers = self->renderable_nodes.updated_and_added();

    for(auto &node : update_renderers)
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

      for(size_t j = 0; j < self->renderable_node_ids.size(); ++j)
      {
        if(self->renderable_node_ids[j] == node.get_id())
        {
          self->renderables[j] = rov_render;
          insert_new_data = false;
          break;
        }
      }

      if(insert_new_data)
      {
        self->renderable_node_ids.emplace_back(node.get_id());
        self->renderables.emplace_back(rov_render);
      }
    }


    /*
      Rebuild the cameras based on the viewport.
      This is alittle hack atm.
    */
    for(uint32_t i = 0; i < self->camera_node_ids.size(); ++i)
    {
      Nil::Node node(self->camera_node_ids[i]);
      LIB_ASSERT(node.is_valid());

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

      self->rov_camera[i] = rov_camera;
    }

    {
      Nil::Node_list update_cameras = self->camera_nodes.updated_and_added();

      for(auto &node : update_cameras)
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

        for(size_t j = 0; j < self->camera_node_ids.size(); ++j)
        {
          if(self->camera_node_ids[j] == node.get_id())
          {
            self->rov_camera[j] = rov_camera;
            insert_new_data = false;
            break;
          }
        }

        if(insert_new_data)
        {
          self->camera_node_ids.emplace_back(node.get_id());
          self->rov_camera.emplace_back(rov_camera);
        }
      }

//        self->pending_renderable_node_updates.clear();
//        self->pending_camera_node_updates.clear();
    } // Update or insert data
  }

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

      // debug_lines
      if(self->debug_lines)
      {
        Nil::Data::Developer line_data{};
        Nil::Data::get(self->debug_lines, line_data);

        const float *data = (float*)line_data.aux_01;
        const size_t count = (size_t)line_data.aux_02;

        LIB_ASSERT(count % 9 == 0);

        const size_t lines = count / 9;

        for(size_t i = 0; i < lines; ++i)
        {
          const size_t index = i * 9;

          rov_setColor(data[index + 6], data[index + 7], data[index + 8], 1.f);
          rov_submitLine(&data[index + 0], &data[index + 3]);
        }

        // Signal to line renderer not reset the data buffer.
        line_data.aux_02 = 0;
        Nil::Data::set(self->debug_lines, line_data);
      }
    }

    rov_execute();
  }
}


} // ns
} // ns
