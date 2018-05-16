#version 330 core
out vec4 FragColor;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
};

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoord;
  
uniform Light light;
uniform vec3 viewPos;

uniform sampler2D heightmap;
uniform sampler2D grass;
uniform sampler2D snow;
uniform sampler2D dirt;

void main()
{    
  // Texture mix
  float h = texture(heightmap, TexCoord).r;
  
  vec3 texMix;
  if (h < 0.5)
  {
    texMix = mix(texture(grass, TexCoord).rgb, texture(dirt, TexCoord).rgb, h * 2);
  } else
  {
    texMix = mix(texture(dirt, TexCoord).rgb, texture(snow, TexCoord).rgb, (h*2) - 1);
  }

  // ambient
  vec3 ambient = light.ambient * texMix;
  
  // diffuse 
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.position - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * texMix;

  vec3 result = ambient + diffuse;
  FragColor = vec4(result, 1.0);
  //FragColor = vec4(Normal, 1.0);
}
