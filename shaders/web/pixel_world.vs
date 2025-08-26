#version 100
precision mediump float;

// Input attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;

// Uniforms
uniform mat4 mvp;

// Output varyings to fragment shader
varying vec2 fragTexCoord;
varying vec4 fragColor;

void main() {
    // Pass attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Compute final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
