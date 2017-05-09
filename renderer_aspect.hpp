#ifndef GL_ASPECT_INCLUDED_5D7A0FCB_0B1D_4455_9F83_E01195FBD0D7
#define GL_ASPECT_INCLUDED_5D7A0FCB_0B1D_4455_9F83_E01195FBD0D7


#include <nil/fwd.hpp>
#include <nil/aspect.hpp>
#include <math/math.hpp>
#include <GL/gl3w.h>

#define ROV_GL_IMPL
#include <external/rov/rov.hpp>

#include <utilities/utilities.hpp>
#include <stddef.h>


namespace Nil_ext {


struct GL_Aspect : public Nil::Aspect
{
  explicit
  GL_Aspect();
  
  ~GL_Aspect();

  void
  node_events(const Nil::Node_event node_events[], const size_t count) override;
  
  void
  early_think(const float dt) override;
  
  void
  think(const float dt) override;
  
  
  // -------------------------------------------------------- [ Member Vars ] --
  
  struct ROV_Camera
  {
    math::mat4 view;
    math::mat4 proj;
    
    uint32_t clear_flags;
  };
  
  std::vector<Nil::Node> m_pending_mesh_load;
  
  std::vector<Nil::Node> m_camera_nodes;
  std::vector<ROV_Camera> m_rov_camera;
  
  
  struct ROV_Renderable
  {
    math::mat4 world;
  };
  
  std::vector<Nil::Node>      m_renderable_nodes;
  std::vector<ROV_Renderable> m_renderables;
  
  
  bool is_init = false;
  
};

} // ns


#endif // inc guard

/*
  Implimentation.
*/
#ifdef RENDERER_ASPECT_IMPL

#ifndef RENDERER_ASPECT_IMPL_C4901940_F51C_496B_A821_BDE103DB8742
#define RENDERER_ASPECT_IMPL_C4901940_F51C_496B_A821_BDE103DB8742


#include <nil/node.hpp>


namespace Nil_ext {


GL_Aspect::GL_Aspect()
{
  register_data_type(Nil::Data::get_type_id(Nil::Data::Camera{}));
  register_data_type(Nil::Data::get_type_id(Nil::Data::Material{}));
  register_data_type(Nil::Data::get_type_id(Nil::Data::Mesh{}));
  register_data_type(Nil::Data::get_type_id(Nil::Data::Texture{}));
  register_data_type(Nil::Data::get_type_id(Nil::Data::Resource{}));
  register_data_type(Nil::Data::get_type_id(Nil::Data::Graphics{}));
  register_data_type(Nil::Data::get_type_id(Nil::Data::Mesh_resource{}));
}
  
  
GL_Aspect::~GL_Aspect()
{
}

  
void
GL_Aspect::node_events(const Nil::Node_event node_events[], const size_t count)
{
  for(uint32_t i = 0; i < count; ++i)
  {
    const Nil::Node_event &evt = node_events[i];
    const Nil::Node &node = node_events[i].node;
    
    /*
      New Nodes
      Added to the scene.
    */
    if(Nil::Event::node_added(evt))
    {
      if(Nil::Data::has_mesh_resource(node))
      {
        m_pending_mesh_load.push_back(node);
      }
    
      Nil::Data::Transform trans;
      Nil::Data::get(node, trans);
    
      if(Nil::Data::has_camera(node))
      {
        Nil::Data::Camera cam_data;
        Nil::Data::get(node, cam_data);
        
        // CPU Task
        {
          math::mat4 proj;
        
          if(cam_data.type == Nil::Data::Camera::PERSPECTIVE)
          {
            proj = math::mat4_projection(cam_data.width, cam_data.height, cam_data.near_plane, cam_data.far_plane  , cam_data.fov);
          }
          else
          {
            proj = math::mat4_orthographic(cam_data.width, cam_data.height, cam_data.near_plane, cam_data.far_plane);
          }
          
          
          const math::vec3 cam_pos = math::vec3_init_with_array(trans.position);
          const math::quat cam_rot = math::quat_init(trans.rotation[0], trans.rotation[1], trans.rotation[2], trans.rotation[3]);
          const math::vec3 cam_fwd = math::quat_rotate_point(cam_rot, math::vec3_init(0,0,1));
          const math::vec3 lookat_pos = math::vec3_add(cam_pos, cam_fwd);

          const math::mat4 view = math::mat4_lookat(cam_pos, lookat_pos, math::vec3_init(0,1,0));
          
          uint32_t clear_flags = 0;
          if(cam_data.clear_color_buffer) { clear_flags |= rovClearFlag_Color; }
          if(cam_data.clear_depth_buffer) { clear_flags |= rovClearFlag_Depth; }
          
          ROV_Camera rov_cam;
          rov_cam.proj = proj;
          rov_cam.view = view;
          rov_cam.clear_flags = clear_flags;
          
          // Keep data.
          m_camera_nodes.push_back(node);
          m_rov_camera.push_back(rov_cam);
        }
      }
      
      if(Nil::Data::has_material(node))
      {
        const math::vec3 pos = math::vec3_init_with_array(trans.position);
        const math::vec3 scale = math::vec3_init_with_array(trans.scale);
        const math::quat rot = math::quat_init(trans.rotation[0], trans.rotation[1], trans.rotation[2], trans.rotation[3]);

        const math::mat4 pos_mat = math::mat4_translate(pos);
        const math::mat4 scale_mat = math::mat4_scale(scale);
        const math::mat4 rot_mat = math::mat4_id();
        // ROTATION!!!
        
        const math::mat4 world = math::mat4_multiply(scale_mat, rot_mat, pos_mat);
        
        ROV_Renderable rov_render;
        rov_render.world = world;
        
        m_renderable_nodes.push_back(node);
        m_renderables.push_back(rov_render);
      }
    }
    
    /*
      Moved Nodes
      Added to the scene.
    */
    if(Nil::Event::node_moved(evt))
    {
      Nil::Data::Transform trans;
      Nil::Data::get(node, trans);
    
      if(Nil::Data::has_camera(node))
      {
        Nil::Data::Camera cam_data;
        Nil::Data::get(node, cam_data);
        
        // CPU Task
        {
          math::mat4 proj;
        
          if(cam_data.type == Nil::Data::Camera::PERSPECTIVE)
          {
            proj = math::mat4_projection(cam_data.width, cam_data.height, cam_data.near_plane, cam_data.far_plane  , cam_data.fov);
          }
          else
          {
            proj = math::mat4_orthographic(cam_data.width, cam_data.height, cam_data.near_plane, cam_data.far_plane);
          }
          
          const math::vec3 cam_pos = math::vec3_init_with_array(trans.position);
          const math::quat cam_rot = math::quat_init(trans.rotation[0], trans.rotation[1], trans.rotation[2], trans.rotation[3]);
          const math::vec3 cam_fwd = math::quat_rotate_point(cam_rot, math::vec3_init(0,0,1));
          const math::vec3 lookat_pos = math::vec3_add(cam_pos, cam_fwd);

          const math::mat4 view = math::mat4_lookat(cam_pos, lookat_pos, math::vec3_init(0,1,0));
          
          uint32_t clear_flags = 0;
          if(cam_data.clear_color_buffer) { clear_flags |= rovClearFlag_Color; }
          if(cam_data.clear_depth_buffer) { clear_flags |= rovClearFlag_Depth; }
          
          ROV_Camera rov_cam;
          rov_cam.proj = proj;
          rov_cam.view = view;
          rov_cam.clear_flags = clear_flags;
          
          // Keep data.
          for(uint32_t i = 0; i < m_camera_nodes.size(); ++i)
          {
            if(node == m_camera_nodes[i])
            {
              m_rov_camera[i] = rov_cam;
              
              break;
            }
          }
        }
      }
      
      if(Nil::Data::has_material(node))
      {
        const math::vec3 pos = math::vec3_init_with_array(trans.position);
        const math::vec3 scale = math::vec3_init_with_array(trans.scale);
        const math::quat rot = math::quat_init(trans.rotation[0], trans.rotation[1], trans.rotation[2], trans.rotation[3]);

        const math::mat4 pos_mat = math::mat4_translate(pos);
        const math::mat4 scale_mat = math::mat4_scale(scale);
        const math::mat4 rot_mat = math::mat4_id();
        // ROTATION!!!
        
        const math::mat4 world = math::mat4_multiply(scale_mat, rot_mat, pos_mat);
        
        ROV_Renderable rov_render;
        rov_render.world = world;
        
        // Keep data.
        for(uint32_t i = 0; i < m_renderable_nodes.size(); ++i)
        {
          if(node == m_renderable_nodes[i])
          {
            m_renderables[i] = rov_render;
            
            break;
          }
        }
      }
    }
    
    if(Nil::Event::node_updated(evt))
    {
      Nil::Data::Transform trans;
      Nil::Data::get(node, trans);
    
      if(Nil::Data::has_camera(node))
      {
        Nil::Data::Camera cam_data;
        Nil::Data::get(node, cam_data);
        
        // CPU Task
        {
          math::mat4 proj;
        
          if(cam_data.type == Nil::Data::Camera::PERSPECTIVE)
          {
            proj = math::mat4_projection(cam_data.width, cam_data.height, cam_data.near_plane, cam_data.far_plane  , cam_data.fov);
          }
          else
          {
            proj = math::mat4_orthographic(cam_data.width, cam_data.height, cam_data.near_plane, cam_data.far_plane);
          }
          
          const math::vec3 cam_pos = math::vec3_init_with_array(trans.position);
          const math::quat cam_rot = math::quat_init(trans.rotation[0], trans.rotation[1], trans.rotation[2], trans.rotation[3]);
          const math::vec3 cam_fwd = math::quat_rotate_point(cam_rot, math::vec3_init(0,0,1));
          const math::vec3 lookat_pos = math::vec3_add(cam_pos, cam_fwd);

          const math::mat4 view = math::mat4_lookat(cam_pos, lookat_pos, math::vec3_init(0,1,0));
          
          uint32_t clear_flags = 0;
          if(cam_data.clear_color_buffer) { clear_flags |= rovClearFlag_Color; }
          if(cam_data.clear_depth_buffer) { clear_flags |= rovClearFlag_Depth; }
          
          ROV_Camera rov_cam;
          rov_cam.proj = proj;
          rov_cam.view = view;
          rov_cam.clear_flags = clear_flags;
          
          // Keep data.
          bool add_to_data = true;
          
          for(uint32_t i = 0; i < m_camera_nodes.size(); ++i)
          {
            if(node == m_camera_nodes[i])
            {
              m_rov_camera[i] = rov_cam;
              add_to_data = false;
              break;
            }
          }
          
          if(add_to_data)
          {
            m_camera_nodes.push_back(node);
            m_rov_camera.push_back(rov_cam);
          }
        }
      }
      
      if(Nil::Data::has_material(node))
      {
        const math::vec3 pos = math::vec3_init_with_array(trans.position);
        const math::vec3 scale = math::vec3_init_with_array(trans.scale);
        const math::quat rot = math::quat_init(trans.rotation[0], trans.rotation[1], trans.rotation[2], trans.rotation[3]);

        const math::mat4 pos_mat = math::mat4_translate(pos);
        const math::mat4 scale_mat = math::mat4_scale(scale);
        const math::mat4 rot_mat = math::mat4_id();
        // ROTATION!!!
        
        const math::mat4 world = math::mat4_multiply(scale_mat, rot_mat, pos_mat);
        
        ROV_Renderable rov_render;
        rov_render.world = world;
        
        // Keep data.
        bool add_to_data = true;
        for(uint32_t i = 0; i < m_renderable_nodes.size(); ++i)
        {
          if(node == m_renderable_nodes[i])
          {
            m_renderables[i] = rov_render;
            
            add_to_data = false;
            
            break;
          }
        }
        
        if(add_to_data)
        {
          m_renderable_nodes.push_back(node);
          m_renderables.push_back(rov_render);
        }
      }
    }
    
    if(Nil::Event::node_removed(evt))
    {
      Nil::Data::Transform trans;
      Nil::Data::get(node, trans);
    
      if(Nil::Data::has_camera(node))
      {
        for(uint32_t i = 0; i < m_camera_nodes.size(); ++i)
        {
          if(node == m_camera_nodes[i])
          {
            m_rov_camera.erase(std::begin(m_rov_camera) + i);
            m_camera_nodes.erase(std::begin(m_camera_nodes) + i);
            
            break;
          }
        }
      }
      
      if(Nil::Data::has_material(node))
      {
        for(uint32_t i = 0; i < m_renderable_nodes.size(); ++i)
        {
          if(node == m_renderable_nodes[i])
          {
            m_renderables.erase(std::begin(m_renderables) + i);
            m_renderable_nodes.erase(std::begin(m_renderable_nodes) + i);
            
            break;
          }
        }
      }
    }
  
    if(Nil::Data::has_graphics(node_events[i].node))
    {
      static int init = 0;
      
      if(!init)
      {
        init = 1;
        rov_initialize();
        
//        const float positions[]
//      {
//      -0.5f, -0.5f,  0.5f,
//      0.5f, -0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,
//      -0.5f,  0.5f,  0.5f,
//      -0.5f, -0.5f,  0.5f,
//      -0.5f,  0.5f,  0.5f,
//      -0.5f,  0.5f, -0.5f,
//      -0.5f, -0.5f, -0.5f,
//      -0.5f, -0.5f, -0.5f,
//      -0.5f, -0.5f,  0.5f,
//      -0.5f,  0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,
//      0.5f,  0.5f, -0.5f,
//      0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,
//      -0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f,  0.5f,
//      0.5f, -0.5f,  0.5f,
//      -0.5f, -0.5f,  0.5f,
//      -0.5f, -0.5f, -0.5f,
//      -0.5f,  0.5f, -0.5f,
//      0.5f,  0.5f, -0.5f,
//      0.5f,  0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,
//      -0.5f,  0.5f,  0.5f,
//      -0.5f,  0.5f, -0.5f,
//      };
//
//      const float normals[] {
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//      };
//
//
//      const float texture_coords[] {
//        0.0f, 0.0f,
//        1.0f, 0.0f,
//        1.0f, 1.0f,
//        1.0f, 1.0f,
//        0.0f, 1.0f,
//        0.0f, 0.0f,
//        0.0f, 0.0f,
//        1.0f, 0.0f,
//        1.0f, 1.0f,
//        1.0f, 1.0f,
//        0.0f, 1.0f,
//        0.0f, 0.0f,
//        1.0f, 0.0f,
//        1.0f, 1.0f,
//        0.0f, 1.0f,
//        0.0f, 1.0f,
//        0.0f, 0.0f,
//        1.0f, 0.0f,
//        1.0f, 0.0f,
//        1.0f, 1.0f,
//        0.0f, 1.0f,
//        0.0f, 1.0f,
//        0.0f, 0.0f,
//        1.0f, 0.0f,
//        0.0f, 1.0f,
//        1.0f, 1.0f,
//        1.0f, 0.0f,
//        1.0f, 0.0f,
//        0.0f, 0.0f,
//        0.0f, 1.0f,
//        0.0f, 1.0f,
//        1.0f, 1.0f,
//        1.0f, 0.0f,
//        1.0f, 0.0f,
//        0.0f, 0.0f,
//        0.0f, 1.0f,
//      };

        if(!m_pending_mesh_load.empty())
        {
          Nil::Data::Mesh_resource mesh_resource{};
          Nil::Data::get(m_pending_mesh_load[0], mesh_resource);
        
          uint32_t mesh = rov_createMesh(mesh_resource.position_vec3, mesh_resource.normal_vec3, mesh_resource.texture_coords_vec2, mesh_resource.count);
        }
        
      }
      
      is_init = true;
    }
  }
}


void
GL_Aspect::early_think(const float dt)
{
  // Check cameras
//  for(auto it = std::begin(camera_nodes); it != std::end(camera_nodes); ++it)
//  {
   // it = camera_nodes.erase(it);
//  }
}


void
GL_Aspect::think(const float dt)
{
  if(is_init)
  {
    for(auto &cam : m_rov_camera)
    {
      rov_setColor(1, 0, 0, 1);
    
      rov_startRenderPass(math::mat4_get_data(cam.view), math::mat4_get_data(cam.proj), cam.clear_flags);
      
      for(auto &render : m_renderable_nodes)
      {
        Nil::Data::Transform trans{};
        Nil::Data::get(render, trans);
        
        math::mat4 pos = math::mat4_translate(math::vec3_init_with_array(trans.position));
      
        rov_submitMeshTransform(math::mat4_get_data(pos));
      }
    }
  
    rov_execute();
  }
}


} // ns


#endif // impl inc
#endif // impl guard


