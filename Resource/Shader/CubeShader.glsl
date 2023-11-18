#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aUV;

out vec3 FragPos;
out vec3 Normal;
out vec3 Tangent;

out vec3 TexCoords;

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
uniform mat4 uModel;
uniform mat4 uObserverView;

void main()
{
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.f);
    //FragPos = vec3(uModel * vec4(aPos, 1.f));
    
    FragPos = vec3(gl_Position);
    TexCoords = aPos;
    Normal = normalize(mat3(transpose(inverse(uObserverView * uModel))) * aNormal);
}

/////////////////////////////////////////////////////////////////////////////


#version 460 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

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
in vec3 Tangent;
in vec3 BiTangent;

in vec3 TexCoords;

out vec4 FragColor;


void main()
{
    vec3 ambient = vec3(.3, .3, .3);
    vec3 diffuse = vec3(.0, .0, .0);
    vec3 specular = vec3(.0, .0, .0);
    vec3 V = normalize(uViewPos - FragPos);

    for(int i = 0; i < lightCount; ++i)
    {
        vec3 L = normalize(lights[i].position - FragPos);
        vec3 R = reflect(-L, Normal);

        float diff = max(dot(L, Normal), 0.0); 
        if (diff > 0.0) 
        {
            diffuse += diff * lights[i].diffuse;

            float spec = max(dot(R, V), 0.0);
            specular += pow(spec, material.shininess) * lights[i].specular * material.specular;
        }
    }

    vec4 color = vec4((ambient + diffuse + specular), 1.f);
    FragColor = min(1.5 * color * texture(uCubeTexture, TexCoords), vec4(1.f, 1.f, 1.f, 1.f));
}