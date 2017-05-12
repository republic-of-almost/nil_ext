#ifndef GL_ASPECT_INCLUDED_5D7A0FCB_0B1D_4455_9F83_E01195FBD0D7
#define GL_ASPECT_INCLUDED_5D7A0FCB_0B1D_4455_9F83_E01195FBD0D7


#include <nil/fwd.hpp>
#include <nil/node.hpp>
#include <math/math.hpp>
#include <utilities/utilities.hpp>
#include <vector>


namespace Nil_ext {
namespace ROV_Aspect {


struct Data
{
  std::vector<Nil::Node> pending_mesh_load;
  
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
  
  struct ROV_Renderable
  {
    uint8_t shader_type;
    math::mat4 world;
    float color[4];
  };
  
  std::vector<Nil::Node>      pending_renderable_node_updates;
  std::vector<Nil::Node>      pending_renderable_node_removals;
  std::vector<Nil::Node>      renderable_nodes;
  std::vector<ROV_Renderable> renderables;
  
  bool has_initialized = false;

};


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


#include <GL/gl3w.h>

#define ROV_GL_IMPL
#include <external/rov/rov.hpp>

#include <nil/node.hpp>
#include <math_nil_extensions.hpp>


namespace Nil_ext {
namespace ROV_Aspect {


void
start_up(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);


  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Camera{}));
  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Material{}));
  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Mesh{}));
  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Texture{}));
  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Resource{}));
  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Graphics{}));
  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Mesh_resource{}));
}


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


void
early_think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  if(self->has_initialized)
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
        Nil::Data::get(node, trans);
      
        Nil::Data::Material mat{};
        Nil::Data::get(node, mat);
      
        Data::ROV_Renderable rov_render
        {
          (uint8_t)mat.shader,
          math::mat4_from_nil_transform(trans)
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
        Nil::Data::get(node, trans);
        
        Nil::Data::Camera cam_data{};
        Nil::Data::get(node, cam_data);
      
        uint32_t clear_flags = 0;
        if(cam_data.clear_color_buffer) { clear_flags |= rovClearFlag_Color; }
        if(cam_data.clear_depth_buffer) { clear_flags |= rovClearFlag_Depth; }

        const Data::ROV_Camera rov_camera
        {
          math::mat4_lookat_from_nil_transform(trans),
          math::mat4_projection_from_nil_camera(cam_data),
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
        static int once = 0;
        
        if(once == 0)
        {
          Nil::Data::Mesh_resource mesh_resource{};
          Nil::Data::get(node, mesh_resource);
          
          uint32_t mesh = rov_createMesh(mesh_resource.position_vec3, mesh_resource.normal_vec3, mesh_resource.texture_coords_vec2, mesh_resource.count);
          
          once = 1;
        }
      }
    }
  } // Has inited
}


void
think(Nil::Engine &engine, Nil::Aspect &aspect)
{
  Data *self = reinterpret_cast<Data*>(aspect.user_data);
  LIB_ASSERT(self);

  if(self->has_initialized)
  {
    for(auto &cam : self->rov_camera)
    {
      rov_startRenderPass(math::mat4_get_data(cam.view), math::mat4_get_data(cam.proj), cam.clear_flags);
      
      for(auto &render : self->renderables)
      {
        rov_setColor(render.color[0], render.color[1], render.color[2], render.color[3]);
        rov_setShader(render.shader_type);

        rov_submitMeshTransform(math::mat4_get_data(render.world));
      }
    }
        
    rov_execute();
  }
}


} // ns


//GL_Aspect::GL_Aspect()
//{
//  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Camera{}));
//  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Material{}));
//  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Mesh{}));
//  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Texture{}));
//  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Resource{}));
//  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Graphics{}));
//  aspect.data_types.emplace_back(Nil::Data::get_type_id(Nil::Data::Mesh_resource{}));
//}
  
  
//GL_Aspect::~GL_Aspect()
//{
//}

  
//void
//GL_Aspect::node_events(const Nil::Node_event node_events[], const size_t count)
//{
//  for(uint32_t i = 0; i < count; ++i)
//  {
//    const Nil::Node_event &evt = node_events[i];
//    const Nil::Node &node = node_events[i].node;
//    
//    /*
//      We add all nodes to the pending and deal with later.
//    */
//    if(!Nil::Event::node_removed(evt))
//    {
//      if(Nil::Data::has_mesh_resource(node))
//      {
//        m_pending_mesh_load.emplace_back(node);
//      }
//      
//      if(Nil::Data::has_camera(node))
//      {
//        m_pending_camera_node_updates.emplace_back(node);
//      }
//      
//      if(Nil::Data::has_material(node))
//      {
//        m_pending_renderable_node_updates.emplace_back(node);
//      }
//      
//      if(Nil::Data::has_graphics(node))
//      {
//        if(!m_has_initialized)
//        {
//          rov_initialize();
//        }
//      
//        m_has_initialized = true;
//      }
//    }
//    else
//    {
//      if(Nil::Data::has_camera(node))
//      {
//        m_pending_camera_node_removals.emplace_back(node);
//      }
//      
//      if(Nil::Data::has_material(node))
//      {
//        m_pending_renderable_node_removals.emplace_back(node);
//      }
//    }
//  }
//}

//
//void
//GL_Aspect::early_think(const float dt)
//{
//  if(m_has_initialized)
//  {
//    /*
//      Remove data first
//    */
//    {
//      for(size_t i = 0; i < m_pending_renderable_node_removals.size(); ++i)
//      {
//        for(size_t j = 0; j < m_renderable_nodes.size(); ++j)
//        {
//          if(m_renderable_nodes[j] == m_pending_renderable_node_removals[i])
//          {
//            m_renderable_nodes.erase(m_renderable_nodes.begin() + j);
//            m_renderables.erase(m_renderables.begin() + j);
//            break;
//          }
//        }
//      }
//      
//      for(size_t i = 0; i < m_pending_camera_node_removals.size(); ++i)
//      {
//        for(size_t j = 0; j < m_camera_nodes.size(); ++j)
//        {
//          if(m_camera_nodes[j] == m_pending_camera_node_removals[i])
//          {
//            m_camera_nodes.erase(m_camera_nodes.begin() + j);
//            m_rov_camera.erase(m_rov_camera.begin() + j);
//            break;
//          }
//        }
//      }
//      
//      m_pending_renderable_node_removals.clear();
//      m_pending_camera_node_removals.clear();
//    }
//    
//    /*
//      Update or insert new data
//    */
//    {
//      for(auto &node : m_pending_renderable_node_updates)
//      {
//        Nil::Data::Transform trans{};
//        Nil::Data::get(node, trans);
//      
//        Nil::Data::Material mat{};
//        Nil::Data::get(node, mat);
//      
//        ROV_Renderable rov_render
//        {
//          (uint8_t)mat.shader,
//          math::mat4_from_nil_transform(trans)
//        };
//        
//        memcpy(rov_render.color, mat.color, sizeof(Nil::Data::Material::color));
//        /*
//          Check to see if we have it already.
//        */
//        bool insert_new_data = true;
//        
//        for(size_t j = 0; j < m_renderable_nodes.size(); ++j)
//        {
//          if(m_renderable_nodes[j] == node)
//          {
//            m_renderables[j] = rov_render;
//            insert_new_data = false;
//            break;
//          }
//        }
//        
//        if(insert_new_data)
//        {
//          m_renderable_nodes.emplace_back(node);
//          m_renderables.emplace_back(rov_render);
//        }
//      }
//      
//      for(auto &node : m_pending_camera_node_updates)
//      {
//        Nil::Data::Transform trans{};
//        Nil::Data::get(node, trans);
//        
//        Nil::Data::Camera cam_data{};
//        Nil::Data::get(node, cam_data);
//      
//        uint32_t clear_flags = 0;
//        if(cam_data.clear_color_buffer) { clear_flags |= rovClearFlag_Color; }
//        if(cam_data.clear_depth_buffer) { clear_flags |= rovClearFlag_Depth; }
//
//        const ROV_Camera rov_camera
//        {
//          math::mat4_lookat_from_nil_transform(trans),
//          math::mat4_projection_from_nil_camera(cam_data),
//          clear_flags
//        };
//        
//        /*
//          Check to see if we have it already.
//        */
//        bool insert_new_data = true;
//        
//        for(size_t j = 0; j < m_camera_nodes.size(); ++j)
//        {
//          if(m_camera_nodes[j] == node)
//          {
//            m_rov_camera[j] = rov_camera;
//            insert_new_data = false;
//            break;
//          }
//        }
//        
//        if(insert_new_data)
//        {
//          m_camera_nodes.emplace_back(node);
//          m_rov_camera.emplace_back(rov_camera);
//        }
//      }
//      
//      m_pending_renderable_node_updates.clear();
//      m_pending_camera_node_updates.clear();
//    } // Update or insert data
//    
//    /*
//      Resources
//    */
//    {
//      for(auto &node : m_pending_mesh_load)
//      {
//        static int once = 0;
//        
//        if(once == 0)
//        {
//          Nil::Data::Mesh_resource mesh_resource{};
//          Nil::Data::get(node, mesh_resource);
//          
//          uint32_t mesh = rov_createMesh(mesh_resource.position_vec3, mesh_resource.normal_vec3, mesh_resource.texture_coords_vec2, mesh_resource.count);
//          
//          once = 1;
//        }
//      }
//    }
//  } // Has inited
//}


//void
//GL_Aspect::think(const float dt)
//{
//  if(m_has_initialized)
//  {
//    for(auto &cam : m_rov_camera)
//    {
//      rov_startRenderPass(math::mat4_get_data(cam.view), math::mat4_get_data(cam.proj), cam.clear_flags);
//      
//      for(auto &render : m_renderables)
//      {
//        rov_setColor(render.color[0], render.color[1], render.color[2], render.color[3]);
//        rov_setShader(render.shader_type);
//
//        rov_submitMeshTransform(math::mat4_get_data(render.world));
//      }
//    }
//        
//    rov_execute();
//  }
//}


} // ns


#endif // impl inc
#endif // impl guard


