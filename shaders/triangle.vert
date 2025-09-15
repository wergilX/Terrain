#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 Diffuse;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lowColor;
uniform vec3 middleColor;
uniform vec3 highColor;

void main()
{
    float height = aPos.y / 20.0;

    if (height < 0.3)
    {
        float t = height / 0.3;
        Diffuse = mix(lowColor, middleColor, t);
    }
    else if (height < 0.7)
    {
        Diffuse = middleColor;
    }
    else
    {
        float t = (height - 0.7) / 0.3;
        Diffuse = mix(middleColor, highColor, t);
    }

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;
    //Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}