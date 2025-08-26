#version 100
precision mediump float;

// Input varyings from vertex shader
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Uniforms
uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main() {
    // Sample the texture
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    
    // Apply color tint and alpha
    vec4 finalColor = texelColor * colDiffuse * fragColor;
    
    // Ensure alpha is properly set
    if (finalColor.a < 0.1) discard;
    
    // Output final color
    gl_FragColor = finalColor;
}
