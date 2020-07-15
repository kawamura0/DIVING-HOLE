sampler tex0 : register(s0);

float AddU;
float AddV;


float4 PS_P0_Main(float2 UV : TEXCOORD0) : COLOR0
{
	float4 color;

    color = tex2D(tex0, UV + float2(0.0f,  0.0f)) * 0.36f;

	color += tex2D(tex0, UV + float2(-AddU, +AddV)) * 0.04;
	color += tex2D(tex0, UV + float2(0.0, +AddV)) * 0.04;
	color += tex2D(tex0, UV + float2(+AddU, +AddV)) * 0.04;
	color += tex2D(tex0, UV + float2(-AddU, 0.0)) * 0.04;
	color += tex2D(tex0, UV + float2(+AddU, 0.0))  * 0.04;

	color += tex2D(tex0, UV + float2(-AddU, -AddV)) * 0.04;
	color += tex2D(tex0, UV + float2(0.0, -AddV))  * 0.04;
	color += tex2D(tex0, UV + float2(+AddU, -AddV))  * 0.04;
	color += tex2D(tex0, UV + float2(-(AddU*2.0),+(AddV*2.0)))  * 0.02;
	color += tex2D(tex0, UV + float2(-AddU,+(AddV * 2.0)))  * 0.02;
	
	color += tex2D(tex0, UV + float2(0.0,+(AddV * 2.0)))  * 0.02;
	color += tex2D(tex0, UV + float2(+AddU,+(AddV * 2.0)))  * 0.02;
	color += tex2D(tex0, UV + float2(+(AddU *2.0),+(AddV * 2.0)))  * 0.02;
	color += tex2D(tex0, UV + float2(-(AddU * 2.0),+AddV)) * 0.02;
	color += tex2D(tex0, UV + float2(+(AddU*2.0),+AddV))  * 0.02;
	
	color += tex2D(tex0, UV + float2(-(AddU*2.0), 0.0))  * 0.02;
	color += tex2D(tex0, UV + float2(+(AddU*2.0), 0.0))  * 0.02;
	color += tex2D(tex0, UV + float2(-(AddU*2.0), -AddV))  * 0.02;
	color += tex2D(tex0, UV + float2(+(AddU*2.0), -AddV))  * 0.02;
	color += tex2D(tex0, UV + float2(-(AddU*2.0), -(AddV*2.0))) * 0.02;
	
	color += tex2D(tex0, UV + float2(-AddU, -(AddV*2.0)))  * 0.02;
	color += tex2D(tex0, UV + float2(0.0, -(AddV*2.0)))  * 0.02;
	color += tex2D(tex0, UV + float2(+AddU, -(AddV*2.0)))  * 0.02;
	color += tex2D(tex0, UV + float2(+(AddU*2.0), -(AddV*2.0)))  * 0.02;



	return color;
}

technique Blur
{
	pass P0
	{
		VertexShader = NULL;
		PixelShader = compile ps_3_0 PS_P0_Main();
	}
}