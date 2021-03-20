#version 430

in vec3 varPosition;

out float color;

void main(void) {
    // Set rules for the generated point cloud (= the 3D Texture)
    // (where is solid rock and where is no rock)
    float sinX = sin(varPosition.x * 5.8905);
    float cosY = cos(varPosition.y * 5.8905);
    float cosZ = cos(varPosition.z * 5.8905);
    color = (sinX * sinX + cosY * cosY + cosZ * cosZ) * (1.0f / 3.0f);
}

