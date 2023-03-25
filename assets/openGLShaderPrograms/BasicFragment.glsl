/*
    BasicFragment.glsl
    OpenGL 3D App Template - App
 
    Created by Tim Arterbury on 3/21/20.
    Copyright 2020 TesserAct Music Technology LLC. All rights reserved.
 
    Fragment Shader
    This fragment shader simply colors all shape fragments with a purple color.
*/

#version 330 core
out vec4 fragColor;

void main()
{
    fragColor = vec4 (0.6f, 0.1f, 1.0f, 0.8f);
} 
