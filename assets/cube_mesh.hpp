#ifndef CUBE_MESH_INCLUDED_FC128B6F_993F_4807_B7BE_4255CF3B1FDC
#define CUBE_MESH_INCLUDED_FC128B6F_993F_4807_B7BE_4255CF3B1FDC


#include <stdint.h>


namespace Nil_ext {
namespace Mesh {


constexpr uint32_t cube_mesh_vert_count = 36;


constexpr float cube_positions[]
{
  -0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f,  0.5f, -0.5f,
  0.5f,  0.5f, -0.5f,
  -0.5f,  0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,

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


constexpr float cube_normals[]
{
  0.f, 0.f, 1.f,
  0.f, 0.f, 1.f,
  0.f, 0.f, 1.f,
  0.f, 0.f, 1.f,
  0.f, 0.f, 1.f,
  0.f, 0.f, 1.f,
  
  0.f, 0.f, -1.f,
  0.f, 0.f, -1.f,
  0.f, 0.f, -1.f,
  0.f, 0.f, -1.f,
  0.f, 0.f, -1.f,
  0.f, 0.f, -1.f,
  
  1.f, 0.f, 0.f,
  1.f, 0.f, 0.f,
  1.f, 0.f, 0.f,
  1.f, 0.f, 0.f,
  1.f, 0.f, 0.f,
  1.f, 0.f, 0.f,
  
  -1.f, 0.f, 0.f,
  -1.f, 0.f, 0.f,
  -1.f, 0.f, 0.f,
  -1.f, 0.f, 0.f,
  -1.f, 0.f, 0.f,
  -1.f, 0.f, 0.f,
  
  0.f, 1.f, 0.f,
  0.f, 1.f, 0.f,
  0.f, 1.f, 0.f,
  0.f, 1.f, 0.f,
  0.f, 1.f, 0.f,
  0.f, 1.f, 0.f,
  
  0.f, -1.f, 0.f,
  0.f, -1.f, 0.f,
  0.f, -1.f, 0.f,
  0.f, -1.f, 0.f,
  0.f, -1.f, 0.f,
  0.f, -1.f, 0.f,
};


constexpr float cube_texture_coords[]
{
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


} // ns
} // ns


#endif // inc guard
