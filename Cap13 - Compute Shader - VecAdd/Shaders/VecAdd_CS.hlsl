
struct Data
{
	float3 v1;
	float3 v2;
};

StructuredBuffer<Data> g_InputA : register(t0);
StructuredBuffer<Data> g_InputB : register(t1);
RWStructuredBuffer<Data> g_Output : register(u0);

[numthreads(32, 1, 1)]
void CS(int3 dtid : SV_DispatchThreadID)
{
	g_Output[dtid.x].v1 = g_InputA[dtid.x].v1 + g_InputB[dtid.x].v1;
	g_Output[dtid.x].v2 = g_InputA[dtid.x].v2 + g_InputB[dtid.x].v2;
}