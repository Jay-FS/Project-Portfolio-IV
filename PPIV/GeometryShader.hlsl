
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

struct GS_INPUT
{
    float4 Pos : POSITION;
};

struct GS_OUTPUT
{
    float4 pos : SV_POSITION;
};

//0  1  z towards me
//2  3

[maxvertexcount(4)]
void main(
	point GS_INPUT input[1] : SV_POSITION, 
	inout TriangleStream< GS_OUTPUT > output
)
{
    //GS_OUTPUT element[4] = { (GS_OUTPUT) 0 };

    //for (int i = 0; i < 4; ++i)
    //{
    //    element[i].pos = input[i].Pos;
    //}
    //element[0].pos.x -= 1.0f;
    //element[0].pos.y += 1.0f;

    //element[1].pos.x += 1.0f;
    //element[1].pos.y += 1.0f;

    //element[2].pos.x -= 1.0f;
    //element[2].pos.y -= 1.0f;
    
    //element[3].pos.x += 1.0f;
    //element[3].pos.y -= 1.0f;

    //for (int i = 0; i < 4; ++i)
    //{
    //    element[i].pos = mul(element[i].pos, World);
    //    element[i].pos = mul(element[i].pos, View);
    //    element[i].pos = mul(element[i].pos, Projection);
    //    output.Append(element[i]);
    //}
}