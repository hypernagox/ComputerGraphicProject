#version 460 core

layout(location = 0) in vec3 inPosition;

uniform mat4 uModelViewProjectionMatrix;
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
    gl_Position = uModelViewProjectionMatrix * vec4(inPosition, 1.f);
    gl_Position.z = -gl_Position.z;
}


////////////////////////////////////////////////////////////////////////////////

#version 460 core

out vec4 outColor;
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
    outColor = vec4(1.0, 1.0, 1.0, 1.0);  
}