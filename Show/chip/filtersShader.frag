#version 410

layout(location = 0) out vec4 fragColor;
uniform sampler2D tex;
in vec2 fragTexCoord;
uniform vec2 imageSize;
uniform mat3 textureMatrix;

void main() {
    // Calculate texture position based on homography resolution in pixels
    vec2 texturePixelCoord = vec2(fragTexCoord.x * imageSize.x, fragTexCoord.y * imageSize.y);

    float determinant = textureMatrix[2][0] * texturePixelCoord.x + textureMatrix[2][1] * texturePixelCoord.y + textureMatrix[2][2];
    float texureX = (textureMatrix[0][0] * texturePixelCoord.x + textureMatrix[0][1] * texturePixelCoord.y + textureMatrix[0][2]) / determinant;
    float texureY = (textureMatrix[1][0] * texturePixelCoord.x + textureMatrix[1][1] * texturePixelCoord.y + textureMatrix[1][2]) / determinant;

    vec2 texCoord = vec2(texureX / imageSize.x, texureY / imageSize.y);

    fragColor = texture(tex, texCoord);
}
