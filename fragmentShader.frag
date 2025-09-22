#version 450
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragColor;

//all of this is completely bodged together.

//lol, i don't even know how to set up my own VkBuffer yet, so i just hardcoded whether a voxel is occupied with this 3d function.
bool isOccupied(ivec3 position) {
    /*int sum = position[0] + (position[1] << 1) + (position[2] << 2);
    return bool(((sum >> 0) & 1) * ((sum >> 1) & 1) * ((sum >> 2) & 1) * ((sum >> 3) & 1));*/

    return (cos((float(position[0]) + float(position[1])) * 0.03125 + 20.0) + cos((float(position[1]) + float(position[2])) * 0.03125 + 30.0) + cos((float(position[2]) + float(position[0])) * 0.03125 + 40.0) > 0.5);

    /*if (position == ivec3(8, 2, 3)) {
        return true;
    }
    else {
        return false;
    }*/
}

void main() {

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
    

    
    
    
    
    ivec3 universalPosition = ivec3(-2, -48, -31);
    //at any given moment a rays x coordinate is always an integer, these will store where the ray intersects the x = integer plane.
    uint16_t xPose = uint16_t(0);
    uint16_t yPose = uint16_t(0);
    float fog = 1;

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
                universalPosition[2] = universalPosition[2] + yDirection;
                if(isOccupied(universalPosition)) {
                    outColor = vec4(0.25 * fog, 0.25 * fog, 0.25 * fog, 1.0);
                    return;
                }

                universalPosition[1] = universalPosition[1] + xDirection;
                if(isOccupied(universalPosition)) {
                    outColor = vec4(0.5 * fog, 0.5 * fog, 0.5 * fog, 1.0);
                    return;
                }
            }
            else {
                universalPosition[1] = universalPosition[1] + xDirection;
                if(isOccupied(universalPosition)) {
                    outColor = vec4(0.5 * fog, 0.5 * fog, 0.5 * fog, 1.0);
                    return;
                }

                universalPosition[2] = universalPosition[2] + yDirection;
                if(isOccupied(universalPosition)) {
                    outColor = vec4(0.25 * fog, 0.25 * fog, 0.25 * fog, 1.0);
                    return;
                }
            }

            

            
        }
        else if (xPose >= 32768) {
            xPose = xPose % uint16_t(32768);

            universalPosition[1] = universalPosition[1] + xDirection;
            if(isOccupied(universalPosition)) {
                outColor = vec4(0.5 * fog, 0.5 * fog, 0.5 * fog, 1.0);
                return;
            }
        }
        else if (yPose >= 32768) {
            yPose = yPose % uint16_t(32768);
            universalPosition[2] = universalPosition[2] + yDirection;
            if(isOccupied(universalPosition)) {
                outColor = vec4(0.25 * fog, 0.25 * fog, 0.25 * fog, 1.0);
                return;
            }
        }
        universalPosition[0]++;
        if(isOccupied(universalPosition)) {
            outColor = vec4(0.75 * fog, 0.75 * fog, 0.75 * fog, 1.0);
            return;
        }

    }

    
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
}