#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

class HarbingerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HarbingerLookAndFeel()
    {
        // Theme: Blood Red, Industrial Slate, Deep Obsidian, White
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff08090b));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff121419));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3a1014));
        setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffff1e2e)); // Blood Red
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff101216));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff800a14)); // Dark Blood Red
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1c2028));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::TextButton::textColourOnId, juce::Colour(0xffff1e2e));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xee0a0c10));
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x55ff1e2e));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        juce::ignoreUnused(slider);

        const float radius = static_cast<float>(std::min(width, height)) * 0.5f - 5.0f;
        const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Outer chassis shadow and recessed well
        g.setColour(juce::Colour(0xff040507));
        g.fillEllipse(centreX - radius - 3.0f, centreY - radius - 2.0f, (radius + 3.0f) * 2.0f, (radius + 3.0f) * 2.0f);

        // Track background groove
        juce::Path trackPath;
        trackPath.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff161920));
        g.strokePath(trackPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Active Glowing Blood Red Arc
        if (sliderPosProportional > 0.001f)
        {
            juce::Path activeArc;
            activeArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, angle, true);
            // Outer soft glow
            g.setColour(juce::Colour(0x55ff1e2e));
            g.strokePath(activeArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            // Inner crisp laser arc
            g.setColour(juce::Colour(0xffff2233));
            g.strokePath(activeArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Inner Machined Knurled Knob Body
        const float knobRadius = radius - 9.0f;
        juce::ColourGradient knobGrad(juce::Colour(0xff323640), centreX, centreY - knobRadius,
                                      juce::Colour(0xff0d0f13), centreX, centreY + knobRadius, false);
        g.setGradientFill(knobGrad);
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        // Machined bezel edge
        g.setColour(juce::Colour(0xff555d6e));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

        // Perimeter grip knurls
        const int numGrips = 18;
        g.setColour(juce::Colour(0xff08090c));
        for (int i = 0; i < numGrips; ++i)
        {
            const float a = static_cast<float>(i) * (juce::MathConstants<float>::twoPi / static_cast<float>(numGrips));
            const float gx = centreX + (knobRadius - 2.8f) * std::sin(a);
            const float gy = centreY - (knobRadius - 2.8f) * std::cos(a);
            g.fillEllipse(gx - 1.2f, gy - 1.2f, 2.4f, 2.4f);
        }

        // Center Cap (Matte Gunmetal)
        const float capRadius = knobRadius * 0.65f;
        juce::ColourGradient capGrad(juce::Colour(0xff22252c), centreX, centreY - capRadius,
                                     juce::Colour(0xff121418), centreX, centreY + capRadius, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);
        g.setColour(juce::Colour(0xff444b58));
        g.drawEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f, 0.8f);

        // Pointer Line (Glowing blood-red needle)
        juce::Path pointer;
        const float pointerLength = knobRadius - 3.0f;
        pointer.startNewSubPath(centreX + (capRadius * 0.4f) * std::sin(angle),
                                centreY - (capRadius * 0.4f) * std::cos(angle));
        pointer.lineTo(centreX + pointerLength * std::sin(angle),
                       centreY - pointerLength * std::cos(angle));

        g.setColour(juce::Colour(0x88ff1e2e));
        g.strokePath(pointer, juce::PathStrokeType(4.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        g.setColour(juce::Colour(0xffff2233));
        g.strokePath(pointer, juce::PathStrokeType(2.2f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        g.setColour(juce::Colours::white);
        g.strokePath(pointer, juce::PathStrokeType(1.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

        juce::Colour base = backgroundColour;
        if (shouldDrawButtonAsDown)
            base = base.darker(0.35f);
        else if (shouldDrawButtonAsHighlighted)
            base = base.brighter(0.2f);

        juce::ColourGradient grad(base.brighter(0.12f), 0, bounds.getY(),
                                  base.darker(0.25f), 0, bounds.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Blood Red highlight on hover
        g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(0xffff1e2e) : juce::Colour(0xff343a47));
        g.drawRoundedRectangle(bounds, 4.0f, 1.2f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH, box);

        auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f);

        juce::ColourGradient grad(juce::Colour(0xff181a22), 0, 0,
                                  juce::Colour(0xff0c0d11), 0, static_cast<float>(height), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(juce::Colour(0xff441217));
        g.drawRoundedRectangle(bounds, 4.0f, 1.2f);

        // Blood Red Arrow
        const float arrowX = static_cast<float>(width) - 16.0f;
        const float arrowY = static_cast<float>(height) * 0.5f;

        juce::Path arrow;
        arrow.startNewSubPath(arrowX - 5.0f, arrowY - 3.0f);
        arrow.lineTo(arrowX, arrowY + 3.0f);
        arrow.lineTo(arrowX + 5.0f, arrowY - 3.0f);

        g.setColour(juce::Colour(0xffff1e2e));
        g.strokePath(arrow, juce::PathStrokeType(2.0f));
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        auto bounds = button.getLocalBounds().toFloat();
        const float toggleW = 34.0f;
        const float toggleH = 18.0f;
        const float toggleY = bounds.getCentreY() - (toggleH * 0.5f);
        const float toggleX = bounds.getX() + 2.0f;

        const bool isOn = button.getToggleState();

        // Switch Slot
        auto slotRect = juce::Rectangle<float>(toggleX, toggleY, toggleW, toggleH);
        g.setColour(juce::Colour(0xff08090c));
        g.fillRoundedRectangle(slotRect, toggleH * 0.5f);
        g.setColour(juce::Colour(0xff351014));
        g.drawRoundedRectangle(slotRect, toggleH * 0.5f, 1.2f);

        // Thumb
        const float thumbDiameter = toggleH - 4.0f;
        const float thumbX = isOn ? (toggleX + toggleW - thumbDiameter - 2.0f) : (toggleX + 2.0f);
        const float thumbY = toggleY + 2.0f;

        if (isOn)
        {
            g.setColour(juce::Colour(0x55ff1e2e));
            g.fillEllipse(thumbX - 3.0f, thumbY - 3.0f, thumbDiameter + 6.0f, thumbDiameter + 6.0f);
        }

        juce::ColourGradient thumbGrad(isOn ? juce::Colour(0xffff2233) : juce::Colour(0xff687080),
                                       thumbX, thumbY,
                                       isOn ? juce::Colour(0xff8a0a14) : juce::Colour(0xff242730),
                                       thumbX, thumbY + thumbDiameter, false);
        g.setGradientFill(thumbGrad);
        g.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);

        g.setColour(isOn ? juce::Colour(0xffffaaaa) : juce::Colour(0xff9ea8ba));
        g.drawEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter, 1.0f);

        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(isOn ? juce::Colours::white : juce::Colour(0xff8c94a4));
        g.drawText(button.getButtonText(),
                   juce::Rectangle<float>(toggleX + toggleW + 8.0f, bounds.getY(),
                                          bounds.getWidth() - toggleW - 10.0f, bounds.getHeight()),
                   juce::Justification::centredLeft);
    }
};

// Custom Rotary Slider supporting right-click for MIDI Learn and double-click reset
class BrutalKnob : public juce::Slider
{
public:
    explicit BrutalKnob(double defaultVal = 0.5)
        : defaultValue(defaultVal)
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0b0d11));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff3a1418));
    }

    void mouseDoubleClick(const juce::MouseEvent& e) override
    {
        juce::ignoreUnused(e);
        setValue(defaultValue, juce::sendNotification);
    }

    void setDefaultResetValue(double val) { defaultValue = val; }

    std::function<void(const juce::MouseEvent&)> onRightClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }
        juce::Slider::mouseDown(e);
    }

private:
    double defaultValue = 0.5;
};

// Power / Enable Toggle Button with right-click MIDI Learn support
class BrutalPowerButton : public juce::TextButton
{
public:
    explicit BrutalPowerButton(const juce::String& text = "⏻ ON")
        : juce::TextButton(text)
    {
    }

    std::function<void(const juce::MouseEvent&)> onRightClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }
        juce::TextButton::mouseDown(e);
    }
};

// Slide Toggle Switch with right-click MIDI Learn support
class BrutalSlideToggle : public juce::ToggleButton
{
public:
    explicit BrutalSlideToggle(const juce::String& text = "")
        : juce::ToggleButton(text)
    {
    }

    std::function<void(const juce::MouseEvent&)> onRightClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }
        juce::ToggleButton::mouseDown(e);
    }
};

// 3-Way Mechanical Toggle Selector Component
class ThreeWaySwitch : public juce::Component
{
public:
    ThreeWaySwitch(const juce::String& option1, const juce::String& option2, const juce::String& option3)
        : opt1(option1), opt2(option2), opt3(option3)
    {
    }

    int getSelectedIndex() const { return selectedIndex; }

    void setSelectedIndex(int idx, juce::NotificationType notify = juce::sendNotification)
    {
        idx = juce::jlimit(0, 2, idx);
        if (selectedIndex != idx)
        {
            selectedIndex = idx;
            repaint();
            if (notify == juce::sendNotification && onChange)
                onChange(selectedIndex);
        }
    }

    std::function<void(int newIndex)> onChange;
    std::function<void(const juce::MouseEvent&)> onRightClick;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Label above or text options
        const float switchH = 22.0f;
        const float switchY = bounds.getCentreY() - switchH * 0.5f;
        const float switchW = bounds.getWidth() - 4.0f;
        const float switchX = bounds.getX() + 2.0f;

        // Slot plate
        auto slotRect = juce::Rectangle<float>(switchX, switchY, switchW, switchH);
        g.setColour(juce::Colour(0xff090a0d));
        g.fillRoundedRectangle(slotRect, 4.0f);
        g.setColour(juce::Colour(0xff3a1418));
        g.drawRoundedRectangle(slotRect, 4.0f, 1.2f);

        // 3 Segments
        const float segW = switchW / 3.0f;
        const juce::String labels[3] = { opt1, opt2, opt3 };

        for (int i = 0; i < 3; ++i)
        {
            auto segRect = juce::Rectangle<float>(switchX + static_cast<float>(i) * segW, switchY, segW, switchH);
            const bool isSelected = (i == selectedIndex);

            if (isSelected)
            {
                // Glowing blood-red active segment
                juce::ColourGradient selGrad(juce::Colour(0xffff2233), 0, segRect.getY(),
                                             juce::Colour(0xff8a0a14), 0, segRect.getBottom(), false);
                g.setGradientFill(selGrad);
                g.fillRoundedRectangle(segRect.reduced(1.5f), 3.0f);
                g.setColour(juce::Colour(0xffffaaaa));
                g.drawRoundedRectangle(segRect.reduced(1.5f), 3.0f, 1.0f);
            }

            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.setColour(isSelected ? juce::Colours::white : juce::Colour(0xff8a909d));
            g.drawText(labels[i], segRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }

        const float switchW = getWidth() - 4.0f;
        const float segW = switchW / 3.0f;
        const float clickX = static_cast<float>(e.x) - 2.0f;
        int clickedIdx = juce::jlimit(0, 2, static_cast<int>(clickX / segW));
        setSelectedIndex(clickedIdx);
    }

private:
    juce::String opt1, opt2, opt3;
    int selectedIndex = 0;
};

// Heavy-Duty Footswitch Stomp Component with dynamic LED indicator
class FootswitchComponent : public juce::Component
{
public:
    FootswitchComponent(const juce::String& labelText)
        : name(labelText)
    {
    }

    void setLedActive(bool active)
    {
        if (isLedOn != active)
        {
            isLedOn = active;
            repaint();
        }
    }

    void setLedBrightness(float brightness)
    {
        ledBrightness = juce::jlimit(0.0f, 1.0f, brightness);
        repaint();
    }

    std::function<void(bool isDown)> onPointerState;
    std::function<void()> onClick;
    std::function<void(const juce::MouseEvent&)> onRightClick;

    void paint(juce::Graphics& g) override
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f - 8.0f;
        const float r = 32.0f;

        // Outer chassis shadow
        g.setColour(juce::Colour(0xff030406));
        g.fillEllipse(cx - r - 4.0f, cy - r - 2.0f, (r + 4.0f) * 2.0f, (r + 4.0f) * 2.0f);

        // Heavy-duty knurled stainless steel hex nut / bezel
        const int numSides = 6;
        juce::Path hexPath;
        const float hexRadius = r + 2.0f;
        for (int i = 0; i < numSides; ++i)
        {
            const float angle = static_cast<float>(i) * (juce::MathConstants<float>::twoPi / 6.0f);
            const float px = cx + hexRadius * std::cos(angle);
            const float py = cy + hexRadius * std::sin(angle);
            if (i == 0) hexPath.startNewSubPath(px, py);
            else        hexPath.lineTo(px, py);
        }
        hexPath.closeSubPath();

        juce::ColourGradient nutGrad(juce::Colour(0xff6e7687), cx - hexRadius, cy - hexRadius,
                                     juce::Colour(0xff22252c), cx + hexRadius, cy + hexRadius, false);
        g.setGradientFill(nutGrad);
        g.fillPath(hexPath);
        g.setColour(juce::Colour(0xff8a94a6));
        g.strokePath(hexPath, juce::PathStrokeType(1.5f));

        // Circular actuator plunger
        const float plungerR = isPressed ? (r - 7.0f) : (r - 5.0f);
        juce::ColourGradient plungerGrad(isPressed ? juce::Colour(0xff22252c) : juce::Colour(0xff4a5260), cx, cy - plungerR,
                                         isPressed ? juce::Colour(0xff121418) : juce::Colour(0xff282d36), cx, cy + plungerR, false);
        g.setGradientFill(plungerGrad);
        g.fillEllipse(cx - plungerR, cy - plungerR, plungerR * 2.0f, plungerR * 2.0f);

        g.setColour(juce::Colour(0xff758092));
        g.drawEllipse(cx - plungerR, cy - plungerR, plungerR * 2.0f, plungerR * 2.0f, 1.5f);

        // Center machined dimple
        g.setColour(juce::Colour(0xff14161b));
        g.fillEllipse(cx - 8.0f, cy - 8.0f, 16.0f, 16.0f);

        // LED Jewel Indicator above switch
        const float ledY = cy - r - 16.0f;
        const float ledR = 7.0f;

        // LED Bezel
        g.setColour(juce::Colour(0xff1f232b));
        g.fillEllipse(cx - ledR - 2.0f, ledY - ledR - 2.0f, (ledR + 2.0f) * 2.0f, (ledR + 2.0f) * 2.0f);
        g.setColour(juce::Colour(0xff4d5464));
        g.drawEllipse(cx - ledR - 2.0f, ledY - ledR - 2.0f, (ledR + 2.0f) * 2.0f, (ledR + 2.0f) * 2.0f, 1.0f);

        const float intensity = isLedOn ? std::max(0.7f, ledBrightness) : ledBrightness;

        if (intensity > 0.05f)
        {
            // Outer radiant aura
            g.setColour(juce::Colour(0x55ff1e2e).withMultipliedAlpha(intensity));
            g.fillEllipse(cx - ledR - 6.0f, ledY - ledR - 6.0f, (ledR + 6.0f) * 2.0f, (ledR + 6.0f) * 2.0f);

            // Glowing lens
            juce::ColourGradient ledGrad(juce::Colour(0xffff6677), cx - 2.0f, ledY - 2.0f,
                                         juce::Colour(0xffcc1122), cx + ledR, ledY + ledR, true);
            g.setGradientFill(ledGrad);
            g.fillEllipse(cx - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);

            // Specular reflection glint
            g.setColour(juce::Colours::white.withAlpha(0.85f * intensity));
            g.fillEllipse(cx - 3.0f, ledY - 4.0f, 4.0f, 3.0f);
        }
        else
        {
            // Inactive unlit dark jewel
            g.setColour(juce::Colour(0xff33060a));
            g.fillEllipse(cx - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);
        }

        // Label below footswitch with dark pill backing
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        const int textW = g.getCurrentFont().getStringWidth(name) + 24;
        const auto pill = juce::Rectangle<float>((getWidth() - textW) * 0.5f, getHeight() - 24.0f, static_cast<float>(textW), 20.0f);
        g.setColour(juce::Colour(0xd008090d));
        g.fillRoundedRectangle(pill, 4.0f);
        g.setColour(juce::Colour(0x66ff1e2e));
        g.drawRoundedRectangle(pill, 4.0f, 1.0f);
        g.setColour(juce::Colours::white);
        g.drawText(name, pill, juce::Justification::centred);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }

        isPressed = true;
        repaint();
        if (onPointerState)
            onPointerState(true);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        juce::ignoreUnused(e);
        if (isPressed)
        {
            isPressed = false;
            repaint();
            if (onPointerState)
                onPointerState(false);
            if (onClick)
                onClick();
        }
    }

private:
    juce::String name;
    bool isPressed = false;
    bool isLedOn = false;
    float ledBrightness = 0.0f;
};
