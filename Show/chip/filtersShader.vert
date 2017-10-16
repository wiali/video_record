#version 400

in vec3 vertTexCoord;
out vec2 fragTexCoord;
uniform vec2 homographyResolution;
uniform mat4 projectionMatrix;
uniform mat3 textureMatrix;

void main() {
    gl_Position = projectionMatrix * vec4(vertTexCoord, 1.0);
    fragTexCoord = vec2(vertTexCoord);
}
