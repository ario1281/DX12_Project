#include"BasicShaderHeader.hlsli"

float4 BasicPS(BasicType input) : SV_TARGET {
    float3 light = normalize(float3(1, -1, 1)); //光の向かうベクトル(平行光線)
    float3 lightColor = float3(1, 1, 1); //ライトのカラー(1,1,1で真っ白)

	//ディフューズ計算
    float diffuseB = saturate(dot(-light, input.normal));
    float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));

	//光の反射ベクトル
    float3 refLight = normalize(reflect(light, input.normal.xyz));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);
	
	//スフィアマップ用UV
    float2 sphereMapUV = input.vnormal.xy;
    sphereMapUV = ((sphereMapUV + float2(1.0, -1.0)) / float2(2.0, 2.0)) * float2(1.0, -1.0);

    float4 texColor = tex.Sample(smp, input.uv); //テクスチャカラー

    return max(saturate(toonDif //輝度(トゥーン)
		* diffuse //ディフューズ色
		* texColor //テクスチャカラー
		* sph.Sample(smp, sphereMapUV)) //スフィアマップ(乗算)
		+ saturate(spa.Sample(smp, sphereMapUV) * texColor //スフィアマップ(加算)
		+ float4(specularB * specular.rgb, 1)) //スペキュラー
		, float4(texColor * ambient, 1)); //アンビエント
}