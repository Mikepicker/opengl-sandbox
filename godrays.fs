#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;

uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 lightPositionOnScreen;
uniform sampler2D firstPass;
const int NUM_SAMPLES = 100;

void main()
{	
  vec2 deltaTextCoord = vec2(gl_TexCoord[0].st - lightPositionOnScreen.xy);
  vec2 textCoo = gl_TexCoord[0].st;
  deltaTextCoord *= 1.0 /  float(NUM_SAMPLES) * density;
  float illuminationDecay = 1.0;


  for(int i=0; i < NUM_SAMPLES ; i++)
  {
    textCoo -= deltaTextCoord;
    vec4 sample = texture2D(firstPass, textCoo );

    sample *= illuminationDecay * weight;

    gl_FragColor += sample;

    illuminationDecay *= decay;
  }
  gl_FragColor *= exposure;
}
