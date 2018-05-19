#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 lightPositionOnScreen;
uniform sampler2D firstPass;
const int NUM_SAMPLES = 50;

void main()
{	
  FragColor = vec4(0.0);
  vec2 deltaTextCoord = vec2(TexCoords - lightPositionOnScreen);
  vec2 texCoo = TexCoords;
  deltaTextCoord *= 1.0 /  float(NUM_SAMPLES) * density;
  float illuminationDecay = 1.0;

  for(int i=0; i < NUM_SAMPLES; i++)
  {
    texCoo -= deltaTextCoord;
    vec4 sample = texture2D(firstPass, texCoo);

    sample *= illuminationDecay * weight;

    FragColor += sample;

    illuminationDecay *= decay;
  }
  FragColor *= exposure;
}
