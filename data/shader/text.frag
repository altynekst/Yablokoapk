uniform sampler2D gTextSampler;
uniform sampler2D gTextOutlineSampler;

uniform vec4 gVertColor;
uniform vec4 gVertOutlineColor;

noperspective in vec2 texCoord;
noperspective in vec4 outVertColor;

out vec4 FragClr;

uniform int gLensCount;
uniform vec4 gLensRects[16];
uniform vec4 gLensParams[16];
uniform float gLensStrength;
uniform vec4 gEffectViewport;
float foregroundBlur()
{
 if(gLensCount <= 0 || gLensStrength <= 0.0) return 0.0;
 vec2 P = (gl_FragCoord.xy - gEffectViewport.xy) / max(gEffectViewport.zw, vec2(1.0));
 P.y = 1.0 - P.y;
 float Amount = 0.0;
 for(int I = 0; I < 16; ++I)
 {
  if(I >= gLensCount) break;
  vec4 R = gLensRects[I];
  if(gLensParams[I].x < 0.5 || gLensParams[I].x > 2.5) continue;
  vec2 Q = (abs(P - R.xy - R.zw * 0.5) - R.zw * 0.5) * gEffectViewport.zw;
  float Radius = min(R.z * gEffectViewport.z, R.w * gEffectViewport.w) * 0.15;
  vec2 Rounded = Q + vec2(Radius);
  float D = length(max(Rounded, vec2(0.0))) + min(max(Rounded.x, Rounded.y), 0.0) - Radius;
  if(D < 0.0)
  {
   float Edge = 1.0 - smoothstep(0.0, max(5.0, Radius * 1.8), -D);
   Amount = max(Amount, (0.6 + 4.5 * Edge) * smoothstep(0.0, 1.3, -D) * gLensParams[I].y);
  }
 }
 return Amount * gLensStrength;
}
vec4 premultiply(vec4 C) { return vec4(C.rgb * C.a, C.a); }
vec4 sampleForeground(sampler2D Source, vec2 UV)
{
 float Radius = foregroundBlur();
 if(Radius < 0.01) return texture(Source, UV);
 vec2 X = dFdx(UV) * Radius, Y = dFdy(UV) * Radius;
 vec4 C = premultiply(texture(Source, UV)) * 4.0;
 C += (premultiply(texture(Source, UV+X)) + premultiply(texture(Source, UV-X)) + premultiply(texture(Source, UV+Y)) + premultiply(texture(Source, UV-Y))) * 2.0;
 C += premultiply(texture(Source, UV+X+Y)) + premultiply(texture(Source, UV-X+Y)) + premultiply(texture(Source, UV+X-Y)) + premultiply(texture(Source, UV-X-Y));
 C /= 16.0;
 return C.a > 0.0001 ? vec4(C.rgb / C.a, C.a) : vec4(0.0);
}
float sampleForegroundMask(sampler2D Source, vec2 UV)
{
 float Radius = foregroundBlur();
 if(Radius < 0.01) return texture(Source, UV).r;
 vec2 X = dFdx(UV) * Radius, Y = dFdy(UV) * Radius;
 float C = texture(Source, UV).r * 4.0;
 C += (texture(Source, UV+X).r + texture(Source, UV-X).r + texture(Source, UV+Y).r + texture(Source, UV-Y).r) * 2.0;
 C += texture(Source, UV+X+Y).r + texture(Source, UV-X+Y).r + texture(Source, UV+X-Y).r + texture(Source, UV-X-Y).r;
 return C / 16.0;
}

uniform float gSunStrength;
uniform vec4 gSunColor;
vec4 sceneLight(vec4 C)
{
 if(gSunStrength <= 0.0) return C;
 vec3 Illumination = mix(vec3(1.0), gSunColor.rgb * 0.45 + vec3(0.78), gSunStrength);
 C.rgb = clamp(C.rgb * Illumination + C.rgb * (vec3(1.0) - C.rgb) * gSunStrength * 0.18, 0.0, 1.0);
 return C;
}

void main()
{
	vec4 textColor = gVertColor * outVertColor * vec4(1.0, 1.0, 1.0, sampleForegroundMask(gTextSampler, texCoord));
	vec4 textOutlineTex = gVertOutlineColor * vec4(1.0, 1.0, 1.0, sampleForegroundMask(gTextOutlineSampler, texCoord));

	// ratio between the two textures
	float OutlineBlend = (1.0 - textColor.a);	

	// since the outline is always black, or even if it has decent colors, it can be just added to the actual color
	// without losing any or too much color
	
	// lerp isn't commutative, so add the color the fragment looses by lerping
	// this reduces the chance of false color calculation if the text is transparent
	
	// first get the right color
	vec4 textOutlineFrag = vec4(textOutlineTex.rgb * textOutlineTex.a, textOutlineTex.a) * OutlineBlend;
	vec3 textFrag = (textColor.rgb * textColor.a);
	vec3 finalFragColor = textOutlineFrag.rgb + textFrag;
	
	float RealAlpha = (textOutlineFrag.a + textColor.a);
	
	// simply add the color we will loose through blending
	if(RealAlpha > 0.0)
		FragClr = vec4(finalFragColor / RealAlpha, RealAlpha);
	else
		FragClr = vec4(0.0, 0.0, 0.0, 0.0);
	FragClr = sceneLight(FragClr);
}
