#version 330 core

uniform vec2 iResolution;
uniform vec3 iBackgroundColor;
uniform float iDisplayScaleFactor;
uniform vec2 iKnobPos1;
uniform vec2 iKnobPos2;
uniform float iTime;
uniform float iModelMix;
uniform float iAudioLevel1;
uniform float iAudioLevel2;
out vec4 fragColor;

vec2 calcKnobPosUV(vec2 knobPos, vec2 uv, float aspect) {
    vec2 pos = knobPos * 2. - 1.; // adjust scale and offset
    pos = vec2(pos.x * aspect, pos.y); // aspect needed
    uv = uv-pos;
    return uv;
}

float[2] calculateRadius(float modelMix) {
    float model_1 = 1.;
    float model_2 = 1.;
    if (modelMix < 0.5) model_1 = modelMix * 2.;
    else if (modelMix > 0.5) model_2 = 2.f * (1.f - modelMix);
    float[2] radius;
    radius[0] = ((model_1)*0.2f);
    radius[1] = ((model_2)*0.2f);
    return radius;
}

float circle(vec2 uv, float radius) {
    // calculate distance from the origin point
    float r = length(uv);
    float circle =  smoothstep(radius, 0.f, pow(r,2.f)); // graphical nicery

    return circle;
}

float circleEdge(vec2 uv, float circle, float radius) {
    float r = length(uv);
    float attenuator = 2.5f;
    if (circle != 0.f) circle = pow(circle, circle * attenuator) - circle;
    else circle = smoothstep(radius*1.02f , 1.f * radius, pow(r,2.f)); // antialiasing
    float modelMix = radius/0.2f;
    return circle * pow(modelMix, 1.f/modelMix);
}

// Noise - MIT License Kasper Arnklit Frandsen 2022 (modified)

// The random values generating function
float randomFunction(vec2 x) {
    return fract(cos(mod(dot(x, vec2(13.9898, 8.141)), 3.14)) * 43758.5453);
}

float noiseValue(vec2 coord, float offset) {
    vec2 size = vec2(3.f);
    vec2 i = floor(coord*size)+size;
    vec2 f = fract(coord*size);
    float p00 = randomFunction(mod(i, size));
    float p01 = randomFunction(mod(i + vec2(0.0, 1.0), size));
    float p10 = randomFunction(mod(i + vec2(1.0, 0.0), size));
    float p11 = randomFunction(mod(i + vec2(1.0, 1.0), size));
    p00 = sin(p00 * 6.28 + offset) / 2.0 + 0.5;
    p01 = sin(p01 * 6.28 + offset) / 2.0 + 0.5;
    p10 = sin(p10 * 6.28 + offset) / 2.0 + 0.5;
    p11 = sin(p11 * 6.28 + offset) / 2.0 + 0.5;
    // Improved smoothstep for smoother gradients, mostly matters for normal maps
    vec2 t =  f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    return mix(mix(p00, p10, t.x), mix(p01, p11, t.x), t.y);
}

// For Coord
vec2 noiseCoord(vec2 coord, float time) {
    return coord * noiseValue(coord, iTime);
}

// For single values
float noiseFloat(vec2 coord, float time) {
    return noiseValue(coord, time);
}

// END Noise - MIT License Kasper Arnklit Frandsen 2022 (modified)

vec3 colorCircle(float circle, float circleEdge, float audioLevel, vec2 coordDistorted, float time, int index) {
    vec3 colorCircle;
    vec3 colorCircleEdge;
    if (index == 0) {
        // initial colorset
        colorCircle = vec3(circle*0.1f, circle*0.3f, circle);
        colorCircle = colorCircle*0.8 + colorCircle*(vec3(audioLevel));
        colorCircleEdge = vec3(0.321*circleEdge, 0.17*circleEdge, circleEdge*0.9);
    
        // pulse the color
        colorCircle.b = colorCircle.b - noiseFloat(coordDistorted*0.3f, time*0.2f)*0.3f;
        colorCircleEdge.r = colorCircleEdge.r + colorCircleEdge.r * noiseFloat(coordDistorted*0.6f, time)*1.5f;
        colorCircle.r = colorCircle.r + colorCircle.r * (noiseFloat(coordDistorted*0.23, time)*0.22);
        colorCircle.g = colorCircle.g - colorCircle.g * noiseFloat(coordDistorted*0.3f, time+0.5f);
    }
    
    else if (index == 1) {
        // initial colorset
        colorCircle = vec3(circle, circle*0.3f, circle*0.5f);
        colorCircle = colorCircle*0.8 + colorCircle*(vec3(audioLevel));
        colorCircleEdge = vec3(circleEdge*0.9f, circleEdge*0.17f, circleEdge*0.321f);
    
        // pulse the color
        colorCircle.r = colorCircle.r - noiseFloat(coordDistorted*0.3f, time*0.2f)*0.3f;
        colorCircleEdge.b = colorCircleEdge.b + colorCircleEdge.b * noiseFloat(coordDistorted*0.6f, time)*1.5f;
        colorCircle.b = colorCircle.b + colorCircle.b * (noiseFloat(coordDistorted*0.23, time)*0.5f);
        colorCircle.g = colorCircle.g - colorCircle.g * noiseFloat(coordDistorted*0.3f, time+0.5f);
    }
    
    // add both colors
    return colorCircle + colorCircleEdge;
}


void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    float time = iTime;
    float modelMix = iModelMix;
    vec2 knobPos1 = iKnobPos1;
    vec2 knobPos2 = iKnobPos2;
    float audioLevel_1 = iAudioLevel1;
    float audioLevel_2 = iAudioLevel2;

    float displayScaleFactor = iDisplayScaleFactor;
    vec2 resolution = iResolution.xy * displayScaleFactor;
    
    // Shift the coordinates, since the OpenGl component is bigger than the XY-Pad
    // make the zero point at the left/button corner of the XY-Pad (dont forget the displayScaleFactor!)
    // Also keep in mind Juce counts from the top left corner and openGL from buttom left
    fragCoord = vec2(fragCoord.x - (120*displayScaleFactor), fragCoord.y - (54*displayScaleFactor));

    // center origin point
    vec2 coord = fragCoord/resolution.xy * 2.f - 1.f;
    
    // Adjust to Aspect Ratio
    float aspect = resolution.x / resolution.y;
    coord.x *= aspect;
    
    vec2 coord_1 = calcKnobPosUV(knobPos1, coord, aspect);
    vec2 coord_2 = calcKnobPosUV(knobPos2, coord, aspect);
    
    // calculate radius for both models
    float[2] radius = calculateRadius(modelMix);
   
    // Distort coordinate systems
    vec2 coordDistorted_1 = coord_1 + noiseCoord(coord_1, iTime) * 0.1;
    vec2 coordDistorted_2 = coord_2 + noiseCoord(coord_2, iTime) * 0.1;
    
    // Calculate inner circle
    float circle_1 = circle(coordDistorted_1, radius[0]);
    float circle_2 = circle(coordDistorted_2, radius[1]);
    
    // Calculate outer circle
    float circleEdge_1 = circleEdge(coordDistorted_1, circle_1, radius[0]);
    float circleEdge_2 = circleEdge(coordDistorted_2, circle_2, radius[1]);
     
    // Color circles
    vec3 colorCircle_1 = colorCircle(circle_1, circleEdge_1, audioLevel_1, coordDistorted_1, time, 0);
    vec3 colorCircle_2 = colorCircle(circle_2, circleEdge_2, audioLevel_2, coordDistorted_2, time + 14.f, 1);
    
    // Add both circles together
    vec3 color = vec3(max((colorCircle_1 + colorCircle_2), 0.));
    color = min(color, 1.);
    
    // adding Background
    color = iBackgroundColor + color;
    
    // for calibration
    // if (fragCoord.x / displayScaleFactor < 500.f && fragCoord.x >= 0.f) {
    //     if (fragCoord.y / displayScaleFactor < 500.f && fragCoord.y >= 0.f) {
    //         color = vec3(1.f, 0.f, 1.f);
    //     }
    // }

    fragColor = vec4(color, 1.f);
}