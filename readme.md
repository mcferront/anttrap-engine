# Anttrap Engine

This was originally an DX9/OpenGL engine which ran on Win, Mac, Linux, and iOS, I built it with some important contributions from friends circa 2010 and it shipped some great indie projects.  Over the past year I've resurrected it and I've upgraded it to DX12 with both a forward and deferred rendering path.  It's not close to shipping quality; there are bugs, performance issues, etc.  However it's usable and improving every day.

My main purpose of putting it up on Github is to have branches of different rendering experiments I play around with as I continue to bug fix and improve the core architecture.

## Running the Binary
To run the compiled code simply navigate to the Game/AssetLibrary/_Resources directory and run Game.exe.  Hopefully it'll work, but no guarantees - I've only tested it on the couple of machines at my house.  

## Building the Code
To build the code, define the environment variable: ANTTRAP_GAME and have it point to the _Resources directory (this will fulfill the post build copy step requirements).  Then just open Game/Code/Win32/Game.sln (VS 2K17) and build.  

## Tools
In the future I plan on posting the code for the tools and instructions on how to build new scenes with the tools.

## todos
Finally, as I mentioned this is a big work in progress - you can see the todo list (which is my random spam of things which need to get done) at todo.md

# Practicle Edge Preserving Depth of Field
Pressing space bar will toggle through different camera shots
Camera can be translated via W,A,S,D and rotated with Left Click + Mouse

## Console Commands
* dof.enable               1 or 0 (Enables or Disables DOF)
* dof.near_kernel          Size of the near kernel, 1 to 5 are reasonable values with no artifacts
* dof.far_kernel           Size of the far kernel, 1 to 21 are reasonable values with no artifacts
* dof.focal_plane_start    Location at which the in focus plane starts (in units)
* dof.focal_plane_end      Location at which the in focus plane ends (in units)
* dof.near_pow             Blur fall off of the near field blur
* dof.coc_blend            A value greater than 0 will cause a blended transition from the focal plane to the far blur field

### Thx to Nvidia and Amazon for the Bistro
https://developer.nvidia.com/orca