#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
//layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec3 aTangent;
//layout (location = 3) in vec2 aUV;

out vec2 TexCoord;
out vec4 Frag;
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

void main()
{
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.f);
    gl_Position.z = -gl_Position.z;
 
    Frag = vec4(aColor,1.f);
    //TexCoord = aUV;
}


////////////////////////////////////////////////////////////////////////////////

#version 460 core

in vec2 TexCoord;
in vec4 Frag;
out vec4 FragColor;
uniform sampler2D uTexture2D;
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
   // vec4 textureColor = texture(uTexture2D,TexCoord);
   // FragColor = textureColor * vec4(1.f,1.f,1.f,1.f);
   FragColor = Frag * vec4(1.f,1.f,1.f,1.f);
}