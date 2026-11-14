layout (location = 0) in vec2 inVertex;
#ifdef TW_TILE_TEXTURED
layout (location = 1) in uvec4 inVertexTexCoord;
#endif

uniform mat4x2 gPos;

uniform vec2 gOffset;
uniform vec2 gScale;

#ifdef TW_TILE_TEXTURED
noperspective out vec3 TexCoord;
#endif

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

void main()
{
	// scale then position vertex
	vec2 VertexPos = (inVertex * gScale) + gOffset;
	gl_Position = vec4(gPos * vec4(VertexPos, 0.0, 1.0), 0.0, 1.0);

 vec2 ScreenPoint = vec2((gl_Position.x + 1.0) * 0.5, (1.0 - gl_Position.y) * 0.5);
 vec2 Shift = worldWaveOffset(ScreenPoint);
 gl_Position.xy += vec2(Shift.x * 2.0, -Shift.y * 2.0);

#ifdef TW_TILE_TEXTURED
	// scale the texture coordinates too
	vec2 TexScale = gScale;
	if (float(inVertexTexCoord.w) > 0.0)
		TexScale = gScale.yx;
	TexCoord = vec3(vec2(inVertexTexCoord.xy) * TexScale, float(inVertexTexCoord.z));
#endif
}
