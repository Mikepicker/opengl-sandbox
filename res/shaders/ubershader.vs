#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;

  // Normal mapping
  vec3 TangentLightPos;
  vec3 TangentViewPos;
  vec3 TangentFragPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Normal mapping
uniform bool hasNormalMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float time;

void main()
{
  vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
  vs_out.TexCoords = aTexCoords;

  // Calculate TBN for normal mapping
  if (hasNormalMap)
  {
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));    
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
  }
  else
  {
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
  }

  float s = (sin(time) + 1) / 2;
  float c = (cos(time) + 1) / 2;
  vec3 pos = vec3(aPos.x * s, aPos.y * s, aPos.z);
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
