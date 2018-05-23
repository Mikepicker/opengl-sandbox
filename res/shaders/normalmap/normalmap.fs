#version 330 core
out vec4 FragColor;

in VS_OUT {
  vec3 FragPos;
  vec2 TexCoords;
  vec3 TangentLightPos;
  vec3 TangentViewPos;
  vec3 TangentFragPos;
} fs_in;

struct Light {
  vec3 diffuse;
  vec3 ambient;
  vec3 specular;
}; 

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform Light light;
uniform float shininess;

void main()
{           
  // obtain normal from normal map in range [0,1]
  vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;

  // transform normal vector to range [-1,1]
  normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

  // get diffuse color
  vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;

  // ambient
  vec3 ambient = light.ambient * color;

  // diffuse
  vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = light.diffuse * diff * color;

  // specular
  vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

  vec3 specular = light.specular * spec;

  FragColor = vec4(ambient + diffuse + specular, 1.0);
  //FragColor = vec4(texture(diffuseMap, fs_in.TexCoords).rgb, 1.0);
  //FragColor = vec4(texture(normalMap, fs_in.TexCoords).rgb, 1.0);
  //FragColor = vec4(diff, 0.0, 0.0, 0.0);
  //FragColor = vec4(fs_in.TangentLightPos.r, 0.0, 0.0, 1.0);
  //FragColor = vec4(diffuse, 1.0);
}
