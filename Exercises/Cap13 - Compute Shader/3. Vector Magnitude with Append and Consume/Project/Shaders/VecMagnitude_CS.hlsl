/**
 * Computes the magnitude of the input vectors.
 */

struct Data
{
	float3 v;
};

ConsumeStructuredBuffer<Data> g_Input : register(u0);
AppendStructuredBuffer<Data> g_Output : register(u1);

[numthreads(64, 1, 1)]
void CS()
{
	Data data = g_Input.Consume();
	//float magnitude = sqrt(v.x * v.x + v.y + v.y + v.z * v.z);
    //float magnitude = 1.0f;
	g_Output.Append(data);
}