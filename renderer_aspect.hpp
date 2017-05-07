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
  
  std::vector<Nil::Node> camera_nodes;
  
  std::vector<Nil::Node> renderable_nodes;
  
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
  
  camera_nodes.resize(1);
}
  
  
GL_Aspect::~GL_Aspect()
{
}

  
void
GL_Aspect::node_events(const Nil::Node_event node_events[], const size_t count)
{
  for(uint32_t i = 0; i < count; ++i)
  {
    auto &node = node_events[i].node;
  
    if(Nil::Data::has_graphics(node_events[i].node))
    {
      static int init = 0;
      
      if(!init)
      {
        init = 1;
        rov_initialize();
        
        const float positions[]
      {
      -0.5f, -0.5f,  0.5f,
      0.5f, -0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,
      -0.5f, -0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,
      0.5f,  0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, -0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,
      -0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, -0.5f,  0.5f,
      0.5f, -0.5f,  0.5f,
      -0.5f, -0.5f,  0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f,  0.5f, -0.5f,
      0.5f,  0.5f, -0.5f,
      0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f, -0.5f,
      };

      const float normals[] {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
      };


      const float texture_coords[] {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
      };

      uint32_t mesh = rov_createMesh(positions, normals, texture_coords, 36);
        
      }
      
      is_init = true;
    }
    
    if(Nil::Data::has_camera(node))
    {
      Nil::Data::Camera cam;
      Nil::Data::get(node, cam);
      
      camera_nodes[i] = node;
    }
    
    if(Nil::Data::has_material(node))
    {
      bool already_exists = false;
    
      for(auto &n : renderable_nodes)
      {
        if (n == node)
        {
          already_exists = true;
        }
      }
      
      if(!already_exists)
      {
        renderable_nodes.push_back(node);
      }
    }
  }
}


void
GL_Aspect::early_think(const float dt)
{
  // Check cameras
  for(auto it = std::begin(camera_nodes); it != std::end(camera_nodes); ++it)
  {
   // it = camera_nodes.erase(it);
  }
}


void
GL_Aspect::think(const float dt)
{
  if(is_init)
  {
    for(auto &cam : camera_nodes)
    {
      Nil::Data::Camera cam_data;
      Nil::Data::get(cam, cam_data);
      
      Nil::Data::Transform cam_trans;
      Nil::Data::get(cam, cam_trans);
    
      rov_setColor(1, 0, 0, 1);
      
//      rov_clearSurface(true, false);
  //    rov_setMesh(mesh);
    
      
      math::mat4 proj;
      
      if(cam_data.type == Nil::Data::Camera::PERSPECTIVE)
      {
        proj = math::mat4_projection(cam_data.width, cam_data.height, cam_data.near_plane, cam_data.far_plane  , cam_data.fov);
      }
      else
      {
        proj = math::mat4_orthographic(cam_data.width, cam_data.height, cam_data.near_plane, cam_data.far_plane);
      }
      
      const math::vec3 cam_pos = math::vec3_init_with_array(cam_trans.position);
      const math::quat cam_rot = math::quat_init(cam_trans.rotation[0], cam_trans.rotation[1], cam_trans.rotation[2], cam_trans.rotation[3]);
      const math::vec3 cam_fwd = math::quat_rotate_point(cam_rot, math::vec3_init(0,0,1));
      const math::vec3 lookat_pos = math::vec3_add(cam_pos, cam_fwd);
      
      const math::mat4 view = math::mat4_lookat(cam_pos, lookat_pos, math::vec3_init(0,1,0));
    
      uint32_t clear_flags = 0;
      if(cam_data.clear_color_buffer) { clear_flags |= rovClearFlag_Color; }
      if(cam_data.clear_depth_buffer) { clear_flags |= rovClearFlag_Depth; }
    
      rov_startRenderPass(math::mat4_get_data(view), math::mat4_get_data(proj), clear_flags);
      
      for(auto &render : renderable_nodes)
      {
        math::mat4 world = math::mat4_id();
        
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


