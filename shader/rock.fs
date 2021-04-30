#version 430

in vec3 varPosition;

out float color;

vec2 pillars[3]= vec2[](
    vec2(0.4f, -0.4f),
    vec2(-0.4f, 0.1f),
    vec2(0.3f, 0.5f)
);

void main(void) {
    // Set rules for the generated point cloud (= the 3D Texture)
    // (where is solid rock and where is no rock)
    float f = 0.0f;
    vec2 newPos = varPosition.xz;

    // Rotate pillars while moving up/down
    mat2 rotMatrix;
    rotMatrix[0] = vec2(cos(varPosition.y), -sin(varPosition.y));
    rotMatrix[1] = vec2(sin(varPosition.y), cos(varPosition.y));
    newPos = rotMatrix * newPos;

    // Create pillars
    for(int i = 0; i < 3; i++){
        float t = 1 / (length(newPos - pillars[i]) * 1.5) - 1;
        t = max(t, 0.0);
        f += t;
    }

    // Add a "water flow" channel
    f -= 1 / length(newPos) - 1;

    // Add strong negative values at outer edge
    float len = length(newPos);
    f = f - pow(len, 10);

    // Add Shelves
    f += cos(varPosition.y * 5);

    color = f;
}

