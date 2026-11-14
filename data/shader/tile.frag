#ifdef TW_TILE_TEXTURED
#ifdef TW_TILE_3D_TEXTURED
uniform sampler3D gTextureSampler;
#else
uniform sampler2DArray gTextureSampler;
#endif
#endif

uniform vec4 gVertColor;

#ifdef TW_TILE_TEXTURED
noperspective in vec3 TexCoord;
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


uniform vec4 gEffectViewport;
uniform int gDamageCount;
uniform vec4 gDamageRects[64];
uniform vec4 gDamageBounds;
uniform vec4 gSceneWorld;
float tileRestoration()
{
 if(gDamageCount <= 0) return 1.0;
 vec2 P = (gl_FragCoord.xy - gEffectViewport.xy) / max(gEffectViewport.zw, vec2(1.0));
 P.y = 1.0 - P.y;
 vec2 World = mix(gSceneWorld.xy, gSceneWorld.zw, P);
 if(any(lessThan(World,gDamageBounds.xy)) || any(greaterThan(World,gDamageBounds.zw))) return 1.0;
 float Damage = 0.0;
 for(int I = 0; I < 64; ++I)
 {
  if(I >= gDamageCount) break;
  vec4 R = gDamageRects[I];
  vec2 D = abs(World - R.xy);
  float Inside = 1.0 - smoothstep(R.z - 0.4, R.z + 0.4, max(D.x, D.y));
  Damage = max(Damage, Inside * R.w);
 }
 return 1.0 - Damage;
}

void main()
{
#ifdef TW_TILE_TEXTURED
	vec4 TexColor = texture(gTextureSampler, TexCoord.xyz);
	FragClr = TexColor * gVertColor;
#else
	FragClr = gVertColor;
#endif
	FragClr = sceneLight(FragClr);
	FragClr.a *= tileRestoration();
}
