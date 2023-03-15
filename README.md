"# ComputeShaderExample" 
An attempt to add a Uniform variable to the code from this video:
https://www.youtube.com/watch?v=nF4X9BIUzx0

Currently the Uniform updates only ONCE, and I don;t know why. No errors are thrown by OpenGL/GLSL.
Tracing the frames with Renderdoc shows everything working fine, 
except the timer Uniform is only getting updated the first time it is set. 
The variable can be seen in debug mode to be changing over time on the C++ side,
but subsequent calls to glUniform1f() do not update the value the compute shader uses.

So far, no one on r/opengl or Victor Gordon's Discord can fathom why.
