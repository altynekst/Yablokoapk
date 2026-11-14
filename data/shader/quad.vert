layout (location = 0) in vec4 inVertex;
layout (location = 1) in vec4 inColor;
#ifdef TW_QUAD_TEXTURED
layout (location = 2) in vec2 inVertexTexCoord;
#endif

uniform mat4x2 gPos;

#ifdef TW_QUAD_GROUPED
uniform vec2 gOffsets[1];
uniform float gRotations[1];
#else
uniform vec2 gOffsets[TW_MAX_QUADS];
uniform float gRotations[TW_MAX_QUADS];
uniform int gQuadOffset;
flat out int QuadIndex;
#endif

noperspective out vec4 QuadColor;

#ifdef TW_QUAD_TEXTURED
noperspective out vec2 TexCoord;
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
	vec2 FinalPos = vec2(inVertex.xy);

#ifndef TW_QUAD_GROUPED
	int TmpQuadIndex = int(gl_VertexID / 4) - gQuadOffset;
#else
#define TmpQuadIndex 0
#endif
	if(gRotations[TmpQuadIndex] != 0.0)
	{
		float X = FinalPos.x - inVertex.z;
		float Y = FinalPos.y - inVertex.w;
		
		FinalPos.x = X * cos(gRotations[TmpQuadIndex]) - Y * sin(gRotations[TmpQuadIndex]) + inVertex.z;
		FinalPos.y = X * sin(gRotations[TmpQuadIndex]) + Y * cos(gRotations[TmpQuadIndex]) + inVertex.w;
	}
	FinalPos += gOffsets[TmpQuadIndex];

#ifndef TW_QUAD_GROUPED
	QuadIndex = TmpQuadIndex;
#endif

	gl_Position = vec4(gPos * vec4(FinalPos, 0.0, 1.0), 0.0, 1.0);
 vec2 ScreenPoint = vec2((gl_Position.x + 1.0) * 0.5, (1.0 - gl_Position.y) * 0.5);
 vec2 Shift = worldWaveOffset(ScreenPoint);
 gl_Position.xy += vec2(Shift.x * 2.0, -Shift.y * 2.0);
	QuadColor = inColor;

#ifdef TW_QUAD_TEXTURED
	TexCoord = inVertexTexCoord;
#endif
}
