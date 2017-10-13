#version 330 core

in vec3 attribPosition;
in vec4 attribColor;
in vec2 attribTexcoord;
in vec3 attribNormal;
uniform mat4 uMVP;

varying highp vec4 color;
varying highp vec2 textureCoord;
varying highp vec3 normal;

void main(void)
{
    color = attribColor;
    textureCoord = attribTexcoord;
    normal = attribNormal;
    gl_Position = uMVP * vec4(attribPosition, 1.0);
}
