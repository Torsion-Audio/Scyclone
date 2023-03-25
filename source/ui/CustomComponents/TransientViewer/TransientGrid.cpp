//
// Created by valentin.ackva on 16.03.2023.
//

#include "TransientGrid.h"

TransientGrid::TransientGrid() = default;

void TransientGrid::paint(juce::Graphics &g) {
    g.setColour(juce::Colour::fromString(ColorPallete::OCTAGON));

    // draw rounded rectangle
    float cornerWidth = 10.0f;
    float lineWidth = 3.0f;
    auto width = static_cast<float>(getWidth()) - 2*lineWidth;
    auto height = static_cast<float>(getHeight()) - 2*lineWidth;
    g.drawRoundedRectangle(lineWidth, lineWidth, width - lineWidth, height - lineWidth, cornerWidth, 3.0f);

    // horizontal lines
    g.drawLine(lineWidth+1, height / 4.f, width-1, height / 4.f, lineWidth);
    g.drawLine(lineWidth+1, 2*height / 4.f, width-1, 2*height / 4.f, lineWidth);
    g.drawLine(lineWidth+1, 3*height / 4.f, width-1, 3*height / 4.f, lineWidth);

    // vertical lines
    g.drawLine(width / 4.f, lineWidth+1.5f, width / 4.f, height/4-1.f, lineWidth);
    g.drawLine(width / 4.f, height/4+1.f, width / 4.f, 2*height/4-1, lineWidth);
    g.drawLine(width / 4.f, 2*height/4+1.f, width / 4.f, 3*height/4-1, lineWidth);
    g.drawLine(width / 4.f, 3*height/4+1.f, width / 4.f, height-1.5f, lineWidth);

    g.drawLine(2.f*width / 4.f, lineWidth + 1.5f, 2*width / 4.f, height/4-1.f, lineWidth);
    g.drawLine(2.f*width / 4.f, height/4 + 1.f, 2*width / 4.f, 2*height/4-1.f, lineWidth);
    g.drawLine(2.f*width / 4.f, 2*height/4 + 1.f, 2*width / 4.f, 3*height/4-1.f, lineWidth);
    g.drawLine(2.f*width / 4.f, 3*height/4 + 1.f, 2*width / 4.f, height-1.5f, lineWidth);

    g.drawLine(3.f*width / 4.f, lineWidth + 1.5f, 3.f*width / 4.f, height/4.f - 1.f, lineWidth);
    g.drawLine(3.f*width / 4.f, height/4.f + 1.f, 3.f*width / 4.f, 2*height/4.f - 1.f, lineWidth);
    g.drawLine(3.f*width / 4.f, 2*height/4.f + 1.f, 3.f*width / 4.f, 3*height/4.f - 1.f, lineWidth);
    g.drawLine(3.f*width / 4.f, 3*height/4 + 1.f, 3.f*width / 4.f, height-1.5f, lineWidth);
}
