//
//  ShapeVertices.hpp
//  OpenGL 3D App Template - App
//
//  Created by Tim Arterbury on 3/22/20.
//  Copyright © 2020 TesserAct Music Technology LLC. All rights reserved.
//

#pragma once

#include <JuceHeader.h>

namespace ShapeVertices
{

/*
static std::vector<juce::Vector3D<GLfloat>> generateTriangle()
{
    return { { -0.5f, -0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { 0.0f,  0.5f, 0.0f } };
}*/

static std::vector<juce::Vector3D<GLfloat>> generateSquare()
{
    return { { -1.f, -1.f, 0.0f }, { -1.f, 1.f, 0.0f }, { 1.f,  1.f, 0.0f }, { -1.f, -1.f, 0.0f }, { 1.f, -1.f, 0.0f }, { 1.f,  1.f, 0.0f } };
}

/*
static std::vector<juce::Vector3D<GLfloat>> generatePyramid()
{
    return { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.5f, 0.0f }, { 0.5f,  -0.5f, -0.5f },
             { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.5f, 0.0f }, { 0.0f,  -0.5f, 0.5f },
             { 0.5f, -0.5f, -0.5f }, { 0.0f, 0.5f, 0.0f }, { 0.0f,  -0.5f, 0.5f },
             { -0.5f, -0.5f, -0.5f }, { 0.0f,  -0.5f, 0.5f }, { 0.5f, -0.5f, -0.5f }
        
    };
}*/

/*
static std::vector<juce::Vector3D<GLfloat>> generateCube()
{
    std::vector<juce::Vector3D<GLfloat>> vertices;
    
    const float offset = 0.5f;
    
    // For every face, there will be one dimension which has an unchanged value
    int unchangedDimensionIndex = 0;
    
    // Each point will either have a positive or negative value
    bool isDimensionPositive[3] { true, true, true };
    
    // Iterate through all faces
    for (int face = 0; face < 6; face++)
    {
        // Which dimension's turn is it to be swapped for the following point?
        int dimensionTurn = (unchangedDimensionIndex + 1) % 3;
        
        // Iterate through both triangles in a face
        for (int tri = 0; tri < 2; tri++)
        {
            // Iterate through all 3 points of a triangle
            for (int point = 0; point < 3; point++)
            {
                // Create a vertex
                vertices.emplace_back( isDimensionPositive[0] ? offset : -offset,
                                      isDimensionPositive[1] ? offset : -offset,
                                      isDimensionPositive[2] ? offset : -offset );
                
                // If not the 3rd vertex, swap the magnitude of the other dimension
                if (point != 2)
                {
                    isDimensionPositive[dimensionTurn] = !isDimensionPositive[dimensionTurn];
                    dimensionTurn = (dimensionTurn + 1) % 3;
                    if (dimensionTurn == unchangedDimensionIndex)
                        dimensionTurn = (dimensionTurn + 1) % 3;
                }
            }
        }
        
        unchangedDimensionIndex = (unchangedDimensionIndex + 1) % 3;
        
        // After first 3 faces, invert the magnitude of all dimensions
        if (face == 2)
            for (bool & i : isDimensionPositive)
                i = !i;
        
    }

    return vertices;
}*/

} // namespace ShapeVertices
