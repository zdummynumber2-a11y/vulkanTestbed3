#version 450
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragColor;
layout(push_constant) uniform PushConstants {
    mat3 screanTranslation;
    ivec3 playerPosition;
} pcBuffer;
float fog = 1;

//all of this is completely bodged together.

//lol, i don't even know how to set up my own VkBuffer yet, so i just hardcoded whether a voxel is occupied with this 3d function.
bool isOccupied(ivec3 position, uint16_t direction, int16_t directionSighn) {
    float returnColor;
    if(cos((float(position[0]) + float(position[1])) * 0.0625 + 20.0) + cos((float(position[1]) + float(position[2])) * 0.0625 + 30.0) + cos((float(position[2]) + float(position[0])) * 0.0625 + 40.0) > 0.5) {
        if(direction == 0) {
            returnColor = float(0.625);
        }
        else if(direction == 1) {
            returnColor = float(0.5);
        }
        else if(direction == 2) {
            returnColor = float(0.25);
        }
        else {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
        return true;
        }

        if(directionSighn == -1) {
            returnColor = 1.0 - returnColor;
        }

        outColor = vec4(returnColor * fog, returnColor * fog, returnColor * fog, 1.0);
        return true;
    }
    return false;
}

void main() {
    
    vec3 screenVector = {1, 2 * fragColor[0] - 1, 2 * fragColor[1] - 1}
    screenVector = screanTranslation * screenVector;
    adjustment = max(max(screenVector[0],screenVector[1]), screenVector[2]); // cry about it
    screenVector /= adjustment;


    //already: looking forward, +y +z, going +x
    //test one: looking down, +y, +x, going -z
    //test two: looking left, +x, +z, going -y
    //test three: looking behind, -y, -z, going -x
    //test one: looking up, -y, -x, going +z
    //test two: looking left, -x, -z, going +y

    uint16_t xIndex = uint16_t(0);
    uint16_t yIndex = uint16_t(1);
    uint16_t zIndex = uint16_t(2);

    

    //the rays angles are stored as slopes, the upside of this is that slopes are really easy and cheap to work with, the downside is that I'm stuck facing north for now.
    uint16_t xSlope;
    uint16_t ySlope;
    //i also need to store the sign of the slope, probably could have done this with signed integers, but that could make bit manipulation more difficult in the future.
    int16_t xDirection;
    int16_t yDirection;
    if(fragColor[0] >= 0.5) {
        xDirection = int16_t(1);
        xSlope = uint16_t((fragColor[0] - 0.5) * 65536);
    }
    else {
        xDirection = int16_t(-1);
        xSlope = uint16_t((0.5 - fragColor[0]) * 65536);
    }
    if(fragColor[1] >= 0.5) {
        yDirection = int16_t(1);
        ySlope = uint16_t((fragColor[1] - 0.5) * 65536);
    }
    else {
        yDirection = int16_t(-1);
        ySlope = uint16_t((0.5 - fragColor[1]) * 65536);
    }
    

    
    
    
    
    ivec3 universalPosition = ivec3(80, -15, -6);
    //at any given moment a rays x coordinate is always an integer, these will store where the ray intersects the x = integer plane.
    uint16_t xPose = uint16_t(0);
    uint16_t yPose = uint16_t(0);
    

    //every iteration of this loop will step the x coordinate of the ray by 1.
    for(int i = 0; i < 384; i++) {
        fog *= 0.984375;
        //adds slope to intra-voxel position, will later check if the new position leaves the current voxel, if it does it checks all the potential voxels it intersected.
        xPose = xPose + xSlope;
        yPose = yPose + ySlope;

        //i could explain this part, but i think it would be more funny if i didn't.
        if (xPose >= 32768 && yPose >= 32768) {
            xPose = xPose % uint16_t(32768);
            yPose = yPose % uint16_t(32768);
            //(uint32_t(yPose) * xSlope) > (uint32_t(xPose) * ySlope)
            //true
            if ((uint32_t(yPose) * xSlope) > (uint32_t(xPose) * ySlope)) {
                universalPosition[zIndex] = universalPosition[zIndex] + yDirection;
                if(isOccupied(universalPosition, zIndex, yDirection)) {
                    return;
                }

                universalPosition[yIndex] = universalPosition[yIndex] + xDirection;
                if(isOccupied(universalPosition, yIndex, xDirection)) {
                    return;
                }
            }
            else {
                universalPosition[yIndex] = universalPosition[yIndex] + xDirection;
                if(isOccupied(universalPosition, yIndex, xDirection)) {
                    return;
                }

                universalPosition[zIndex] = universalPosition[zIndex] + yDirection;
                if(isOccupied(universalPosition, zIndex, yDirection)) {
                    return;
                }
            }

            

            
        }
        else if (xPose >= 32768) {
            xPose = xPose % uint16_t(32768);

            universalPosition[yIndex] = universalPosition[yIndex] + xDirection;
            if(isOccupied(universalPosition, yIndex, xDirection)) {
                return;
            }
        }
        else if (yPose >= 32768) {
            yPose = yPose % uint16_t(32768);
            universalPosition[zIndex] = universalPosition[zIndex] + yDirection;
            if(isOccupied(universalPosition, zIndex, yDirection)) {
                return;
            }
        }
        universalPosition[xIndex]++;
        if(isOccupied(universalPosition, xIndex, int16_t(1))) {
            return;
        }

    }

    
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
    

}