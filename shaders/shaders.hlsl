cbuffer FrameData : register(b0) {
  float2 viewportSize;
  float2 _padding;
};

struct VSOutput {
  float4 position : SV_POSITION;
  float4 color    : COLOR;
};

VSOutput VSMain(
  float2 corner    : POSITION,
  float2 instPos   : INSTANCE_POS,
  float2 instSize  : INSTANCE_SIZE,
  float4 instColor : INSTANCE_COLOR)
{
  VSOutput o;

  float2 pixelPos = instPos + corner * instSize;

  // Pixel Y increases downward; NDC Y increases upward — flip the sign.
  float2 ndc;
  ndc.x =  (pixelPos.x / viewportSize.x) * 2.0f - 1.0f;
  ndc.y = -((pixelPos.y / viewportSize.y) * 2.0f - 1.0f);

  o.position = float4(ndc, 0.0f, 1.0f);
  o.color    = instColor;
  return o;
}

float4 PSMain(VSOutput input) : SV_TARGET {
  return input.color;
}
