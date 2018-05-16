#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform sampler2D heightmap;
uniform float elevation;

float height(vec2 i)
{
  float h = texture(heightmap, i).r;
  return (h*2)-1;
}

void main()
{
  vec3 elevPos = vec3(aPos.x, height(aPos.xz) * elevation, aPos.z);

  FragPos = vec3(model * vec4(elevPos, 1.0));
  TexCoord = aPos.xz;
  gl_Position = projection * view * model * vec4(elevPos, 1.0);

  // Central difference method to compute normals
  vec3 off = vec3(1.2, 0.0, 1.2);
  float hL = height(aPos.xz - off.xy);
  float hR = height(aPos.xz + off.xy);
  float hD = height(aPos.xz - off.yz);
  float hU = height(aPos.xz + off.yz);
  
  // Deduce terrain normal
  Normal = normalize(vec3(hL - hR, 2.0, hD - hU));
}
