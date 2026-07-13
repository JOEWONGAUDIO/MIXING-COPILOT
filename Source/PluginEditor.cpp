#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
constexpr double defaultReferenceBpm = 120.0;
constexpr double visualSyncDelaySeconds = 0.035;

const juce::Colour ink        { 0xfff6efe2 };
const juce::Colour mutedInk   { 0xffbcae95 };
const juce::Colour panel      { 0xff171511 };
const juce::Colour panelLine  { 0xff423828 };
const juce::Colour brass      { 0xffd7a84c };
const juce::Colour red        { 0xffd85d4a };
const juce::Colour green      { 0xff6fbe8c };
const juce::Colour deep       { 0xff0e0d0b };
const juce::Colour cardFill   { 0xff201b14 };

juce::String zh (const char* text)
{
    return juce::String::fromUTF8 (text);
}

juce::FontOptions zhFont (float height, int style = juce::Font::plain)
{
    return { "PingFang SC", height, style };
}

juce::Rectangle<float> gridCell (juce::Rectangle<float> area, int columns, int rows, int index, float gap)
{
    const auto column = index % columns;
    const auto row = index / columns;
    const auto width = (area.getWidth() - gap * (float) (columns - 1)) / (float) columns;
    const auto height = (area.getHeight() - gap * (float) (rows - 1)) / (float) rows;

    return { area.getX() + (width + gap) * (float) column,
             area.getY() + (height + gap) * (float) row,
             width,
             height };
}

float textWidth (const juce::Font& font, const juce::String& text)
{
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText (font, text, 0.0f, 0.0f);
    return glyphs.getBoundingBox (0, glyphs.getNumGlyphs(), true).getWidth();
}

juce::Rectangle<float> fittedImageBounds (const juce::Image& image, juce::Rectangle<float> target)
{
    if (! image.isValid())
        return {};

    const auto source = image.getBounds().toFloat();
    const auto scale = juce::jmin (target.getWidth() / source.getWidth(),
                                   target.getHeight() / source.getHeight());

    return juce::Rectangle<float> (source.getWidth() * scale, source.getHeight() * scale)
        .withCentre (target.getCentre());
}
}

MixingCopilotAudioProcessorEditor::MixingCopilotAudioProcessorEditor (MixingCopilotAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      logoImage (juce::ImageFileFormat::loadFrom (BinaryData::mixing_copilot_logo_png,
                                                  BinaryData::mixing_copilot_logo_pngSize)),
      metronomeImage (juce::ImageFileFormat::loadFrom (BinaryData::vintage_metronome_png,
                                                       BinaryData::vintage_metronome_pngSize)),
      pendulumImage (juce::ImageFileFormat::loadFrom (BinaryData::metronome_pendulum_png,
                                                      BinaryData::metronome_pendulum_pngSize))
{
    setSize (920, 700);
    setBufferedToImage (false);
    lastFrameTimeSeconds = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    startTimerHz (60);
}

void MixingCopilotAudioProcessorEditor::resized() {}

void MixingCopilotAudioProcessorEditor::timerCallback()
{
    const auto timestampSeconds = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    const auto nowMs = timestampSeconds * 1000.0;

    const auto deltaSeconds = lastFrameTimeSeconds > 0.0
        ? juce::jlimit (0.0, 0.05, timestampSeconds - lastFrameTimeSeconds)
        : 0.0;

    lastFrameTimeSeconds = timestampSeconds;

    const auto bpm = audioProcessor.getCurrentBpm();
    const auto playing = audioProcessor.getIsPlaying();
    if (bpm > 0.0 && playing)
        swingPhase += deltaSeconds * (bpm / 120.0) * juce::MathConstants<double>::twoPi;

    const auto bpmChanged = std::abs (bpm - lastSeenBpm) > 0.01;
    const auto playingChanged = playing != lastSeenPlaying;
    const auto hasFeedbackAnimation = nowMs < clickFeedbackUntilMs || nowMs < copyNotificationUntilMs;

    lastSeenBpm = bpm;
    lastSeenPlaying = playing;

    if ((bpm > 0.0 && playing) || bpmChanged || playingChanged || hasFeedbackAnimation)
        repaint();
}

void MixingCopilotAudioProcessorEditor::paint (juce::Graphics& g)
{
    hitAreas.clearQuick();

    auto bounds = getLocalBounds().toFloat();
    g.fillAll (deep);

    juce::ColourGradient bg (juce::Colour (0xff252018), 0.0f, 0.0f,
                             juce::Colour (0xff0c0b09), bounds.getRight(), bounds.getBottom(), false);
    bg.addColour (0.55, juce::Colour (0xff16130f));
    g.setGradientFill (bg);
    g.fillRect (bounds);

    auto left = bounds.removeFromLeft (bounds.getWidth() * 0.32f).reduced (18.0f);
    auto right = bounds.reduced (14.0f, 18.0f);

    drawLeftPanel (g, left);
    drawParameterPanels (g, right);
}

void MixingCopilotAudioProcessorEditor::drawLeftPanel (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    drawPanel (g, bounds, {});

    auto inner = bounds.reduced (22.0f);
    auto logoArea = inner.removeFromTop (54.0f);
    if (logoImage.isValid())
    {
        const auto imageBounds = logoImage.getBounds().toFloat();
        const auto scale = juce::jmin (logoArea.getWidth() / imageBounds.getWidth(),
                                       logoArea.getHeight() / imageBounds.getHeight());
        auto fitted = juce::Rectangle<float> (imageBounds.getWidth() * scale,
                                              imageBounds.getHeight() * scale)
                          .withCentre (logoArea.getCentre());
        g.drawImage (logoImage, fitted);
    }
    else
    {
        g.setColour (brass);
        g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
        g.drawFittedText ("MIXING COPILOT", logoArea.toNearestInt(), juce::Justification::centred, 1);
    }

    g.setColour (mutedInk);
    g.setFont (juce::FontOptions (12.0f, juce::Font::plain));
    auto makerArea = inner.removeFromTop (16.0f).translated (0.0f, -4.0f);
    g.drawFittedText ("JOEWONGAUDIO", makerArea.toNearestInt(),
                      juce::Justification::centred, 1);

    inner.removeFromTop (8.0f);
    const auto rawBpm = audioProcessor.getCurrentBpm();
    const auto bpmText = rawBpm > 0.0 ? juce::String (rawBpm, 1) : "0";

    g.setColour (ink);
    g.setFont (juce::FontOptions (72.0f, juce::Font::bold));
    g.drawFittedText (bpmText, inner.removeFromTop (82.0f).toNearestInt(),
                      juce::Justification::centred, 1);

    g.setColour (mutedInk);
    g.setFont (juce::FontOptions (16.0f, juce::Font::plain));
    g.drawFittedText ("BPM FROM HOST", inner.removeFromTop (28.0f).toNearestInt(),
                      juce::Justification::centred, 1);

    auto status = inner.removeFromTop (30.0f);
    auto statusText = audioProcessor.getIsPlaying() ? "PLAYING" : "STOPPED";
    auto statusWidth = 104.0f;
    auto statusGroup = status.withSizeKeepingCentre (statusWidth, status.getHeight());
    g.setColour (audioProcessor.getIsPlaying() ? green : red);
    g.fillEllipse (statusGroup.getX(), statusGroup.getCentreY() - 5.0f, 10.0f, 10.0f);
    g.setColour (ink);
    g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    g.drawFittedText (statusText,
                      statusGroup.withTrimmedLeft (18.0f).toNearestInt(),
                      juce::Justification::centredLeft, 1);

    inner.removeFromTop (8.0f);
    auto notificationArea = inner.removeFromBottom (62.0f);
    auto meter = inner.reduced (6.0f, 2.0f);
    auto body = meter.withSizeKeepingCentre (juce::jmin (meter.getWidth(), 244.0f),
                                             juce::jmin (meter.getHeight(), 276.0f));
    if (metronomeImage.isValid())
    {
        const auto fitted = fittedImageBounds (metronomeImage, body).toNearestInt();
        if (! cachedMetronomeImage.isValid() || cachedMetronomeBounds != fitted)
        {
            cachedMetronomeBounds = fitted;
            cachedMetronomeImage = metronomeImage.rescaled (juce::jmax (1, fitted.getWidth()),
                                                            juce::jmax (1, fitted.getHeight()),
                                                            juce::Graphics::highResamplingQuality);
        }

        g.drawImageAt (cachedMetronomeImage, fitted.getX(), fitted.getY());
    }

    const auto centreX = body.getCentreX();
    const auto pivotY = body.getY() + body.getHeight() * 0.695f;
    const auto syncDelayRadians = rawBpm > 0.0
        ? visualSyncDelaySeconds * (rawBpm / 60.0) * juce::MathConstants<double>::twoPi
        : 0.0;
    const auto angle = rawBpm > 0.0 && audioProcessor.getIsPlaying()
        ? std::sin (swingPhase - syncDelayRadians) * 0.48
        : 0.0;

    if (pendulumImage.isValid())
    {
        const auto pivot = juce::Point<float> ((float) centreX, (float) pivotY);
        const auto sourceBounds = pendulumImage.getBounds().toFloat();
        const auto desiredHeight = body.getHeight() * 0.82f;
        const auto scale = desiredHeight / sourceBounds.getHeight();
        const auto scaledHeight = juce::jmax (1, (int) std::round (desiredHeight));
        const auto scaledWidth = juce::jmax (1, (int) std::round (sourceBounds.getWidth() * scale));

        if (! cachedPendulumImage.isValid() || cachedPendulumHeight != scaledHeight)
        {
            cachedPendulumHeight = scaledHeight;
            cachedPendulumImage = pendulumImage.rescaled (scaledWidth, scaledHeight,
                                                          juce::Graphics::highResamplingQuality);
        }

        const auto imagePivot = juce::Point<float> ((float) scaledWidth * 0.5f,
                                                    (float) scaledHeight * 0.765f);
        const auto pendulumTopLeft = juce::Point<float> (pivot.x - imagePivot.x,
                                                         pivot.y - imagePivot.y);
        const auto transform = juce::AffineTransform::rotation ((float) angle, pivot.x, pivot.y);
        const auto imageTransform = juce::AffineTransform::translation (pendulumTopLeft.x,
                                                                        pendulumTopLeft.y);

        g.drawImageTransformed (cachedPendulumImage, imageTransform.followedBy (transform),
                                false);
    }

    drawCopyNotification (g, notificationArea);
}

void MixingCopilotAudioProcessorEditor::drawParameterPanels (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto hintArea = bounds.removeFromBottom (22.0f);
    bounds.removeFromBottom (6.0f);

    const auto gap = 8.0f;
    const auto cardGap = 6.0f;
    const auto titleReserve = 20.0f;
    const auto verticalPad = 8.0f;
    const auto totalCardRows = 7.0f;
    const auto cardHeight = std::floor ((bounds.getHeight() - gap * 3.0f
                                         - titleReserve * 4.0f
                                         - verticalPad * 8.0f
                                         - cardGap * 3.0f) / totalCardRows);
    const auto panelHeightForRows = [=] (int rows)
    {
        return titleReserve + verticalPad * 2.0f + cardHeight * (float) rows + cardGap * (float) (rows - 1);
    };
    const auto contentAreaForRows = [=] (juce::Rectangle<float> panelBounds, int rows)
    {
        auto contentBounds = panelBounds.reduced (10.0f, verticalPad);
        contentBounds.removeFromTop (titleReserve);
        contentBounds.setHeight (cardHeight * (float) rows + cardGap * (float) (rows - 1));
        return contentBounds;
    };

    auto row = bounds.removeFromTop (panelHeightForRows (1));
    auto bpm = getEffectiveBpm();
    auto beat = 60000.0 / bpm;

    drawPanel (g, row, "PRE-DELAY", zh ("预延迟"));
    auto content = contentAreaForRows (row, 1);
    drawValueCard (g, gridCell (content, 3, 1, 0, cardGap), "SHORT", zh ("短"), "predelay_short", beat / 32.0);
    drawValueCard (g, gridCell (content, 3, 1, 1, cardGap), "MEDIUM", zh ("中"), "predelay_medium", beat / 16.0);
    drawValueCard (g, gridCell (content, 3, 1, 2, cardGap), "LONG", zh ("长"), "predelay_long", beat / 8.0);

    bounds.removeFromTop (gap);
    row = bounds.removeFromTop (panelHeightForRows (2));
    drawPanel (g, row, "REVERB TIME", zh ("混响时间"));
    content = contentAreaForRows (row, 2);
    drawValueCard (g, gridCell (content, 3, 2, 0, cardGap), "ROOM A", zh ("房间 A"), "reverb_room_a", beat * 0.5);
    drawValueCard (g, gridCell (content, 3, 2, 1, cardGap), "ROOM B", zh ("房间 B"), "reverb_room_b", beat * 1.0);
    drawValueCard (g, gridCell (content, 3, 2, 2, cardGap), "PLATE A", zh ("板式 A"), "reverb_plate_a", beat * 2.0);
    drawValueCard (g, gridCell (content, 3, 2, 3, cardGap), "PLATE B", zh ("板式 B"), "reverb_plate_b", beat * 4.0);
    drawValueCard (g, gridCell (content, 3, 2, 4, cardGap), "HALL A", zh ("大厅 A"), "reverb_hall_a", beat * 4.0);
    drawValueCard (g, gridCell (content, 3, 2, 5, cardGap), "HALL B", zh ("大厅 B"), "reverb_hall_b", beat * 8.0);

    bounds.removeFromTop (gap);
    row = bounds.removeFromTop (panelHeightForRows (2));
    drawPanel (g, row, "COMP RELEASE", zh ("压缩释放"));
    content = contentAreaForRows (row, 2);
    drawValueCard (g, gridCell (content, 3, 2, 0, cardGap), "FAST A", zh ("快速 A"), "release_fast_a", beat / 16.0);
    drawValueCard (g, gridCell (content, 3, 2, 1, cardGap), "FAST B", zh ("快速 B"), "release_fast_b", beat / 8.0);
    drawValueCard (g, gridCell (content, 3, 2, 2, cardGap), "MED A", zh ("中速 A"), "release_med_a", beat / 4.0);
    drawValueCard (g, gridCell (content, 3, 2, 3, cardGap), "MED B", zh ("中速 B"), "release_med_b", beat / 2.0);
    drawValueCard (g, gridCell (content, 3, 2, 4, cardGap), "SLOW A", zh ("慢速 A"), "release_slow_a", beat * 1.0);
    drawValueCard (g, gridCell (content, 3, 2, 5, cardGap), "SLOW B", zh ("慢速 B"), "release_slow_b", beat * 2.0);

    bounds.removeFromTop (gap);
    row = bounds.removeFromTop (panelHeightForRows (2));
    drawPanel (g, row, "DELAY TIME", zh ("延迟时间"));
    content = contentAreaForRows (row, 2);
    drawValueCard (g, gridCell (content, 3, 2, 0, cardGap), "1/2", {}, "delay_1_2", beat * 2.0);
    drawValueCard (g, gridCell (content, 3, 2, 1, cardGap), "1/4", {}, "delay_1_4", beat * 1.0);
    drawValueCard (g, gridCell (content, 3, 2, 2, cardGap), "1/8", {}, "delay_1_8", beat / 2.0);
    drawValueCard (g, gridCell (content, 3, 2, 3, cardGap), "1/16", {}, "delay_1_16", beat / 4.0);
    drawValueCard (g, gridCell (content, 3, 2, 4, cardGap), "1/32", {}, "delay_1_32", beat / 8.0);
    drawValueCard (g, gridCell (content, 3, 2, 5, cardGap), "1/64", {}, "delay_1_64", beat / 16.0);

    drawCopyHint (g, hintArea);
}

void MixingCopilotAudioProcessorEditor::drawPanel (juce::Graphics& g,
                                                   juce::Rectangle<float> bounds,
                                                   const juce::String& title,
                                                   const juce::String& translation)
{
    g.setColour (panel.withAlpha (0.96f));
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (panelLine);
    g.drawRoundedRectangle (bounds, 8.0f, 1.2f);

    if (title.isNotEmpty())
    {
        const auto titleFont = juce::Font (juce::FontOptions (15.0f, juce::Font::bold));
        auto titleArea = bounds.withTrimmedLeft (10.0f)
                               .withTrimmedRight (10.0f)
                               .withY (bounds.getY() + 7.0f)
                               .withHeight (20.0f);
        const auto titleWidth = textWidth (titleFont, title) + 5.0f;
        g.setColour (brass);
        g.setFont (titleFont);
        g.drawFittedText (title, titleArea.removeFromLeft (titleWidth).toNearestInt(),
                          juce::Justification::centredLeft, 1);
        g.setColour (ink.withAlpha (0.34f));
        g.setFont (zhFont (15.0f, juce::Font::bold));
        g.drawFittedText (translation, titleArea.toNearestInt(),
                          juce::Justification::centredLeft, 1);
    }
}

void MixingCopilotAudioProcessorEditor::drawValueCard (juce::Graphics& g,
                                                       juce::Rectangle<float> card,
                                                       const juce::String& label,
                                                       const juce::String& translation,
                                                       const juce::String& key,
                                                       double milliseconds)
{
    const auto now = juce::Time::getMillisecondCounterHiRes();
    const auto active = activeClickKey == key && now < clickFeedbackUntilMs;
    const auto cardBounds = active ? card.reduced (1.0f) : card;
    auto inner = cardBounds.reduced (8.0f, 5.0f);

    g.setColour (active ? brass.withAlpha (0.30f) : cardFill);
    g.fillRoundedRectangle (cardBounds, 6.0f);
    g.setColour (active ? brass : panelLine.withAlpha (0.8f));
    g.drawRoundedRectangle (cardBounds, 6.0f, active ? 1.8f : 1.0f);

    auto labelArea = inner.removeFromTop (16.0f);
    const auto labelFont = juce::Font (juce::FontOptions (13.0f, juce::Font::bold));
    const auto labelWidth = translation.isNotEmpty() ? textWidth (labelFont, label) + 5.0f
                                                     : labelArea.getWidth();
    g.setColour (mutedInk);
    g.setFont (labelFont);
    g.drawFittedText (label, labelArea.removeFromLeft (labelWidth).toNearestInt(),
                      juce::Justification::centredLeft, 1);
    if (translation.isNotEmpty())
    {
        g.setColour (ink.withAlpha (0.32f));
        g.setFont (zhFont (13.0f, juce::Font::bold));
        g.drawFittedText (translation, labelArea.toNearestInt(), juce::Justification::centredLeft, 1);
    }

    inner.removeFromTop (1.0f);
    auto metricArea = inner;
    const auto canToggle = milliseconds >= 1000.0;
    auto unitColumn = metricArea.removeFromRight (canToggle ? 64.0f : 34.0f);
    auto unitArea = unitColumn.withSizeKeepingCentre (unitColumn.getWidth(),
                                                      juce::jmin (24.0f, unitColumn.getHeight()));
    const auto fullUnitArea = canToggle ? unitArea : juce::Rectangle<float>();
    auto valueArea = metricArea.withTrimmedRight (8.0f);

    g.setColour (ink);
    g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
    g.drawFittedText (getValueText (key, milliseconds), valueArea.toNearestInt(),
                      juce::Justification::centredRight, 1);

    const auto showingMs = isShowingMilliseconds (key, milliseconds);
    if (canToggle)
    {
        g.setColour (juce::Colour (0xff0c0a08).withAlpha (0.88f));
        g.fillRoundedRectangle (unitArea, 4.0f);
        g.setColour (panelLine.withAlpha (0.58f));
        g.drawRoundedRectangle (unitArea, 4.0f, 1.0f);

        auto secondsArea = unitArea.removeFromLeft (unitArea.getWidth() * 0.5f);
        auto msArea = unitArea;
        const auto secondsActive = ! showingMs;
        auto activeArea = (secondsActive ? secondsArea : msArea).reduced (2.0f, 2.0f);

        juce::ColourGradient activeFill (brass.withAlpha (0.42f), activeArea.getX(), activeArea.getY(),
                                         brass.withAlpha (0.18f), activeArea.getRight(), activeArea.getBottom(), false);
        g.setGradientFill (activeFill);
        g.fillRoundedRectangle (activeArea, 3.0f);

        g.setColour (panelLine.withAlpha (0.45f));
        const auto separatorX = msArea.getX();
        g.drawVerticalLine ((int) std::round (separatorX), unitArea.getY() + 4.0f, unitArea.getBottom() - 4.0f);

        g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
        g.setColour (secondsActive ? brass : mutedInk.withAlpha (0.64f));
        g.drawFittedText ("s", secondsArea.reduced (1.0f, 0.0f).toNearestInt(), juce::Justification::centred, 1);
        g.setColour (! secondsActive ? brass : mutedInk.withAlpha (0.64f));
        g.drawFittedText ("ms", msArea.reduced (1.0f, 0.0f).toNearestInt(), juce::Justification::centred, 1);
    }
    else
    {
        g.setColour (mutedInk.withAlpha (0.72f));
        g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
        g.drawFittedText ("ms", unitArea.toNearestInt(), juce::Justification::centred, 1);
    }

    hitAreas.add ({ key, milliseconds, cardBounds, valueArea, fullUnitArea });
}

void MixingCopilotAudioProcessorEditor::drawCopyHint (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto hint = bounds.reduced (4.0f, 0.0f);
    auto hintGroup = hint.withSizeKeepingCentre (360.0f, hint.getHeight());

    g.setColour (mutedInk.withAlpha (0.55f));
    g.setFont (juce::FontOptions (13.0f, juce::Font::plain));
    g.drawFittedText ("Click value to copy parameter",
                      hintGroup.removeFromLeft (190.0f).toNearestInt(),
                      juce::Justification::centredRight, 1);

    g.setColour (ink.withAlpha (0.32f));
    g.setFont (zhFont (13.0f));
    g.drawFittedText (zh ("点击数值以复制参数"),
                      hintGroup.toNearestInt(),
                      juce::Justification::centredLeft, 1);
}

double MixingCopilotAudioProcessorEditor::getEffectiveBpm() const
{
    const auto hostBpm = audioProcessor.getCurrentBpm();
    return hostBpm > 0.0 ? hostBpm : defaultReferenceBpm;
}

juce::String MixingCopilotAudioProcessorEditor::getValueText (const juce::String& key,
                                                              double milliseconds) const
{
    if (! isShowingMilliseconds (key, milliseconds))
        return juce::String (milliseconds / 1000.0, 2);

    return juce::String (milliseconds, 2);
}

juce::String MixingCopilotAudioProcessorEditor::getUnitText (const juce::String& key,
                                                             double milliseconds) const
{
    if (! isShowingMilliseconds (key, milliseconds))
        return "s";

    return "ms";
}

bool MixingCopilotAudioProcessorEditor::isShowingMilliseconds (const juce::String& key,
                                                               double milliseconds) const
{
    return milliseconds < 1000.0 || millisecondModeKeys.contains (key);
}

void MixingCopilotAudioProcessorEditor::drawCopyNotification (juce::Graphics& g,
                                                              juce::Rectangle<float> bounds)
{
    const auto now = juce::Time::getMillisecondCounterHiRes();
    if (now >= copyNotificationUntilMs)
        return;

    const auto remaining = (float) juce::jlimit (0.0, 1.0, (copyNotificationUntilMs - now) / 700.0);
    auto textArea = bounds.withSizeKeepingCentre (bounds.getWidth(), 54.0f);

    g.setColour (ink.withAlpha (remaining));
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawFittedText ("COPIED", textArea.removeFromTop (30.0f).toNearestInt(),
                      juce::Justification::centred, 1);

    g.setColour (green.withAlpha (0.82f * remaining));
    g.setFont (zhFont (18.0f, juce::Font::bold));
    g.drawFittedText (zh ("复制成功"), textArea.removeFromTop (22.0f).toNearestInt(),
                      juce::Justification::centred, 1);
}

void MixingCopilotAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    const auto pos = e.position;
    const auto now = juce::Time::getMillisecondCounterHiRes();

    for (const auto& area : hitAreas)
    {
        if (area.unitBounds.contains (pos) && area.milliseconds >= 1000.0)
        {
            if (millisecondModeKeys.contains (area.key))
                millisecondModeKeys.removeString (area.key);
            else
                millisecondModeKeys.add (area.key);

            repaint();
            return;
        }

        if (area.cardBounds.contains (pos))
        {
            juce::SystemClipboard::copyTextToClipboard (getValueText (area.key, area.milliseconds));
            activeClickKey = area.key;
            clickFeedbackUntilMs = now + 160.0;
            copyNotificationUntilMs = now + 900.0;
            repaint();
            return;
        }
    }
}
