/*
    BasicVertex.glsl
    OpenGL 3D App Template - App
 
    Created by Tim Arterbury on 3/21/20.
    Copyright 2020 TesserAct Music Technology LLC. All rights reserved.
 
    Vertex Shader
    This vertex shader takes the object vertices and applies view and projection
    based transformations to those vertices.
*/

#version 330 core
layout (location = 0) in vec3 position;

void main()
{
    gl_Position = vec4 (position.x, position.y, position.z, 1.f);
}
