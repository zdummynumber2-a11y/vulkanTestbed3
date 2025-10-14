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
float bright = float(0.625);
float medium = float(0.5);
float dark = float(0.25);
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
            returnColor = bright;
        }
        else if(direction == 1) {
            returnColor = medium;
        }
        else if(direction == 2) {
            returnColor = dark;
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

bool orientationSphere(float screenX, float screenY) {
    float centeredX = screenX * 10 + 8.0;
    float centeredY = screenY * 10 + 7.0 * pcBuffer.intraVoxelPos[3];
    float rotatedX = pcBuffer.screanTranslation[2][3] * centeredY - pcBuffer.screanTranslation[1][3] * centeredX;
    float rotatedY = pcBuffer.screanTranslation[1][3] * centeredY + pcBuffer.screanTranslation[2][3] * centeredX;
    if (centeredX * centeredX + centeredY * centeredY < 1.0) {
        bool sphereSide = rotatedY >= pcBuffer.screanTranslation[0][3] * sqrt(1.0 - rotatedX * rotatedX);
        if(sphereSide) {
            outColor = vec4(0.5, 0.5, 0.0, 1.0);
            return true;
        }
        else {
            outColor = vec4(0.0, 0.5, 0.5, 1.0);
            return true;
        }
        
    }
    else {
        return false;
    }
}

void main() {
    uvec3 testVector = {131072, 0, 0};
    if(testVector[0] != 131072) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    
    vec3 screenVector = {1, 2 * fragColor[0] - 1, 2 * fragColor[1] - 1};
    float screenX = screenVector[1];
    float screenY = screenVector[2] * pcBuffer.intraVoxelPos[3];
    //x's range is -1 to 1, y is proportianal to x in screenspace.
    if(screenX * screenX + screenY * screenY < 0.0002) {
        invertCollor = true;
    }
    if(orientationSphere(screenX, screenY)) {
        return;
    }
    
    //screanTranslation is a 3X4 matrix, it needs to be this way because of stupid padding rules because programers hate the number 3. i dont know if strait multiplying it here will break anything in the future.
    //i just asume when i convert to mat3 it just deleats the last collumn. i eventually plan to use the last 3 floats for the players intravoxel position.
    screenVector = mat3(pcBuffer.screanTranslation) * screenVector;
    float maxValue = max(max(abs(screenVector[0]),abs(screenVector[1])), abs(screenVector[2]));

    uint16_t xIndex;
    uint16_t yIndex;
    uint16_t zIndex;
    int16_t majorDirectionDirection;
    if(maxValue == abs(screenVector[0])) {
        xIndex = uint16_t(0);
        yIndex = uint16_t(1);
        zIndex = uint16_t(2);
        majorDirectionDirection = int16_t(sign(screenVector[0]));
        if (majorDirectionDirection == -1) {
            //in lue of a propper solution to lighting, i'm just gonna do this.
            bright = float(1.0 - 0.625);
            medium = float(0.5); //1.0 - 
            dark = float(0.25);
        }
        
    }
    else if(maxValue == abs(screenVector[1])) {
        xIndex = uint16_t(1);
        yIndex = uint16_t(0);
        zIndex = uint16_t(2);
        majorDirectionDirection = int16_t(sign(screenVector[1]));
        
    }
    else if(maxValue == abs(screenVector[2])) {
        xIndex = uint16_t(2);
        yIndex = uint16_t(0);
        zIndex = uint16_t(1);
        majorDirectionDirection = int16_t(sign(screenVector[2]));
        if (majorDirectionDirection == -1) {
            //in lue of a propper solution to lighting, i'm just gonna do this.
            bright = float(0.625);
            medium = float(1.0 - 0.5);
            dark = float(1.0 - 0.25);
        }
        
    }

    screenVector /= maxValue;
    
    
    

    //uint16_t xIndex = uint16_t(0);
    //uint16_t yIndex = uint16_t(1);
    //uint16_t zIndex = uint16_t(2);

    

    //the rays angles are stored as slopes, the upside of this is that slopes are really easy and cheap to work with, the downside is that I'm stuck facing north for now.
    uint16_t xSlope;
    uint16_t ySlope;
    //i also need to store the sign of the slope, probably could have done this with signed integers, but that could make bit manipulation more difficult in the future.
    int16_t xDirection;
    int16_t yDirection;
    if(screenVector[yIndex] >= 0.0) {
        xDirection = int16_t(1);
        xSlope = uint16_t((screenVector[yIndex]) * 32768);
    }
    else {
        xDirection = int16_t(-1);
        xSlope = uint16_t((-screenVector[yIndex]) * 32768);
    }
    if(screenVector[zIndex] >= 0.0) {
        yDirection = int16_t(1);
        ySlope = uint16_t((screenVector[zIndex]) * 32768);
    }
    else {
        yDirection = int16_t(-1);
        ySlope = uint16_t((-screenVector[zIndex]) * 32768);
    }
    

    
    
    
    
    uvec3 universalPosition = pcBuffer.playerPosition;
    //this number is not acounting for the players y and z coordanits
    uint16_t xPose;
    uint16_t yPose;
    if(majorDirectionDirection == 1) {
        xPose = uint16_t(((1.0 - pcBuffer.intraVoxelPos[xIndex]) * float(xSlope)));
        yPose = uint16_t(((1.0 - pcBuffer.intraVoxelPos[xIndex]) * float(ySlope)));
    }
    else {
        xPose = uint16_t(((pcBuffer.intraVoxelPos[xIndex]) * float(xSlope)));
        yPose = uint16_t(((pcBuffer.intraVoxelPos[xIndex]) * float(ySlope)));
    }
    


    if(xDirection == -1) {
        xPose += uint16_t(((1.0 - pcBuffer.intraVoxelPos[yIndex]) * 32768.0)); 
    }
    else {
        xPose += uint16_t(((pcBuffer.intraVoxelPos[yIndex]) * 32768.0));
    }
    if(yDirection == -1) {
        yPose += uint16_t(((1.0 - pcBuffer.intraVoxelPos[zIndex]) * 32768.0)); 
    }
    else {
        yPose += uint16_t(((pcBuffer.intraVoxelPos[zIndex]) * 32768.0));
    }

    if((pcBuffer.intraVoxelPos[zIndex] * 32768.0) + (pcBuffer.intraVoxelPos[xIndex] * float(ySlope)) >= 65536) {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }

    //every iteration of this loop will step the x coordinate of the ray by 1.
    float fogConstant = 1.0 - (1.0 / 64);

    for(int i = 0; i < 512; i++) {
        fog *= fogConstant;
        //adds slope to intra-voxel position, will later check if the new position leaves the current voxel, if it does it checks all the potential voxels it intersected.
        
        if (xSlope >= 32768 || ySlope >= 32768) {
            outColor = vec4(1.0, 0.0, 1.0, 1.0);
            return;
        }

        //i could explain this part, but i think it would be more funny if i didn't.
        if (xPose >= 32768 && yPose >= 32768) {
            xPose = xPose % uint16_t(32768);
            yPose = yPose % uint16_t(32768);
            //(uint32_t(yPose) * xSlope) > (uint32_t(xPose) * ySlope)
            //true
            if ((uint32_t(yPose) * xSlope) > (uint32_t(xPose) * ySlope)) {
                universalPosition[zIndex] = universalPosition[zIndex] + uint32_t(yDirection);
                if(isOccupied(universalPosition, zIndex, yDirection)) {
                    return;
                }

                universalPosition[yIndex] = universalPosition[yIndex] + uint32_t(xDirection);
                if(isOccupied(universalPosition, yIndex, xDirection)) {
                    return;
                }
            }
            else {
                universalPosition[yIndex] = universalPosition[yIndex] + uint32_t(xDirection);
                if(isOccupied(universalPosition, yIndex, xDirection)) {
                    return;
                }

                universalPosition[zIndex] = universalPosition[zIndex] + uint32_t(yDirection);
                if(isOccupied(universalPosition, zIndex, yDirection)) {
                    return;
                }
            }

            

            
        }
        else if (xPose >= 32768) {
            xPose = xPose % uint16_t(32768);

            universalPosition[yIndex] = universalPosition[yIndex] + uint32_t(xDirection);
            if(isOccupied(universalPosition, yIndex, xDirection)) {
                return;
            }
        }
        else if (yPose >= 32768) {
            yPose = yPose % uint16_t(32768);
            universalPosition[zIndex] = universalPosition[zIndex] + uint32_t(yDirection);
            if(isOccupied(universalPosition, zIndex, yDirection)) {
                return;
            }
        }
        universalPosition[xIndex] += uint32_t(majorDirectionDirection);
        if(isOccupied(universalPosition, xIndex, int16_t(1))) {
            return;
        }
        xPose = xPose + xSlope;
        yPose = yPose + ySlope;

    }

    if(invertCollor) {
        outColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    else {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    
}