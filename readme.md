# Anttrap Engine
http://www.trapzz.com

This was originally an DX9/OpenGL engine which ran on Win, Mac, Linux, and iOS, I built it with some important contributions from friends circa 2010 and it shipped some great indie projects.  Over the past year I've resurrected it and I've upgraded it to DX12 with both a forward and deferred rendering path.  It's not close to shipping quality; there are bugs, performance issues, etc.  However it's usable and improving every day.

My main purpose of putting it up on Github is to have branches of different rendering experiments I play around with as I continue to bug fix and improve the core architecture.

## Running the Binary
To run the compiled code simply navigate to the Game/AssetLibrary/_Resources directory and run Game.exe.  Hopefully it'll work, but no guarantees - I've only tested it on the couple of machines at my house.  

## Building the Code
To build the code, define the environment variable: ANTTRAP_GAME and have it point to the _Resources directory (this will fulfill the post build copy step requirements).  Then just open Game/Code/Win32/Game.sln (VS 2K17) and build.  

## Tools
In the future I plan on posting the code for the tools and instructions on how to build new scenes with the tools.

## Branches
Practical Edge Preserving DOF: https://github.com/mcferront/anttrap-engine/tree/pep_dof

## Todos
Finally, as I mentioned this is a big work in progress - you can see the todo list (which is my random spam of things which need to get done) at todo.md

Here's some examples of it:
![alt text](http://www.trapzz.com/wp-content/uploads/2018/09/bokeh_20.png) 
![alt text](http://www.trapzz.com/wp-content/uploads/2018/09/plant_3.png)
![alt text](http://www.trapzz.com/wp-content/uploads/2019/01/mw.png)
![alt text](http://www.trapzz.com/wp-content/uploads/2019/01/se.png)
