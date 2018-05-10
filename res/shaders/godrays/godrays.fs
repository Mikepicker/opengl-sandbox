#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 lightPositionOnScreen;
uniform sampler2D firstPass;
const int NUM_SAMPLES = 100;

void main()
{	
  FragColor = vec4(0.0);
  //vec2 deltaTextCoord = vec2(TexCoords - vec2(1.0, 1.0));
  vec2 deltaTextCoord = vec2(TexCoords - lightPositionOnScreen);
  vec2 texCoo = TexCoords;
  deltaTextCoord *= 1.0 /  float(NUM_SAMPLES) * density;
  float illuminationDecay = 1.0;

  for(int i=0; i < NUM_SAMPLES; i++)
  {
    texCoo -= deltaTextCoord;
    //vec4 sample = texture(firstPass, vec4(clamp(texCoo,0,1), 0));
    vec4 sample = texture2D(firstPass, texCoo);

    sample *= illuminationDecay * weight;

    FragColor += sample;

    illuminationDecay *= decay;
  }
  FragColor *= exposure;
}
//FragColor = texture(screenTexture, TexCoords);
//FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
