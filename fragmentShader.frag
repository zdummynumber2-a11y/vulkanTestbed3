#version 450
#extension GL_EXT_shader_8bit_storage: require

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragColor;

//takes inputs in the range (0, 0) -> (1, 1)
float function(float x, float y) {
    float result = mod(cos(tan(4 * (x - 0.5))) + sin(tan(4 * y)), 1);
    return result;
}

void main() {
    //overkill multisampling(256 samples)
    int resolution = 65536; // idk why this number works the best, is should be 16384(resolution * #of rows), this one just works the best.
    float runingTotal = 0;
    for(int i = 0; i < 256; i++) {
        runingTotal += function(fragColor[0] + (float(i & 0xf0) / resolution), fragColor[1] + (float(i & 0x0f) / resolution)) / 256;
    }

    float value = mod(runingTotal, 1);
    outColor = vec4(value, value, value, 1.0);
}