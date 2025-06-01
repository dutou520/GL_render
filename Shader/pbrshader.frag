#version 330 core
#define PI 3.141592653589793
in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;
in vec2 TexCoords;

out vec4 FragColor;

// PBR材质属性
uniform vec3 albedoColor;    // 基础颜色
uniform float metallic;      // 金属度
uniform float roughness;     // 粗糙度
uniform float ao;            // 环境光遮蔽
uniform vec3 emissionColor;  // 自发光颜色

// 纹理贴图
uniform sampler2D albedoMap;    // 反照率贴图
uniform sampler2D normalMap;    // 法线贴图
uniform sampler2D metallicMap; // 金属度贴图
uniform sampler2D roughnessMap; // 粗糙度贴图
uniform sampler2D aoMap;       // 环境光遮蔽贴图
uniform sampler2D emissionMap; // 自发光贴图
//是否启用纹理贴图
uniform bool useAlbedoMap;//反照率贴图
uniform bool useNormalMap;//法线贴图
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useAOMap;
uniform bool useEmissionMap;
// 光源属性
uniform vec3 lightColor;     // 光源颜色

// 相机位置
uniform vec3 viewPos;        // 观察者位置

// PBR光照计算函数
vec3 calculatePBR(vec3 albedo, float metallic, float roughness, float ao, vec3 emission, vec3 N, vec3 V, vec3 L) {
    // 金属度影响
    vec3 diffuseColor = albedo * (1.0 - metallic);
    vec3 specularColor = mix(vec3(0.04), albedo, metallic);
    
    // 粗糙度影响
    float alpha = roughness * roughness;
    
    // 标准化向量
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float LdotH = max(dot(L, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);
    
    // 法线分布函数 (Trowbridge-Reitz GGX)
    float D = (alpha * alpha) / (PI * pow((NdotH * NdotH * (alpha * alpha - 1.0) + 1.0), 2.0));
    
    // 几何遮蔽函数 (Smith-Schlick GGX)
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float G1 = NdotV / (NdotV * (1.0 - k) + k);
    float G2 = NdotL / (NdotL * (1.0 - k) + k);
    float G = G1 * G2;
    
    // 菲涅尔方程 (Fresnel-Schlick)
    vec3 F = specularColor + (1.0 - specularColor) * pow(1.0 - VdotH, 5.0);
    
    // 漫反射项
    vec3 diffuse = diffuseColor / PI;
    
    // 镜面反射项 (Cook-Torrance BRDF)
    float denominator = 4.0 * NdotV * NdotL;
    denominator = max(denominator, 0.0001);
    vec3 specular = (D * G * F) / denominator;
    
    // 组合结果
    vec3 color = (diffuse + specular) * lightColor * NdotL;
    color = color * ao + emission;
    
    return color;
}

void main() {
    // 获取材质参数（根据是否使用贴图选择贴图或默认值）
    vec3 albedo = useAlbedoMap ? texture(albedoMap, TexCoords).rgb * albedoColor : albedoColor;
    vec3 normal = useNormalMap ? texture(normalMap, TexCoords).rgb * 2.0 - 1.0 : Normal;
    // 这里假设法线贴图已经是世界空间法线，如果是切线空间法线需要转换
    normal = normalize(normal); 
    float metallicVal = useMetallicMap ? texture(metallicMap, TexCoords).r * metallic : metallic;
    float roughnessVal = useRoughnessMap ? texture(roughnessMap, TexCoords).r * roughness : roughness;
    float aoVal = useAOMap ? texture(aoMap, TexCoords).r * ao : ao;
    vec3 emissionVal = useEmissionMap ? texture(emissionMap, TexCoords).rgb * emissionColor : emissionColor;
    
    // 标准化向量
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(LightPos - FragPos);
    
    // 计算PBR光照
    vec3 color = calculatePBR(albedo, metallicVal, roughnessVal, aoVal, emissionVal, normal, viewDir, lightDir);
    
    FragColor = vec4(normal, 1.0);
}