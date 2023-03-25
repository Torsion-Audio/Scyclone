//
//  OpenGLBackground.cpp
//  OpenGL 3D App Template - App
//
//  Created by Tim Arterbury on 3/21/20.
//  Copyright © 2020 TesserAct Music Technology LLC. All rights reserved.
//

#include "OpenGLBackground.h"


OpenGLBackground::OpenGLBackground(juce::AudioProcessorValueTreeState& parameters, AudioPluginAudioProcessor& p) : processorRef(p), xyPad(parameters, p)
{
    // Sets the OpenGL version to 3.2
    openGLContext.setOpenGLVersionRequired (juce::OpenGLContext::OpenGLVersion::openGL3_2);

    // Set default 3D orientation for the draggable GUI tool
    draggableOrientation.reset ({ 0.0, 1.0, 0.0 });

    // Attach the OpenGL context
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);
    openGLContext.setContinuousRepainting (true); // Enable rendering

    // Setup OpenGL GUI Overlay Label: Status of Shaders, compiler errors, etc.
    //addAndMakeVisible (openGLStatusLabel);
    openGLStatusLabel.setJustificationType (juce::Justification::topLeft);
    openGLStatusLabel.setFont (juce::Font (14.0f));
    
    addAndMakeVisible(xyPad);
    xyPad.onButtonMove = [this](float x, float y, int no){this->xyButtonMoved(x,y,no);};
    xyPad.onModelMixChange = [this](float modelMix){this->xyModelMixChanged(modelMix);};
    SetJuceLabels();
    addAndMakeVisible(openGlTextureComponent);
    
    processorRef.setLevelType(PEAK);

    resolution_juce.add((float) getWidth());
    resolution_juce.add((float) getHeight());

    // dirty work around to make the blobs appear correctly from the beginning
    auto fadeParam = parameters.getParameter(PluginParameters::FADE_ID);
    auto fadeStatus = fadeParam->getValue();
    fadeParam->setValueNotifyingHost(0.5f*fadeStatus);
    fadeParam->setValueNotifyingHost(fadeStatus);
}

OpenGLBackground::~OpenGLBackground()
{
    openGLContext.setContinuousRepainting (false);
    openGLContext.detach();
}

// OpenGLRenderer Callbacks ================================================
void OpenGLBackground::newOpenGLContextCreated()
{
    compileOpenGLShaderProgram();
    
    vertices = ShapeVertices::generateSquare(); // Setup vertices
    
    // Generate opengl vertex objects ==========================================
    openGLContext.extensions.glGenVertexArrays(1, &VAO); // Vertex Array Object
    openGLContext.extensions.glGenBuffers (1, &VBO);     // Vertex Buffer Object
    
    
    // Bind opengl vertex objects to data ======================================
    openGLContext.extensions.glBindVertexArray (VAO);
//
//    // Fill VBO buffer with vertices array
    openGLContext.extensions.glBindBuffer (juce::gl::GL_ARRAY_BUFFER, VBO);
    openGLContext.extensions.glBufferData (juce::gl::GL_ARRAY_BUFFER,
                                           sizeof (GLfloat) * vertices.size() * 3,
                                           vertices.data(),
                                           juce::gl::GL_STATIC_DRAW);
//
//
//    // Define that our vertices are laid out as groups of 3 GLfloats
    openGLContext.extensions.glVertexAttribPointer (0, 3, juce::gl::GL_FLOAT, juce::gl::GL_FALSE,
                                                    3 * sizeof (GLfloat), NULL);
    openGLContext.extensions.glEnableVertexAttribArray (0);
    
    
    // Optional OpenGL styling commands ========================================
    
    juce::gl::glEnable (juce::gl::GL_BLEND); // Enable alpha blending, allowing for transparency of objects
    juce::gl::glBlendFunc (juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA); // Paired with above line
    
//     juce::gl::glPolygonMode(juce::gl::GL_FRONT_AND_BACK, juce::gl::GL_LINE); // Show wireframe
}

void OpenGLBackground::openGLContextClosing()
{
    // Add any OpenGL related cleanup code here . . .
}

void OpenGLBackground::renderOpenGL()
{
    jassert (juce::OpenGLHelpers::isContextActive());
    
    // Scale viewport
    const float renderingScale = (float) openGLContext.getRenderingScale();
    juce::gl::glViewport (0, 0, juce::roundToInt (renderingScale * getWidth()), juce::roundToInt (renderingScale * getHeight()));

    // Set background color
//    OpenGLHelpers::clear (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

//    // Select shader program
    shaderProgram->use();
//
//    // Setup the Uniforms for use in the Shader
    if (resolution) resolution->set(resolution_juce[0], resolution_juce[1]);
    if (displayScaleFactor) displayScaleFactor->set(displayScaleFactor_juce);
    if (backgroundColor) backgroundColor->set(static_cast<GLfloat>(backgroundColor_juce.getFloatRed()), static_cast<GLfloat>(backgroundColor_juce.getFloatGreen()), static_cast<GLfloat>(backgroundColor_juce.getFloatBlue()));
    if (time){
        const auto time_juce = juce::Time::getMillisecondCounter();
        auto currentTimeSeconds = float(time_juce/1000.0);
        //        std::cout << "Time: " << currentTimeSeconds << std::endl;
        time->set(currentTimeSeconds);
    }
    if (modelMix) modelMix->set(modelMix_juce);
    if (knobPos1) knobPos1->set(knobPos1_juce.xPosition,knobPos1_juce.yPosition);
    if (knobPos2) knobPos2->set(knobPos2_juce.xPosition,knobPos2_juce.yPosition);
    if (audioLevel){
        audioLevel_juce = processorRef.getCurrentLevel();
        audioLevel->set(static_cast<GLfloat>(audioLevel_juce));
    }
    
//
//    // Draw Vertices
    openGLContext.extensions.glBindVertexArray (VAO);
    juce::gl::glDrawArrays (juce::gl::GL_TRIANGLES, 0, (int) vertices.size());
    openGLContext.extensions.glBindVertexArray (0);
}

// JUCE Component Callbacks ====================================================
void OpenGLBackground::paint (juce::Graphics&)
{
    // You can optionally paint any JUCE graphics over the top of your OpenGL graphics
}

void OpenGLBackground::resized ()
{
    resolution_juce = {static_cast<GLfloat>(getWidth()),static_cast<GLfloat>(getHeight())};
    displayScaleFactor_juce = static_cast<GLfloat>(juce::Desktop::getInstance().getDisplays().displays.getFirst().scale);
    DBG(displayScaleFactor_juce);
    draggableOrientation.setViewport (getLocalBounds());
    xyPad.setBounds(120, 94, 500, 500);
    backgroundColor_juce = juce::Colour::fromString(ColorPallete::BG);
    openGLStatusLabel.setBounds (getLocalBounds().reduced (4).removeFromTop (75));
    openGlTextureComponent.setBounds(getLocalBounds());
    labels.attack.setBounds(50, 335, 70, 19);
    labels.sharp.setBounds(347, 60, 70, 19);
    labels.sustain.setBounds(636, 335, 70, 19);
    labels.smooth.setBounds(340, 605, 70, 19);
}

void OpenGLBackground::mouseDown (const juce::MouseEvent& e)
{
    draggableOrientation.mouseDown (e.getPosition());
}

void OpenGLBackground::mouseDrag (const juce::MouseEvent& e)
{
    draggableOrientation.mouseDrag (e.getPosition());
}

void OpenGLBackground::handleAsyncUpdate()
{
    openGLStatusLabel.setText (openGLStatusText, juce::dontSendNotification);
}

// OpenGL Related Member Functions =============================================
void OpenGLBackground::compileOpenGLShaderProgram()
{
    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgramAttempt
        = std::make_unique<juce::OpenGLShaderProgram> (openGLContext);

    // Attempt to compile the program
    if (shaderProgramAttempt->addVertexShader ({ BinaryData::BasicVertex_glsl })
        && shaderProgramAttempt->addFragmentShader ({ BinaryData::blob4_glsl })
        && shaderProgramAttempt->link())
    {

        resolution.disconnectFromShaderProgram();
        time.disconnectFromShaderProgram();
        displayScaleFactor.disconnectFromShaderProgram();
        backgroundColor.disconnectFromShaderProgram();
        modelMix.disconnectFromShaderProgram();
        knobPos1.disconnectFromShaderProgram();
        knobPos2.disconnectFromShaderProgram();
        audioLevel.disconnectFromShaderProgram();
        
        shaderProgram.reset (shaderProgramAttempt.release());

//        std::cout << "UniformLocation: " << juce::gl::glGetUniformLocation(shaderProgram->getProgramID(), "resolution") << std::endl;
        
        resolution.connectToShaderProgram (openGLContext, *shaderProgram);
        time.connectToShaderProgram (openGLContext, *shaderProgram);
        displayScaleFactor.connectToShaderProgram(openGLContext, *shaderProgram);
        backgroundColor.connectToShaderProgram(openGLContext, *shaderProgram);
        modelMix.connectToShaderProgram(openGLContext, *shaderProgram);
        knobPos1.connectToShaderProgram(openGLContext, *shaderProgram);
        knobPos2.connectToShaderProgram(openGLContext, *shaderProgram);
        audioLevel.connectToShaderProgram(openGLContext, *shaderProgram);
        
        openGLStatusText = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
        
    }
    else
    {
        openGLStatusText = shaderProgramAttempt->getLastError();
    }

    triggerAsyncUpdate(); // Update status text
}

void OpenGLBackground::xyButtonMoved(float x, float y, int modelID) {
    x = x + 0.05f;
    if (modelID == 1)
    {
        if (x >= 0.15f && x <= 0.9f) knobPos1_juce.xPosition = x;
        else if (x < 0.15f) knobPos1_juce.xPosition = 0.15f;
        else knobPos1_juce.xPosition = 0.9f;
        if (y >= 0.15f && y <= 0.9f) knobPos1_juce.yPosition = y;
        else if (y < 0.15f) knobPos1_juce.yPosition = 0.15f;
        else knobPos1_juce.yPosition = 0.9f;
//        std::cout << knobPos1_juce.xPosition << ", " << knobPos1_juce.yPosition << std::endl;
    }
    else if (modelID == 2)
    {
        if (x >= 0.15f && x <= 0.9f) knobPos2_juce.xPosition = x;
        else if (x < 0.15f) knobPos2_juce.xPosition = 0.15f;
        else knobPos2_juce.xPosition = 0.9f;
        if (y >= 0.15f && y <= 0.9f) knobPos2_juce.yPosition = y;
        else if (y < 0.15f) knobPos2_juce.yPosition = 0.15f;
        else knobPos2_juce.yPosition = 0.9f;
//        std::cout << knobPos2_juce.xPosition << ", " << knobPos2_juce.yPosition << std::endl;
    }
}

void OpenGLBackground::xyModelMixChanged(float newModelMix) {
    modelMix_juce = newModelMix;
}


void  OpenGLBackground::SetJuceLabels()
{

    auto font = CustomFontLookAndFeel::getCustomFont().withHeight(19.0f);

    labels.sharp.setText("Sharp", juce::dontSendNotification);
    labels.sharp.setFont(font);
    labels.sharp.setJustificationType(juce::Justification::centredLeft);
    labels.sharp.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(labels.sharp);

    labels.sustain.setText("Sustain", juce::dontSendNotification);
    labels.sustain.setFont(font);
    labels.sustain.setJustificationType(juce::Justification::centredLeft);
    labels.sustain.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(labels.sustain);

    labels.smooth.setText("Smooth", juce::dontSendNotification);
    labels.smooth.setFont(font);
    labels.smooth.setJustificationType(juce::Justification::centredLeft);
    labels.smooth.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(labels.smooth);

    labels.attack.setText("Attack", juce::dontSendNotification);
    labels.attack.setFont(font);
    labels.attack.setJustificationType(juce::Justification::centredLeft);
    labels.attack.setColour(juce::Label::ColourIds::textColourId, juce::Colour::fromString(ColorPallete::TEXT2));
    addAndMakeVisible(labels.attack);
}

void OpenGLBackground::setSharpnessParamActive(bool active, int modelID){
    if (modelID == 1)
    {
        if (active) sharpnesParamActive1_juce = 1.f;
        else sharpnesParamActive1_juce = 0.f;
    }
    else if (modelID == 2)
    {
        if (active) sharpnesParamActive2_juce = 1.f;
        else sharpnesParamActive2_juce = 0.f;
    }

    juce::ignoreUnused(sharpnesParamActive1_juce, sharpnesParamActive2_juce);
}
