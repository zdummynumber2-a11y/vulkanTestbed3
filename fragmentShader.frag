#version 450
#extension GL_EXT_shader_explicit_arithmetic_types : require
#define PI 3.141593
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragColor;
layout(push_constant) uniform PushConstants {
    mat3x4 screanTranslation;
    vec4 intraVoxelPos;
    uvec3 playerPosition;
} pcBuffer;
float fog = 1;

//all of this is completely bodged together.

//lol, i don't even know how to set up my own VkBuffer yet, so i just hardcoded whether a voxel is occupied with this 3d function.
//this function was originally made for looking in +x exclucivly, these varyables allow to reset each faces shading based on what direction the ray is, these are the defaults.

float ocilator (float x) {
    return float(((6.0 * x - 9.0) * x + 3.0) * x);
}

bool invertCollor = false;
bool isOccupied(uvec3 position, uint16_t direction, int16_t directionSighn) {
    float returnColor;
    float terainScale = 1.0 / 128.0;
    float downscaleFactor = 0.618034;
    
    //very basic noise function, probobly could have used perline noise but i'm using this instead
    //ocilator(pos % terainScale)
    bool voxelOcupied =
    ocilator(mod(float(position[0] + position[1]) * terainScale, 1.0)) + 
    ocilator(mod(float(position[1] + position[2]) * terainScale, 1.0)) + 
    ocilator(mod(float(position[2] + position[0]) * terainScale, 1.0)) +
    ocilator(mod(float(position[0]) * terainScale * downscaleFactor, 1.0)) + 
    ocilator(mod(float(position[1]) * terainScale * downscaleFactor, 1.0)) + 
    ocilator(mod(float(position[2]) * terainScale * downscaleFactor, 1.0)) > 0.0;
    if(voxelOcupied) {
        if(direction == 0) {
            returnColor = 0.375;
        }
        else if(direction == 1) {
            returnColor = 0.25;
        }
        else if(direction == 2) {
            returnColor = 0.5;
        }
        else {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
        return true;
        }

        if((directionSighn == -1)) {
            returnColor = (1.0 - returnColor) * fog;
        }
        else {
            returnColor = returnColor * fog;
        }
        if (invertCollor) {
        returnColor = 1.0 - returnColor;
        }

        outColor = vec4(returnColor, returnColor, returnColor, 1.0);
        return true;
    }
    return false;
}


float minimum(vec3 vector) {
    return min(min(vector[0], vector[1]), vector[2]);
}

int minIndex(vec3 vector) {
    return int((vector[1] < vector[2]) && (vector[1] < vector[0])) + (int((vector[2] < vector[1]) && (vector[2] < vector[0])) * 2);
}
void main() {
    vec3 directionVector = {2.0 * fragColor[0] - 1.0, 2.0 * fragColor[1] - 1.0, 1.0}; // usumes a 32 bit float
    float screenX = directionVector[0];
    float screenY = directionVector[1] * pcBuffer.intraVoxelPos[3];
    //x's range is -1 to 1, y is proportianal to x in screenspace.
    if(screenX * screenX + screenY * screenY < 0.0002) {
        invertCollor = true;
    }
    
    //screanTranslation is a 3X4 matrix, it needs to be this way because of stupid padding rules because programers hate the number 3.
    //i just asume when i convert to mat3 it just deleats the last collumn.
    directionVector *= mat3(pcBuffer.screanTranslation);
    float adjustment = 1.0 / (abs(directionVector[0]) + abs(directionVector[1]) + abs(directionVector[2]));
    directionVector *= adjustment;
    vec3 rayPos = vec3(pcBuffer.intraVoxelPos); // distence to corisponding voxel wall
    if (directionVector[0] > 0) {
        rayPos[0] = 1.0 - rayPos[0];
    }
    if (directionVector[1] > 0) {
        rayPos[1] = 1.0 - rayPos[1];
    }
    if (directionVector[2] > 0) {
        rayPos[2] = 1.0 - rayPos[2];
    }

    vec3 distTilStep = 1.0 / abs(directionVector);
    uvec3 univercalPos = pcBuffer.playerPosition;

    //if (directionVector[0] < 0.0 || directionVector[1] < 0.0 || directionVector[2] < 0.0) {
    //    outColor = vec4(1.0, 0.0, 1.0, 1.0);
    //    return;
    //}
    float stepDist;
    uint16_t stepIndex;
    float fogConstant =  507.0 / 512.0;//507.0 / 512.0
    for (int i = 0; i < 512; i++) {
        fog *= fogConstant;
        float stepDist = minimum(rayPos * distTilStep);
        rayPos -= abs(directionVector) * stepDist;
        uint16_t stepIndex = uint16_t(minIndex(rayPos));
        univercalPos[stepIndex] += int32_t(sign(directionVector[stepIndex]));
        rayPos[stepIndex]++;
        if (isOccupied(univercalPos, stepIndex, int16_t(sign(directionVector[stepIndex])))) {
            return;
        }
    }

    
    if (invertCollor) {
        outColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    else {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    return;
    
    
}