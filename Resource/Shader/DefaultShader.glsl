#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aUV;
layout (location = 4) in vec4 aColor;


out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

//uniform mat4 uProj;
//uniform mat4 uView;
uniform mat4 uModel;
//uniform mat4 uBoneTransforms[100];

struct Light
{
    vec3 position;
    float pad1;
    vec3 ambient;
    float pad2;
    vec3 diffuse;
    float pad3;
    vec3 specular;
    float pad4;
};

layout (std140, binding = 0) uniform UBOData
{
    mat4 uProj;
    mat4 uView;
    vec3 uViewPos;
    float pad5;  
    Light lights[50];
    uint lightCount;
    vec3 pad6; 
};

void main()
{
    
    gl_Position = uProj * uView  * uModel  *vec4(aPos, 1.f);
    FragPos = vec3( uView * uModel * vec4(aPos, 1.f));
    TexCoords = aUV;
    Normal = normalize(mat3(transpose(inverse(uView * uModel))) * aNormal);
}


////////////////////////////////////////////////////////////////////////////////

#version 460 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};


uniform Material material;

//uniform samplerCube uCubeTexture[50];
//uniform sampler2D uTexture2D[50];

uniform samplerCube uCubeTexture;
uniform sampler2D uTexture2D;
uniform sampler2D uNormalTexture;

//uniform int uCubeTexIdx;
//uniform int uTex2DIdx;

in vec3 FragPos;
in vec3 Normal;

in vec2 TexCoords;

out vec4 FragColor;

struct Light
{
    vec3 position;
    float pad1;
    vec3 ambient;
    float pad2;
    vec3 diffuse;
    float pad3;
    vec3 specular;
    float pad4;
};

layout (std140, binding = 0) uniform UBOData
{
    mat4 uProj;
    mat4 uView;
    vec3 uViewPos;
    float pad5;  
    Light lights[50];
    uint lightCount;
    vec3 pad6; 
};

void main()
{
    vec3 ambient = vec3(0.1, 0.1, 0.1);
    vec3 diffuse = vec3(0.1, 0.1, 0.1);
    vec3 specular = vec3(0.1, 0.1, 0.1);
    
    
    vec3 V = normalize(uViewPos - FragPos);

    for(int i = 0; i < lightCount; ++i)
    {
        vec3 L = normalize(lights[i].position - FragPos);
        vec3 R = reflect(-L, Normal);

        ambient += lights[i].ambient * material.ambient;
        diffuse += max(dot(L, Normal), 0.0) * lights[i].diffuse * material.diffuse;
        
        float spec = max(dot(R, V), 0.0);
        specular += pow(spec, material.shininess) * lights[i].specular * material.specular;
    }

    vec4 color = vec4((ambient + diffuse + specular),1.f) * texture(uTexture2D, TexCoords);
    
    FragColor = min(1.5 * color, vec4(1.f, 1.f, 1.f, 1.f));
}