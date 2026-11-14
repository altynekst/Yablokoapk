#ifdef TW_QUAD_TEXTURED
uniform sampler2D gTextureSampler;
#endif

#ifndef TW_QUAD_GROUPED
uniform vec4 gVertColors[TW_MAX_QUADS];
#else
uniform vec4 gVertColors[1];
#endif

noperspective in vec4 QuadColor;
#ifndef TW_QUAD_GROUPED
flat in int QuadIndex;
#else
#define QuadIndex 0
#endif
#ifdef TW_QUAD_TEXTURED
noperspective in vec2 TexCoord;
#endif

out vec4 FragClr;

uniform float gSunStrength;
uniform vec4 gSunColor;
vec4 sceneLight(vec4 C)
{
 if(gSunStrength <= 0.0) return C;
 vec3 Illumination = mix(vec3(1.0), gSunColor.rgb * 0.45 + vec3(0.78), gSunStrength);
 C.rgb = clamp(C.rgb * Illumination + C.rgb * (vec3(1.0) - C.rgb) * gSunStrength * 0.18, 0.0, 1.0);
 return C;
}

uniform int gWorldWaveCount;
uniform vec4 gWorldWaves[8];
uniform vec4 gWorldWaveParams[8];
uniform float gWaveAspect;
vec2 worldWaveOffset(vec2 P)
{
 vec2 Result = vec2(0.0);
 for(int I = 0; I < 8; ++I)
 {
  if(I >= gWorldWaveCount) break;
  vec4 W = gWorldWaves[I];
  vec2 D = vec2((P.x - W.x) * gWaveAspect, P.y - W.y);
  float Dist = length(D);
  if(Dist < 0.00001) continue;
  float O = (Dist - W.z) / max(gWorldWaveParams[I].x, 0.00001);
  float Wave = exp(-O * O) * (1.0 - O * O * 0.4);
  vec2 Shift = D / Dist * (Wave * clamp(W.w, 0.0, 2.0) * 0.02);
  Result += vec2(Shift.x / max(gWaveAspect, 0.00001), Shift.y);
 }
 return Result;
}

uniform vec4 gEffectViewport;
void main()
{
#ifdef TW_QUAD_TEXTURED
	vec2 ScreenPoint = (gl_FragCoord.xy - gEffectViewport.xy) / max(gEffectViewport.zw, vec2(1.0));
 ScreenPoint.y = 1.0 - ScreenPoint.y;
 vec2 Shift = worldWaveOffset(ScreenPoint) * gEffectViewport.zw;
 vec2 RefractedTex = TexCoord - dFdx(TexCoord) * Shift.x + dFdy(TexCoord) * Shift.y;
 vec4 TexColor = texture(gTextureSampler, RefractedTex);
	FragClr = TexColor * QuadColor * gVertColors[QuadIndex];
#else
	FragClr = QuadColor * gVertColors[QuadIndex];
#endif
	FragClr = sceneLight(FragClr);
}
