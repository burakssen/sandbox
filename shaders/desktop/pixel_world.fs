#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// Simple color adjustments
uniform float brightness = 1.0;

void main() {
    // Sample the texture with nearest neighbor filtering
    vec4 texelColor = texture(texture0, fragTexCoord) * colDiffuse;
    
    // Apply brightness
    vec3 color = texelColor.rgb * brightness;
    
    // Final color with original alpha
    finalColor = vec4(color, texelColor.a);
}
