//
//  OpenGLComponent.hpp
//  OpenGL 3D App Template - App
//
//  Created by Tim Arterbury on 3/21/20.
//  Copyright © 2020 TesserAct Music Technology LLC. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "OpenGLUtil/OpenGLUtil.h"
#include "ShapeVertices.h"
#include "../Texture/TextureComponent.h"
#include "../../LookAndFeel/CustomFontLookAndFeel.h"
#include"../XYPad/XYPad.h"
#include "../../../PluginProcessor.h"

struct KnobPos {
    float xPosition;
    float yPosition;
};

struct Labels{
    juce::Label sharp;
    juce::Label attack;
    juce::Label smooth;
    juce::Label sustain;
} ;

/** A custom JUCE Component which renders using OpenGL. You can use this class
    as a template for any Component you want to create which renders OpenGL inside
    of its view. If you were creating a spectrum visualizer, you might name this
    class SpectrumVisualizer.
 */
class OpenGLBackground : public juce::Component,
private juce::OpenGLRenderer,
private juce::AsyncUpdater
{
public:

    OpenGLBackground(juce::AudioProcessorValueTreeState& parameters, AudioPluginAudioProcessor& p);
    ~OpenGLBackground() override;
    
    // OpenGLRenderer Callbacks ================================================
    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;
    void renderOpenGL() override;
    
    // Component Callbacks =====================================================
    void paint (juce::Graphics& g) override;
    void resized () override;

    // Used to connect Draggable3DOrientation to mouse movements
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    
    // AsyncUpdater Callback ===================================================
    /** If the OpenGLRenderer thread needs to update some form JUCE GUI object
        reserved for the JUCE Message Thread, this callback allows the message
        thread to handle the update. */
    void handleAsyncUpdate() override;
    void setSharpnessParamActive(bool active, int modelID);

    void externalModelLoaded(int modelID, juce::String modelName) {
        xyPad.updateKnobName(modelID, modelName);
    }

private:
    
    /** Attempts to compile the OpenGL program at runtime and setup OpenGL variables. */
    void compileOpenGLShaderProgram();

    // OpenGL Variables
    juce::OpenGLContext openGLContext;
    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    
    OpenGLUtil::UniformWrapper resolution {"iResolution"};
    OpenGLUtil::UniformWrapper time {"iTime"};
    OpenGLUtil::UniformWrapper displayScaleFactor {"iDisplayScaleFactor"};
    OpenGLUtil::UniformWrapper backgroundColor {"iBackgroundColor"};
    OpenGLUtil::UniformWrapper knobPos1 {"iKnobPos1"};
    OpenGLUtil::UniformWrapper knobPos2 {"iKnobPos2"};
    OpenGLUtil::UniformWrapper modelMix {"iModelMix"};
    OpenGLUtil::UniformWrapper audioLevel {"iAudioLevel"};
    // Fade aus ValueTreeState
    // On Off pro source
    
    GLuint VAO, VBO;
    std::vector<juce::Vector3D<GLfloat>> vertices;
    
    juce::Array<GLfloat> resolution_juce;
    GLfloat displayScaleFactor_juce;
    juce::Colour backgroundColor_juce;
    float audioLevel_juce;
    AudioPluginAudioProcessor& processorRef;
    
    // GUI Mouse Drag Interaction
    juce::Draggable3DOrientation draggableOrientation;
    
    // GUI overlay status text
    juce::String openGLStatusText;
    juce::Label openGLStatusLabel;
    
    XYPad xyPad;
    KnobPos knobPos1_juce;
    KnobPos knobPos2_juce;
    float modelMix_juce;
    float sharpnesParamActive1_juce;
    float sharpnesParamActive2_juce;
    void xyButtonMoved(float x, float y, int modelID);
    void xyModelMixChanged(float modelMix);
    OpenGLTextureComponent openGlTextureComponent;
    Labels labels;
    void SetJuceLabels();
    CustomFontLookAndFeel customFontLookAndFeel;
};

