#ifndef MATH_NIL_EXTENSIONS_INCLUDED_BE44ABF0_26B5_40A1_B384_D71D09A5C58A
#define MATH_NIL_EXTENSIONS_INCLUDED_BE44ABF0_26B5_40A1_B384_D71D09A5C58A


#include <math/math.hpp>
#include <nil/data/transform.hpp>
#include <nil/data/camera.hpp>


namespace math {


// ---------------------------------------- [ Math Extensions World Vectors ] --


inline math::vec3
vec3_nil_world_up()
{
  return math::vec3_init(0,1,0);
}


inline math::vec3
vec3_nil_world_fwd()
{
  return math::vec3_init(0,0,1);
}


// ------------------------------------- [ Math Extensions Mat4 Conversions ] --


inline math::mat4
mat4_from_nil_transform(const Nil::Data::Transform &trans)
{
  const math::vec3 pos = math::vec3_init_with_array(trans.position);
  const math::vec3 scale = math::vec3_init_with_array(trans.scale);
//  const math::quat rot = math::quat_init(trans.rotation[0], trans.rotation[1], trans.rotation[2], trans.rotation[3]);

  const math::mat4 pos_mat = math::mat4_translate(pos);
  const math::mat4 scale_mat = math::mat4_scale(scale);
  const math::mat4 rot_mat = math::mat4_id();
  // ROTATION!!!

  return math::mat4_multiply(scale_mat, rot_mat, pos_mat);
}


inline math::mat4
mat4_lookat_from_nil_transform(const Nil::Data::Transform &trans)
{
  const math::vec3 cam_pos  = math::vec3_init_with_array(trans.position);
  const math::quat cam_rot  = math::quat_init(trans.rotation[0], trans.rotation[1], trans.rotation[2], 1);
  const math::vec3 cam_fwd  = math::quat_rotate_point(cam_rot, vec3_nil_world_fwd());
  const math::vec3 look_fwd = math::vec3_add(cam_pos, cam_fwd);

  return math::mat4_lookat(cam_pos, look_fwd, vec3_nil_world_up()); // This will crash if lookat vector is up.
}


inline math::mat4
mat4_projection_from_nil_camera(const Nil::Data::Camera &cam)
{
  if(cam.type == Nil::Data::Camera::PERSPECTIVE)
  {
    return math::mat4_projection(
      cam.width,
      cam.height,
      cam.near_plane,
      cam.far_plane,
      cam.fov
    );
  }
  else
  {
    return math::mat4_orthographic(
      cam.width,
      cam.height,
      cam.near_plane,
      cam.far_plane
    );
  }
}


} // ns


#endif // inc guard
