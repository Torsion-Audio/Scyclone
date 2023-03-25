#version 330 core

uniform vec2 iResolution;
uniform vec3 iBackgroundColor;
uniform float iDisplayScaleFactor;
uniform vec2 iKnobPos1;
uniform vec2 iKnobPos2;
uniform float iTime;
uniform float iAudioLevel;
uniform float iModelMix;
out vec4 fragColor;


// Noise - MIT License Kasper Arnklit Frandsen 2022
float rand(vec2 x) {
    return fract(cos(mod(dot(x, vec2(13.9898, 8.141)), 3.14)) * 43758.5453);
}

float value_noise(vec2 coord, vec2 size, float offset) {
    vec2 o = floor(coord)+size;
    vec2 f = fract(coord);
    float p00 = rand(mod(o, size));
    float p01 = rand(mod(o + vec2(0.0, 1.0), size));
    float p10 = rand(mod(o + vec2(1.0, 0.0), size));
    float p11 = rand(mod(o + vec2(1.0, 1.0), size));
    p00 = sin(p00 * 6.28 + offset) / 2.0 + 0.5; // This is where the animation of the points happen
    p01 = sin(p01 * 6.28 + offset) / 2.0 + 0.5; // This is where the animation of the points happen
    p10 = sin(p10 * 6.28 + offset) / 2.0 + 0.5; // This is where the animation of the points happen
    p11 = sin(p11 * 6.28 + offset) / 2.0 + 0.5; // This is where the animation of the points happen
    vec2 t =  f * f * f * (f * (f * 6.0 - 15.0) + 10.0); // Improved smoothstep for smoother gradients, mostly matters for normal maps
    return mix(mix(p00, p10, t.x), mix(p01, p11, t.x), t.y);
}

float fbm(vec2 coord, vec2 size, int folds, int octaves, float persistence, float offset) {
    float normalize_factor = 0.0;
    float value = 0.0;
    float scale = 1.0;
    for (int i = 0; i < octaves; i++) {
        float noise = value_noise(coord*size, size, offset);
        for (int f = 0; f < folds; ++f) {
            noise = abs(2.0*noise-1.0);
        }
        value += noise * scale;
        normalize_factor += scale;
        size *= 2.0;
        scale *= persistence;
    }
    return value / normalize_factor;
}
// END Noise - MIT License Kasper Arnklit Frandsen 2022

float mainNoise(vec2 fragCoords, float time)
{
    vec2 p = fragCoords.xy / iResolution.xy;

    vec2 uv = p*vec2(iResolution.x/iResolution.y,1.0);

    float noise = 0.0;

    noise = fbm(uv, vec2(3), 0, 4, 0.3, time);
    return noise;
}

// White Noise - Source: https://www.shadertoy.com/view/tlcBRl
float noise1(float seed1,float seed2){
    return(
    fract(seed1+12.34567*
    fract(100.*(abs(seed1*0.91)+seed2+94.68)*
    fract((abs(seed2*0.41)+45.46)*
    fract((abs(seed2)+757.21)*
    fract(seed1*0.0171))))))
    * 1.0038 - 0.00185;
}


// Circle
float circle(vec2 uv, float radius)
{
    //calculate distance from the origin point
    float d = length(uv);

    //     float value =  smoothstep(radius, 0.0 , d); // normal
    float value =  smoothstep(radius, 0.0,pow(d,2.0)); // graphical nicery

    return value;
}

float circle_simple(vec2 uv, vec2 pos, float rad) {
    float d = length(pos - uv) - rad;
    float t = clamp(d, 0.0, 1.0);
    return float(1.0 - t);
}

float circle_w_edge(vec2 uv, float radius, float edge_width)
{
    float value = circle(uv, radius);
    //    value = pow(value, value); // this line bought the Bug because if value = 0 its undefined -> different handling between Mac and shadertoy;
    if (value != 0.f) value = pow(value, value*edge_width) - value;
    else value = 0.f;
    return value;
}

mat2 rotationMatrix(float angle)
{
    float pi = 3.14;
    angle *= pi / 180.0;
    float s=sin(angle), c=cos(angle);
    return mat2( c, -s, s, c );
}

vec3 blob(vec2 fragCoord, float modelMix, vec2 uv, float time, float audioLevel, int switchRB)
{
    // Distort UV
    float radius = ((modelMix)*0.2f);
    // float radius = 0.2 + ((iModelMix+1.0)*0.2f);
    float edge_width = 0.7;
    float dist_map = circle_w_edge(uv, radius*2.0, edge_width);
    dist_map = dist_map * 0.017 + 0.11;

    // Distorted UV
    vec2 uv_distorted = uv + vec2((mainNoise(fragCoord, time)) * dist_map);


    // Create Circle
    float circle = circle(uv_distorted, radius);
    circle = pow(circle, 1.2);
    edge_width = 1.8;
    float circle_w_edge = circle_w_edge(uv_distorted, radius, edge_width);


    vec3 col_circle = vec3(pow(circle, 2.1)*0.1, circle*0.42, circle);
    col_circle = col_circle*0.8 + col_circle*(vec3(audioLevel, audioLevel, audioLevel));
    vec3 col_circle_edge = vec3(0.321*circle_w_edge, 0.17*circle_w_edge, circle_w_edge*0.9);

    vec3 color = col_circle + col_circle_edge;

    //rotate
    uv_distorted *= rotationMatrix(time*3.0);

    // pulse Color
    if (switchRB == 0)
    {
        color.b = color.b - (mainNoise(fragCoord*0.3, time)*0.17);
        color.r = color.r + ((circle_w_edge+(circle*0.8))*fbm(uv_distorted, vec2(1), 0, 2, 0.421, time)*(sin(0.1*time)+0.4)*0.641*(uv_distorted.x+sin(time+0.6))*(uv_distorted.y+sin(time+0.17)));
        color.r = color.r - (mainNoise(fragCoord*0.23, time)*0.22);
        color.g = color.g + ((circle_w_edge+(circle*0.8))*fbm(uv_distorted, vec2(2), 0, 2, 0.421, 0.17*time)*(sin(0.1*time)+0.4)*0.141*(uv_distorted.x+sin(time+0.3))*(uv_distorted.y+sin(time+0.17)));
    }
    if (switchRB == 1)
    {
        color = vec3(color.z*0.6, color.y*.1, color.x);
        col_circle_edge = vec3(col_circle_edge.x * 0.1, col_circle_edge.y*.3, col_circle_edge.x);
        color.g = color.b - (mainNoise(fragCoord*0.3, time)*0.17);
        color.r = color.r +((circle_w_edge+(circle*0.8))*fbm(uv_distorted, vec2(1), 0, 2, 0.421, time)*(sin(0.1*time)+0.4)*0.641*(uv_distorted.x+sin(time+0.6))*(uv_distorted.y+sin(time+0.17)));
        color.r = color.r - (mainNoise(fragCoord*0.23, time)*0.22);
        color.g = color.g - ((circle_w_edge+(circle*0.8))*fbm(uv_distorted, vec2(2), 0, 2, 0.421, 0.17*time)*(sin(0.1*time)+0.4)*0.141*(uv_distorted.x+sin(time+0.3))*(uv_distorted.y+sin(time+0.17)));
    }
    return color;
}

vec2 calcKnobPosUV(vec2 knobPos, vec2 resolution, vec2 uv, float aspect)
{
    vec2 mouse = vec2(knobPos.x, knobPos.y); // flip axis
    vec2 mUv = mouse * 2.; // adjust scale
    mUv = vec2(mUv.x - 1., mUv.y - 1.); // offset
    vec2 pos = vec2(mUv.x*0.7716049383,mUv.y*0.7142857143);
    uv = uv-pos;
    return uv;
}



void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    float time = iTime;
    float modelMix = iModelMix;
    vec2 knobPos1 = iKnobPos1;
    vec2 knobPos2 = iKnobPos2;
    float audioLevel = 1.0;

    float displayScaleFactor = iDisplayScaleFactor;
    vec2 resolution = iResolution.xy * displayScaleFactor;

    // Calc Model Values
    float model1 = 1.;
    float model2 = 1.;
    if (modelMix < 0.5)
    {
        model1 = modelMix * 2.;
    }
    if (modelMix > 0.5)
    {
        model2 = (1. - modelMix) * 2;
    }

    //center origin point
    vec2 uv = fragCoord/resolution.xy * 2.0 - 1.0;

    // Adjust to Aspect Ratio
    float aspect = resolution.x / resolution.y;
    uv.x *= aspect;

    // Offset UV with KnobPos
    vec2 uv_1 = calcKnobPosUV(knobPos1, resolution, uv, aspect);
    vec2 uv_2 = calcKnobPosUV(knobPos2, resolution, uv, aspect);

    // Generate Blobs
    vec3 blob_1 = blob(fragCoord, model1, uv_1, time, audioLevel, 0);
    vec3 blob_2 = blob(fragCoord, model2, uv_2, time, audioLevel, 1);

    blob_1 = vec3(blob_1.x * 0.3, blob_1.yz);
    blob_2 = vec3(blob_2.xy, blob_2.z);

    vec3 color = blob_1 + blob_2;

    // adding Background
    vec3 bg_color = iBackgroundColor;
    color = max(color, 0.0);
    color = bg_color + color;

    fragColor = vec4(color, 1.0);
}

