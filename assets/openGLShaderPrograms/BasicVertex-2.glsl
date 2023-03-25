#version 150 core

uniform float distance;
uniform vec2 resolution;
out vec4 out_color;

void main()
{
    // Center our coordinate system
    // vec2 uv = (gl_FragCoord.xy - 0.5*resolution.xy) / min(resolution.x, resolution.y);
    vec2 uv = gl_FragCoord.xy;

    // Vary pixel color with y-coordinate
    vec3 col = vec3(uv.x, uv.y, fract(uv.x*uv.y));
    // col = vec3(1.0,0.1,0.1);
    // Output to screen
    out_color = vec4(col,1.0);
}