# vulkanTestbed3



anyway, that's the name. this project is supposed to be the beginnings of a full game engine. currently I'm only working on the very basics such as movement and looking around, though when i get those done i plan on making a sort of flight simulator inspired by the Minecraft elytra, only with manual roll control. after that the plans are a bit hazy but here is the general outline.



1. figure out how to set up a vkBuffer to store level data
2. switch from using the fragment shader as the main renderer to a proper compute shader.
3. figure out transparency(similar to the current fog, only it works within voxels)
4. use a sparce octree both for level compression and for levels of detail(may involve making a custom file format)
5. optimize.
6. make tools for making voxel files(specifically one that converts normal 3d models to a voxel format)
7. whatever else. this point in the future is so far away that plans start to get fuzzy, i could implement portals and non-standard spatial geometry(which is the main reason i decided to go with ray casting instead of rasterization), experiment with procedural generation, implement lighting, or probably fix some fundamental problem i found along the way.



i know this might all be a bit ambitious for a relatively new programmer, but I've had fun so far(except for setting up Vulkan), so I'm exited for the future. i have so many ideas in my head, i have for the longest time, and i know they'll never see reality if i don't implement them myself.



also for anyone reading this, here is a highlight real of all the [visually interesting bugs](https://drive.google.com/drive/folders/1EtuWzIdOcuY1snB4_NvYhLG9boP5WWRR?usp=drive_link) I've come across throughout the development of this.



