#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 morph;
uniform float mixFactor;

void main()
{
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(transpose(inverse(model))) * aNormal;  

  //gl_Position = projection * view * vec4(FragPos, 1.0);
  vec4 morphPos = morph * vec4(aPos, 1.0);
  vec4 mix = mix(vec4(aPos, 1.0), morphPos, mixFactor);
  gl_Position = projection * view * mix;
  TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
