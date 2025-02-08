#version 330 core

layout(location = 0) in vec3 in_position; // Vertex position

uniform mat4 Model;      // Model matrix
uniform mat4 View;       // View matrix
uniform mat4 Projection; // Projection matrix
uniform vec3 objectColor; // Color of the object

out vec3 fragColor; // Pass color to Fragment Shader

void main()
{
    fragColor = objectColor; // Set the color of the object
    // Compute the final position
    gl_Position = Projection * View * Model * vec4(in_position, 1.0);
}
