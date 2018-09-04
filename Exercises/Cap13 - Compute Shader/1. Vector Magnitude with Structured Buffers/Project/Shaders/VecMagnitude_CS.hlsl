
struct Data
{
	float3 v;
};

StructuredBuffer<Data> g_Input : register(t0);
RWStructuredBuffer<float> g_Output : register(u0);

[numthreads(64, 1, 1)]
void CS(int3 dtid : SV_DispatchThreadID)
{
	float3 v = g_Input[dtid.x].v;
	g_Output[dtid.x] = sqrt(v.x * v.x + v.y + v.y + v.z * v.z);
}