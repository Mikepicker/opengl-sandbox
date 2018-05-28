#version 330 core
out vec4 FragColor;

in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;

  // Normal mapping
  vec3 TangentLightPos;
  vec3 TangentViewPos;
  vec3 TangentFragPos;
} fs_in;

struct Material {
  sampler2D diffuseMap;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

uniform vec3 viewPos;

// Shadows
uniform samplerCube shadowMap;
uniform bool softShadows;
uniform bool hasShadows;
uniform float far_plane;
uniform float bias;

// Normal mapping
uniform bool hasNormalMap;
uniform sampler2D normalMap;

// Specular map
uniform bool hasSpecularMap;
uniform sampler2D specularMap;

// Alpha masking
uniform bool hasMaskMap;
uniform sampler2D maskMap;

float ShadowCalculation(vec3 fragPos)
{
  vec3 fragToLight = fragPos - light.position;
  float closestDepth = texture(shadowMap, fragToLight).r;
  closestDepth *= far_plane;

  float currentDepth = length(fragToLight);

  float shadow = 0.0;

  if (!softShadows)
  {
    // Hard shadows
    shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
  }
  else
  {
    // Soft shadows
    vec3 sampleOffsetDirections[20] = vec3[]
    (
     vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
     vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
     vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
     vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
     vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );

    int samples  = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
      float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
      closestDepth *= far_plane;   // Undo mapping [0;1]
      if(currentDepth - bias > closestDepth)
        shadow += 1.0;
    }
    shadow /= float(samples);  
  }
      
  FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

  return shadow;
}

vec3 computeNormal()
{
  // obtain normal from normal map in range [0,1]
  vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;

  // transform normal vector to range [-1,1]
  normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

  return normal;
}

void main()
{           
  // Alpha masking
  if (hasMaskMap) {
    vec4 alpha = texture(maskMap, fs_in.TexCoords).rgba;
    FragColor = vec4(0.0, alpha.a, 0.0, 1.0);
    if (alpha.r < 0.1)
      discard;
  }

  vec3 color = texture(material.diffuseMap, fs_in.TexCoords).rgb;
  vec3 normal = hasNormalMap ? computeNormal() : normalize(fs_in.Normal);

  // Ambient
  vec3 ambient = material.ambient * light.ambient * color;

  // Diffuse
  vec3 lightDir = hasNormalMap ? normalize(fs_in.TangentLightPos - fs_in.TangentFragPos) : normalize(light.position - fs_in.FragPos);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * material.diffuse * color * light.diffuse;

  // Specular
  vec3 viewDir = hasNormalMap ? normalize(fs_in.TangentViewPos - fs_in.TangentFragPos) : normalize(viewPos - fs_in.FragPos);
  float spec = 0.0;
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  vec3 specular = spec * material.specular * light.specular;    

  if (hasSpecularMap)
    specular *= texture(specularMap, fs_in.TexCoords).rgb;

  // Calculate shadow
  float shadow = hasShadows ? ShadowCalculation(fs_in.FragPos) : 0.0;
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
  
  FragColor = vec4(lighting, 1.0);
  //FragColor = vec4(texture(normalMap, fs_in.TexCoords).rgb, 1.0);
  //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  //FragColor = vec4(texture(shadowMap, fs_in.TexCoords).xy, 0.0, 1.0);
}
