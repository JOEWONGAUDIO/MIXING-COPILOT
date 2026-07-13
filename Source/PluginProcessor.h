#pragma once

#include <JuceHeader.h>
#include <atomic>

class MixingCopilotAudioProcessor final : public juce::AudioProcessor
{
public:
    MixingCopilotAudioProcessor();
    ~MixingCopilotAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    double getCurrentBpm() const noexcept;
    bool getIsPlaying() const noexcept;

private:
    template <typename SampleType>
    void updateTransportAndClear (juce::AudioBuffer<SampleType>& buffer);

    std::atomic<double> currentBpm { 0.0 };
    std::atomic<bool> isPlaying { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixingCopilotAudioProcessor)
};
