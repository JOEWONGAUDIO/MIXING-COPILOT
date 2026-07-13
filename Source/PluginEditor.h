#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MixingCopilotAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit MixingCopilotAudioProcessorEditor (MixingCopilotAudioProcessor&);
    ~MixingCopilotAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    struct ValueHitArea
    {
        juce::String key;
        double milliseconds = 0.0;
        juce::Rectangle<float> cardBounds;
        juce::Rectangle<float> valueBounds;
        juce::Rectangle<float> unitBounds;
    };

    void timerCallback() override;

    void drawLeftPanel (juce::Graphics&, juce::Rectangle<float> bounds);
    void drawParameterPanels (juce::Graphics&, juce::Rectangle<float> bounds);
    void drawCopyHint (juce::Graphics&, juce::Rectangle<float> bounds);
    void drawPanel (juce::Graphics&, juce::Rectangle<float> bounds,
                    const juce::String& title, const juce::String& translation = {});
    void drawValueCard (juce::Graphics&, juce::Rectangle<float> card,
                        const juce::String& label, const juce::String& translation,
                        const juce::String& key, double milliseconds);
    void drawCopyNotification (juce::Graphics&, juce::Rectangle<float> bounds);

    double getEffectiveBpm() const;
    juce::String getValueText (const juce::String& key, double milliseconds) const;
    juce::String getUnitText (const juce::String& key, double milliseconds) const;
    bool isShowingMilliseconds (const juce::String& key, double milliseconds) const;

    MixingCopilotAudioProcessor& audioProcessor;
    juce::Image logoImage;
    juce::Image metronomeImage;
    juce::Image pendulumImage;
    juce::Image cachedMetronomeImage;
    juce::Image cachedPendulumImage;
    juce::Rectangle<int> cachedMetronomeBounds;
    int cachedPendulumHeight = 0;
    juce::Array<ValueHitArea> hitAreas;
    juce::StringArray millisecondModeKeys;
    juce::String activeClickKey;
    double clickFeedbackUntilMs = 0.0;
    double copyNotificationUntilMs = 0.0;

    double swingPhase = 0.0;
    double lastFrameTimeSeconds = 0.0;
    double lastSeenBpm = -1.0;
    bool lastSeenPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixingCopilotAudioProcessorEditor)
};
