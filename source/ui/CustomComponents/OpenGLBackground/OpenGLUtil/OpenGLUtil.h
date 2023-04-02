//
//  OpenGLUtil.hpp
//  OpenGL 3D App Template - App
//
//  Created by Tim Arterbury on 3/21/20.
//  Copyright © 2020 TesserAct Music Technology LLC. All rights reserved.
//

#pragma once

namespace OpenGLUtil
{
// OpenGL Uniform & Attribute Helpers ==========================================

/** Fail-safe method for creating an OpenGLShaderProgram::Uniform.
 
 If a uniform with the given `uniformName` is not found within the
 `shaderProgram`, then it is invalid, and nullptr is returned. If in debug mode,
 this will notify you of the naming error and provides a descriptive error message.
 */
static juce::OpenGLShaderProgram::Uniform* createUniform (juce::OpenGLContext& context,
                                                    juce::OpenGLShaderProgram& shaderProgram,
                                                    const juce::String& uniformName)
{
    bool uniformNameFoundInShaderProgram = context.extensions.glGetUniformLocation (
                                                                                    shaderProgram.getProgramID(), uniformName.toRawUTF8()) >= 0;
    
#if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
    /** If you hit this assertion, then you have attempted to create an
     OpenGLShaderProgram::Uniform with a name that is not found in the
     OpenGLShaderProgram it is linked to. Make sure your
     OpenGLShaderProgram::Uniform name matches the uniform names in your
     GLSL shader programs.
     */
//    jassert (uniformNameFoundInShaderProgram);
#endif
    
    if (!uniformNameFoundInShaderProgram)
        return nullptr;
    
    return new juce::OpenGLShaderProgram::Uniform (shaderProgram, uniformName.toRawUTF8());
}

/** Fail-safe method for creating an OpenGLShaderProgram::Attribute.
 
 If an attribute with the given `attributeName` is not found within the
 `shaderProgram`, then it is invalid, and nullptr is returned. If in debug mode,
 this will notify you of the naming error and provides a descriptive error message.
 */
static juce::OpenGLShaderProgram::Attribute* createAttribute (juce::OpenGLContext& context,
                                                        juce::OpenGLShaderProgram& shaderProgram,
                                                        const juce::String& attributeName)
{
    bool attributeNameFoundInShaderProgram = context.extensions.glGetAttribLocation (
                                                                                     shaderProgram.getProgramID(), attributeName.toRawUTF8()) >= 0;
    
#if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
    /** If you hit this assertion, then you have attempted to create an
     OpenGLShaderProgram::Attribute with a name that is not found in the
     OpenGLShaderProgram it is linked to. Make sure your
     OpenGLShaderProgram::Attribute name matches the attribute names in your
     GLSL shader programs.
     */
    jassert (attributeNameFoundInShaderProgram);
#endif
    
    if (!attributeNameFoundInShaderProgram)
        return nullptr;
    
    return new juce::OpenGLShaderProgram::Attribute (shaderProgram, attributeName.toRawUTF8());
}


/** Wrapper class that makes it easier to work with OpenGLShaderProgram::Uniform
 and OpenGLShaderProgram::Attribute. It permanently associates a particular
 name String with the uniform or attribute which is usually how Uniform and
 Attribute objects tend to be used. This makes use of these objects a bit
 cleaner instead of having the rewrite name strings in multiple places.
 
 For ease of use, the interface is similar to a pointer/unique_ptr with methods
 `get()`, `operator bool()`, and `operator->()`.
 
 I hope functionality such as this gets integrated into the juce::Uniform class
 in the future.
 */
template<class InternalType, InternalType * (*factoryFunction)(juce::OpenGLContext&, juce::OpenGLShaderProgram&, const juce::String&)>
class OpenGLNamedIDWrapper
{
public:
    OpenGLNamedIDWrapper (const juce::String & idName)
    {
        this->idName = idName;
    }
    
    InternalType * get() const noexcept
    {
        return uniform.get();
    }
    
    operator bool() const noexcept
    {
        return uniform.get() != nullptr;
    }
    
    InternalType * operator->() const noexcept
    {
        return uniform.get();
    }
    
    void connectToShaderProgram (juce::OpenGLContext & context, juce::OpenGLShaderProgram & shaderProgram)
    {
        uniform.reset (factoryFunction (context, shaderProgram, idName));
    }
    
    void disconnectFromShaderProgram()
    {
        uniform.reset();
    }
    
private:
    juce::String idName;
    std::unique_ptr<InternalType> uniform;
};


/** Wrapper class around OpenGLShaderProgram::Uniform for ease of use with a
 permanetly associated name. See `OpenGLNamedIDWrapper` for more information.
 */
typedef OpenGLNamedIDWrapper<juce::OpenGLShaderProgram::Uniform, createUniform> UniformWrapper;

/** Wrapper class around OpenGLShaderProgram::Uniform for ease of use with a
 permanetly associated name. See `OpenGLNamedIDWrapper` for more information.
 */
typedef OpenGLNamedIDWrapper<juce::OpenGLShaderProgram::Attribute, createAttribute> AttributeWrapper;


//==============================================================================
// This class just manages the attributes that the shaders use.
/*struct Attributes
 {
 Attributes (OpenGLContext& context, OpenGLShaderProgram& shaderProgram)
 {
 position      .reset (createAttribute (context, shaderProgram, "position"));
 normal        .reset (createAttribute (context, shaderProgram, "normal"));
 sourceColour  .reset (createAttribute (context, shaderProgram, "sourceColour"));
 textureCoordIn.reset (createAttribute (context, shaderProgram, "textureCoordIn"));
 }
 
 void enable (OpenGLContext& context)
 {
 if (position.get() != nullptr)
 {
 context.extensions.glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
 context.extensions.glEnableVertexAttribArray (position->attributeID);
 }
 
 if (normal.get() != nullptr)
 {
 context.extensions.glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
 context.extensions.glEnableVertexAttribArray (normal->attributeID);
 }
 
 if (sourceColour.get() != nullptr)
 {
 context.extensions.glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
 context.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
 }
 
 if (textureCoordIn.get() != nullptr)
 {
 context.extensions.glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
 context.extensions.glEnableVertexAttribArray (textureCoordIn->attributeID);
 }
 }
 
 void disable (OpenGLContext& context)
 {
 if (position.get() != nullptr)       context.extensions.glDisableVertexAttribArray (position->attributeID);
 if (normal.get() != nullptr)         context.extensions.glDisableVertexAttribArray (normal->attributeID);
 if (sourceColour.get() != nullptr)   context.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
 if (textureCoordIn.get() != nullptr) context.extensions.glDisableVertexAttribArray (textureCoordIn->attributeID);
 }
 
 std::unique_ptr<OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn;
 
 private:
 static OpenGLShaderProgram::Attribute* createAttribute (OpenGLContext& context,
 OpenGLShaderProgram& shader,
 const String& attributeName)
 {
 if (context.extensions.glGetAttribLocation (shader.getProgramID(), attributeName.toRawUTF8()) < 0)
 return nullptr;
 
 return new OpenGLShaderProgram::Attribute (shader, attributeName.toRawUTF8());
 }
 };
 */
//==============================================================================

/** Vertex data to be passed to the shaders.
 For the purposes of this demo, each vertex will have a 3D position, a colour and a
 2D texture co-ordinate. Of course you can ignore these or manipulate them in the
 shader programs but are some useful defaults to work from.
 */
//struct Vertex
//{
//    float position[3];
//    float normal[3];
//    float colour[4];
//    float texCoord[2];
//};

} // OpenGLUtil
