#include "XYPad.h"

XYPad::XYPad(juce::AudioProcessorValueTreeState& parameters, AudioPluginAudioProcessor& p) :
        parameters(parameters),
        processor(p),
        network1SelectButton(1, *parameters.getParameter(PluginParameters::SELECT_NETWORK1_ID.getParamID())),
        network1GrainButton(2, *parameters.getParameter(PluginParameters::GRAIN_ON_OFF_NETWORK1_ID.getParamID())),
        network1OnOffButton(3, *parameters.getParameter(PluginParameters::ON_OFF_NETWORK1_ID.getParamID())),
        network2SelectButton(1, *parameters.getParameter(PluginParameters::SELECT_NETWORK2_ID.getParamID())),
        network2GrainButton(2, *parameters.getParameter(PluginParameters::GRAIN_ON_OFF_NETWORK2_ID.getParamID())),
        network2OnOffButton(3, *parameters.getParameter(PluginParameters::ON_OFF_NETWORK2_ID.getParamID())),
        knob1(parameters, 1, arrow1, network1SelectButton, network1GrainButton, network1OnOffButton),
        knob2(parameters, 2, arrow2, network2SelectButton, network2GrainButton, network2OnOffButton),
        arrow2(parameters, 1),
        arrow1(parameters, 2)
{
	addMouseListener(this, true);
	setComponentID("xyPad");
	// create Drawables from Binary
	octagon = juce::Drawable::createFromImageData(BinaryData::Octagon_svg, BinaryData::Octagon_svgSize);
    parameters.addParameterListener(PluginParameters::FADE_ID.getParamID(), this);
    parameters.addParameterListener(PluginParameters::SELECT_NETWORK1_ID.getParamID(), this);
    parameters.addParameterListener(PluginParameters::SELECT_NETWORK2_ID.getParamID(), this);

	knob1.setComponentID("knob1");
	knob2.setComponentID("knob2");

	arrow2.setOrientation(Arrow::orientations::upRight);
	arrow1.setOrientation(Arrow::orientations::downLeft);

    arrow1.setComponentID("arrow1");
    arrow2.setComponentID("arrow2");

    network1SelectButton.setComponentID("network1SelectButton");
    network1GrainButton.setComponentID("network1GrainButton");
    network1OnOffButton.setComponentID("network1OnOffButton");

    network2SelectButton.setComponentID("network2SelectButton");
    network2GrainButton.setComponentID("network2GrainButton");
    network2OnOffButton.setComponentID("network2OnOffButton");


    addAndMakeVisible(knob1);
    addAndMakeVisible(arrow1);
    addAndMakeVisible(network1SelectButton);
	addAndMakeVisible(network1GrainButton);
	addAndMakeVisible(network1OnOffButton);
    setArrowAndButtonsVisible(false, 1);

	addAndMakeVisible(knob2);
	addAndMakeVisible(arrow2);
	addAndMakeVisible(network2SelectButton);
	addAndMakeVisible(network2GrainButton);
	addAndMakeVisible(network2OnOffButton);
    setArrowAndButtonsVisible(false, 2);

    // find and connect audio parameters to automatable gui objects
    auto x1 = parameters.getParameter(PluginParameters::TRAN_SHAPER_NETWORK1_ID.getParamID());
    auto y1 = parameters.getParameter(PluginParameters::FILTER_NETWORK1_ID.getParamID());
    auto x2 = parameters.getParameter(PluginParameters::TRAN_SHAPER_NETWORK2_ID.getParamID());
    auto y2 = parameters.getParameter(PluginParameters::FILTER_NETWORK2_ID.getParamID());
    connectParameters(*x1, *y1, *x2, *y2);

    processor.onNetwork1NameChange = [this](juce::String newName){knob1.setName(newName);};
    processor.onNetwork2NameChange = [this](juce::String newName){knob2.setName(newName);};

    componentAnimator = std::make_unique<juce::ComponentAnimator>();
    juce::String n1Name = processor.network1Name.toString();
    knob1.setName(n1Name);
    juce::String n2Name = processor.network2Name.toString();
    knob2.setName(n2Name);
}

XYPad::~XYPad() {
    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.removeParameterListener(parameterID, this);
    }
}

void XYPad::paint(juce::Graphics& g)
{
	g.setColour(juce::Colour::fromString(ColorPallete::OCTAGON));
    int lineWidth = 3;
    octagon->drawWithin(g, getLocalBounds().reduced(lineWidth).toFloat(), juce::RectanglePlacement::centred, 1.0);
}

void XYPad::resized()
{
    moveButton(&knob1, 1);
    moveButton(&knob2, 2);
}

void XYPad::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == PluginParameters::SELECT_NETWORK1_ID.getParamID() && newValue == 0.f) {
        updateKnobName(1, network1Name);
    } else if (parameterID == PluginParameters::SELECT_NETWORK2_ID.getParamID() && newValue == 0.f) {
        updateKnobName(2, network2Name);
    }
    
    /*
    int newDiameter1 = calcCurrentButtonDiameters(1);
    int newDiameter2 = calcCurrentButtonDiameters(2);

    if (parameterID == PluginParameters::FADE_ID.getParamID()){
        setKnobButtonDiameter(newDiameter1, 1);
        setKnobButtonDiameter(newDiameter2, 2);
    }
     */

    if (parameterID == PluginParameters::FADE_ID.getParamID()){
        float fadeValue = parameters.getRawParameterValue(PluginParameters::FADE_ID.getParamID())->load();
        onModelMixChange(fadeValue);
    }
}

void XYPad::mouseDown(const juce::MouseEvent& e)
{
    auto eventComponentID = e.eventComponent->getComponentID();

    if (eventComponentID == knob1.getComponentID() || eventComponentID == knob1.getLabelID())
	{
		knob1XAttachment->beginGesture();
		knob1YAttachment->beginGesture();
		knobOnDrag = KnobOnDrag::knob1OnDrag;
	}
	else if (eventComponentID == knob2.getComponentID() || eventComponentID == knob2.getLabelID())
	{
		knob2XAttachment->beginGesture();
		knob2YAttachment->beginGesture();
        knobOnDrag = KnobOnDrag::knob2OnDrag;
	}
}

void XYPad::mouseDrag(const juce::MouseEvent&)
{
	if (knobOnDrag == KnobOnDrag::knob1OnDrag)
	{
		knob1XAttachment->setValueAsPartOfGesture(pixelToNormWidth(getMouseXYRelative().x));
		knob1YAttachment->setValueAsPartOfGesture(pixelToNormHeight(getMouseXYRelative().y));
	}
	else if (knobOnDrag == KnobOnDrag::knob2OnDrag)
	{
		knob2XAttachment->setValueAsPartOfGesture(pixelToNormWidth(getMouseXYRelative().x));
		knob2YAttachment->setValueAsPartOfGesture(pixelToNormHeight(getMouseXYRelative().y));
	}
}

void XYPad::mouseUp(const juce::MouseEvent&)
{
	if (knobOnDrag == KnobOnDrag::knob1OnDrag)
	{
		knob1XAttachment->endGesture();
		knob1YAttachment->endGesture();
		knobOnDrag = KnobOnDrag::noKnobOnDrag;
	}
	else if (knobOnDrag == KnobOnDrag::knob2OnDrag)
	{
		knob2XAttachment->endGesture();
		knob2YAttachment->endGesture();
		knobOnDrag = KnobOnDrag::noKnobOnDrag;
	}
}

void XYPad::mouseEnter(const juce::MouseEvent &e)
{
    if (isButton1OnHover(e))
    {
        stopTimer();
        setArrowAndButtonsVisible(true, 1);
        setArrowAndButtonsVisible(false, 2);
        knobOnHover = KnobOnHover::knob1OnHover;
    }
    else if (isButton2OnHover(e))
    {
        stopTimer();
        setArrowAndButtonsVisible(true, 2);
        setArrowAndButtonsVisible(false, 1);
        knobOnHover = KnobOnHover::knob2OnHover;
    }
}


void XYPad::mouseExit(const juce::MouseEvent&)
{
    startTimerHz(20);
}

void XYPad::setArrowAndButtonsVisible(bool newState, int buttonNumber)
{
    int fadeTime = 300;

    if (buttonNumber == 1)
    {
        if (newState)
        {
            componentAnimator->fadeIn(&arrow1, fadeTime);
            componentAnimator->fadeIn(&network1SelectButton, fadeTime);
            componentAnimator->fadeIn(&network1GrainButton, fadeTime);
            componentAnimator->fadeIn(&network1OnOffButton, fadeTime);
        }
        else
        {
            componentAnimator->fadeOut(&arrow1, fadeTime);
            componentAnimator->fadeOut(&network1SelectButton, fadeTime);
            componentAnimator->fadeOut(&network1GrainButton, fadeTime);
            componentAnimator->fadeOut(&network1OnOffButton, fadeTime);
        }
    }
    if (buttonNumber == 2)
    {
        if (newState)
        {
            componentAnimator->fadeIn(&arrow2, fadeTime);
            componentAnimator->fadeIn(&network2SelectButton, fadeTime);
            componentAnimator->fadeIn(&network2GrainButton, fadeTime);
            componentAnimator->fadeIn(&network2OnOffButton, fadeTime);
        }
        else
        {
            componentAnimator->fadeOut(&arrow2, fadeTime);
            componentAnimator->fadeOut(&network2SelectButton, fadeTime);
            componentAnimator->fadeOut(&network2GrainButton, fadeTime);
            componentAnimator->fadeOut(&network2OnOffButton, fadeTime);
        }
    }
}

bool XYPad::isButton1OnHover(const juce::MouseEvent &e) {
    auto eventComponentID = e.eventComponent->getComponentID();

    if (eventComponentID == knob1.getComponentID() ||
        eventComponentID == knob1.getLabelID() ||
        eventComponentID == arrow1.getComponentID() ||
        eventComponentID == network1SelectButton.getComponentID() ||
        eventComponentID == network1GrainButton.getComponentID() ||
        eventComponentID == network1OnOffButton.getComponentID())
        return true;
    else
        return false;
}
bool XYPad::isButton2OnHover(const juce::MouseEvent &e)
{
    auto eventComponentID = e.eventComponent->getComponentID();

    if (eventComponentID == knob2.getComponentID() ||
        eventComponentID == knob2.getLabelID() ||
        eventComponentID == arrow2.getComponentID() ||
        eventComponentID == network2SelectButton.getComponentID() ||
        eventComponentID == network2GrainButton.getComponentID() ||
        eventComponentID == network2OnOffButton.getComponentID())
        return true;
    else
        return false;
}


void XYPad::timerCallback()
{
    timerCounter += 1;
    if (timerCounter == 20*4)
    {
        if (knobOnHover == KnobOnHover::knob1OnHover)
            setArrowAndButtonsVisible(false, 1);
        else if (knobOnHover == KnobOnHover::knob2OnHover)
            setArrowAndButtonsVisible(false, 2);
        stopTimer();
        knobOnHover = KnobOnHover::noKnobOnHover;
        timerCounter = 0;
    }
}

void XYPad::moveButton(XYKnob* knob, int knobNumber)
{
    int knobButtonDiameter = knobButtonNeutralDiameter;

    float x1Norm = *parameters.getRawParameterValue(PluginParameters::TRAN_SHAPER_NETWORK1_ID.getParamID());
	float y1Norm = *parameters.getRawParameterValue(PluginParameters::FILTER_NETWORK1_ID.getParamID());
	float x2Norm = *parameters.getRawParameterValue(PluginParameters::TRAN_SHAPER_NETWORK2_ID.getParamID());
	float y2Norm = *parameters.getRawParameterValue(PluginParameters::FILTER_NETWORK2_ID.getParamID());

	auto xPixel = (float) normToPixelWidth(x1Norm);
	auto yPixel = (float) normToPixelHeight(y1Norm);

	if (knobNumber == 1)
	{
		xPixel = (float) normToPixelWidth(x1Norm);
		yPixel = (float) normToPixelHeight(y1Norm);

		moveArrow(&arrow1, x1Norm, y1Norm, (int) xPixel, (int) yPixel, knobButtonDiameter);
		moveArrowButton(&network1SelectButton, &arrow1);
		moveArrowButton(&network1GrainButton, &arrow1);
		moveArrowButton(&network1OnOffButton, &arrow1);
	}
	else if (knobNumber == 2)
	{
		xPixel = (float) normToPixelWidth(x2Norm);
		yPixel = (float) normToPixelHeight(y2Norm);
		
		moveArrow(&arrow2, x2Norm, y2Norm, (int) xPixel, (int) yPixel, knobButtonDiameter);
		moveArrowButton(&network2SelectButton, &arrow2);
		moveArrowButton(&network2GrainButton, &arrow2);
		moveArrowButton(&network2OnOffButton, &arrow2);
	}

	bool xInside = false;
	bool yInside = false;


	if (xPixel > (float) knobButtonDiameter / 2.f && xPixel < (float) getWidth() - (float) knobButtonDiameter / 2.f)
        xInside = true;
	if (yPixel > (float) knobButtonDiameter / 2.f && yPixel < (float) getHeight() - (float) knobButtonDiameter / 2.f)
        yInside = true;


	if (xInside && yInside) {
        knob->setBounds((int) xPixel - knobButtonDiameter / 2, (int) yPixel - knobButtonDiameter / 2, knobButtonDiameter,
                        knobButtonDiameter);
        if (knobNumber == 1) onButtonMove(x1Norm, y1Norm, 1);
        else if (knobNumber == 2) onButtonMove(x2Norm, y2Norm, 2);
    }
	else if (!xInside && yInside) {
        knob->setBounds(knob->getX(), (int) yPixel - knobButtonDiameter / 2, knobButtonDiameter, knobButtonDiameter);
        if (knobNumber == 1) onButtonMove(x1Norm, y1Norm, 1);
        else if (knobNumber == 2) onButtonMove(x2Norm, y2Norm, 2);
    }
	else if (xInside && !yInside){
        knob->setBounds((int) xPixel - knobButtonDiameter / 2, (int) knob->getY(), knobButtonDiameter, knobButtonDiameter);
        if (knobNumber == 1) onButtonMove(x1Norm, y1Norm, 1);
        else if (knobNumber == 2) onButtonMove(x2Norm, y2Norm, 2);
    }
	repaint();
}

void XYPad::moveArrow(Arrow* arrow, float xNorm, float yNorm, int xPixel, int yPixel, int knobButtonDiameter)
{
	int quadrant{};
	int xOffset = (int)((double)knobButtonDiameter/2);
	int yOffset = (int)((double)knobButtonDiameter/2);
    int newX = 0;
    int newY = 0;

    int sloppyDecompensation = 4;
	if (xNorm > 0.5 && yNorm > 0.5)
	{
		quadrant = Quadrant::upRight;
        newX = xPixel + xOffset;
        newY = yPixel - yOffset - arrowHeight + sloppyDecompensation;
	}
	else if (xNorm <= 0.5 && yNorm > 0.5)
	{
		quadrant = Quadrant::upLeft;
        newX = xPixel - xOffset - arrowWidth;
        newY = yPixel - yOffset - arrowHeight + sloppyDecompensation;
	}
	else if (xNorm > 0.5 && yNorm < 0.5)
	{
		quadrant = Quadrant::downRight;
        newX = xPixel + xOffset;
        newY = yPixel + yOffset;
    }
	else if (xNorm <= 0.5 && yNorm <= 0.5)
	{
		quadrant = Quadrant::downLeft;
        newX = xPixel - xOffset - arrowWidth;
        newY = yPixel + yOffset;
    }

    arrow->setOrientation(quadrant);

    if (xPixel - xOffset - arrowWidth<= 0.001)
        newX = 0;
    else if (xPixel + xOffset + arrowWidth >= getWidth())
        newX = getWidth() - arrowWidth;

    if (yPixel - yOffset - arrowHeight<= 0.001)
        newY =  0;
    else if (yPixel + yOffset + arrowHeight> getHeight())
        newY =  getHeight() - yOffset;

    arrow->setBounds(newX, newY, arrowWidth, arrowHeight);
}

void XYPad::moveArrowButton(ArrowButtons* arrowButton, Arrow* arrow) const
{
	int x = arrow->getX() + arrow->getWidth() / 3;
	int y = arrow->getY() + arrow->getHeight();
	int width = arrowButtonDiameter;
	int height = arrowButtonDiameter;

	int type = arrowButton->getType();
	int xTilt = 0;
	switch (type)
	{
	case 1:
		xTilt = 0;
		break;
	case 2:
		xTilt = arrowButtonDiameter + 15;
		break;
	case 3:
		xTilt = 2 * arrowButtonDiameter + 2 * 15;
		break;
    default:
        break;
	}

	if (arrow->getOrientation() == Arrow::orientations::upLeft)
	{
		x = arrow->getX() + 10 + xTilt;
		y = arrow->getY() + 8;
	}
	else if (arrow->getOrientation() == Arrow::orientations::upRight)
	{
		x = arrow->getX() + 76 + xTilt;
        y = arrow->getY()  + 8;
	}
	else if (arrow->getOrientation() == Arrow::orientations::downLeft)
	{
		x = arrow->getX() + 10 + xTilt;
		y = arrow->getY() + arrowHeight - 8 - arrowButtonDiameter; //+ 75;
	}
	else if (arrow->getOrientation() == Arrow::orientations::downRight)
	{
		x = arrow->getX() + 76 + xTilt;
        y = arrow->getY() + arrowHeight - 8 - arrowButtonDiameter; //+ 75;
	}
	arrowButton->setBounds(x, y, width, height);
}

float XYPad::pixelToNormWidth(int pixelValue)
{
	return float(pixelValue) / float(getWidth());
}

float XYPad::pixelToNormHeight(int pixelValue)
{
	return float(getHeight() - pixelValue) / float(getHeight());
}

int XYPad::normToPixelWidth(float normValue)
{
	return juce::roundToInt(normValue * (float)getWidth());
}

int XYPad::normToPixelHeight(float normValue)
{
	return getHeight() - juce::roundToInt(normValue * (float)getHeight());
}

void XYPad::connectParameters(juce::RangedAudioParameter& x1, 
							  juce::RangedAudioParameter& y1, 
							  juce::RangedAudioParameter& x2, 
							  juce::RangedAudioParameter& y2)
{
	// knobs
	knob1XAttachment = std::make_unique<juce::ParameterAttachment>(x1, [this](float)
		{
			moveButton(&knob1, 1);
		});
	knob1YAttachment = std::make_unique<juce::ParameterAttachment>(y1, [this](float)
		{
			moveButton(&knob1, 1);
		});
	knob2XAttachment = std::make_unique<juce::ParameterAttachment>(x2, [this](float)
		{
			moveButton(&knob2, 2);
		});
	knob2YAttachment = std::make_unique<juce::ParameterAttachment>(y2, [this](float)
		{
			moveButton(&knob2, 2);
		});
}

void XYPad::setKnobButtonDiameter(int newDiameter, int buttonNumber) {
   juce::ignoreUnused(newDiameter);
   if (buttonNumber == 1){
       //knobButton1Diameter = newDiameter;
       /*
       if (shouldKnobBeMuted(buttonNumber))
           knob1.setVisible(false);
       else
           knob1.setVisible(true);
           */
   }
   else if (buttonNumber == 2){
       //knobButton2Diameter = newDiameter;
       /*

       if (shouldKnobBeMuted(2))
           knob2.setVisible(false);
       else
           knob2.setVisible(true);
           */

   }
   moveButton(&knob1, 1);
   moveButton(&knob2, 2);
}

int XYPad::calcCurrentButtonDiameters(int buttonNumber) {
    float fadeValue = parameters.getRawParameterValue(PluginParameters::FADE_ID.getParamID())->load();


    if (buttonNumber == 1)
        return int((float)knobButtonNeutralDiameter + (fadeValue * 100 / 2));
    else if (buttonNumber ==2)
        return int((float)knobButtonNeutralDiameter - (fadeValue * 100 / 2));
    else
        return knobButtonNeutralDiameter;

}



bool XYPad::shouldKnobBeMuted(int knobNumber) {
    float fadeValue = parameters.getRawParameterValue(PluginParameters::FADE_ID.getParamID())->load();


    //int buttonToBeMuted = 0;
    if ((knobNumber == 1 && fadeValue == -1.0) || (knobNumber == 2 && fadeValue == 1.0))
        return true;
    else
        return false;

}




