#include"BasicShaderHeader.hlsli"

BasicType BasicVS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD)
{
    BasicType output; //ピクセルシェーダへ渡す値
    pos = mul(world, pos);
    output.svpos = mul(mul(proj, view), pos);           //シェーダでは列優先なので注意
    output.pos = mul(view, pos);
    normal.w = 0;                                       //ここ重要(平行移動成分を無効にする)
    output.normal = mul(world, normal);                 //法線にもワールド変換を行う
    output.vnormal = mul(view, output.normal);
    output.uv = uv;
    output.ray = normalize(pos.xyz - mul(view, look)); //視線ベクトル

    return output;
}

