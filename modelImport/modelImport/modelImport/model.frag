#version 330 core

in VS_OUTT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TextCoord;
	vec4 FragPosLightSpace;
} fs_in;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_specular3;
uniform sampler2D shadowMap;

uniform vec3 object_Color;
uniform vec3 viewPos;
uniform vec3 lightPosition;

out vec4 color;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 将坐标范围变换到0到1
    projCoords = projCoords * 0.5 + 0.5;
    // 依据坐标去纹理中找到对应的最小深度值
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 获取当前点实际的深度值
	float currentDepth;
	currentDepth= projCoords.z;
	vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPosition - fs_in.FragPos);
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
	float shadow = 0.0;
    vec2 texelSize = vec2(1.0/800,1.0/600);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
	shadow /= 9.0;
	//shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0; 
	if(projCoords.z > 1.0)
        shadow = 1.0;
    return shadow;
}


void main()
{

	
	vec3	lightColor = vec3(1.0);
	vec3    fragcolor=texture(texture_diffuse1, fs_in.TextCoord).rgb;
	vec3	ambient = 0.1*fragcolor ;
	vec3	lightDir = normalize(lightPosition - fs_in.FragPos);
	vec3	normal = normalize(fs_in.Normal);
	float	diffFactor = max(dot(lightDir, normal), 0.0);
	vec3	diffuse = diffFactor * lightColor;

	float	specularStrength = 0.5f;
	vec3	reflectDir = normalize(reflect(-lightDir, normal));
	vec3	viewDir = normalize(viewPos - fs_in.FragPos);
	float	specFactor = pow(max(dot(reflectDir, viewDir), 0.0), 64.0f);
	vec3	specular = specFactor * lightColor;
	
	float 	shadow = ShadowCalculation(fs_in.FragPosLightSpace);
	shadow = min(shadow, 0.75);
	//vec3 result = ambient * fragcolor + (diffuse + specular) * fragcolor;
	vec3 result = (ambient + (1.0-shadow)*(diffuse+specular))*fragcolor;
	color	= vec4(result , 1.0f);
}