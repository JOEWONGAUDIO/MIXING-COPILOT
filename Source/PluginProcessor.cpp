#include "PluginProcessor.h"
#include "PluginEditor.h"

MixingCopilotAudioProcessor::MixingCopilotAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

void MixingCopilotAudioProcessor::prepareToPlay (double, int) {}

void MixingCopilotAudioProcessor::releaseResources() {}

bool MixingCopilotAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainIn = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();

    return mainIn == mainOut
        && (mainOut == juce::AudioChannelSet::mono()
            || mainOut == juce::AudioChannelSet::stereo());
}

template <typename SampleType>
void MixingCopilotAudioProcessor::updateTransportAndClear (juce::AudioBuffer<SampleType>& buffer)
{
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            const auto hostBpm = position->getBpm();
            currentBpm.store (hostBpm && *hostBpm > 0.0 ? *hostBpm : 0.0,
                              std::memory_order_relaxed);

            isPlaying.store (position->getIsPlaying(), std::memory_order_relaxed);
        }
        else
        {
            currentBpm.store (0.0, std::memory_order_relaxed);
            isPlaying.store (false, std::memory_order_relaxed);
        }
    }
    else
    {
        currentBpm.store (0.0, std::memory_order_relaxed);
        isPlaying.store (false, std::memory_order_relaxed);
    }

    buffer.clear();
}

void MixingCopilotAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer&)
{
    updateTransportAndClear (buffer);
}

void MixingCopilotAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer,
                                                juce::MidiBuffer&)
{
    updateTransportAndClear (buffer);
}

juce::AudioProcessorEditor* MixingCopilotAudioProcessor::createEditor()
{
    return new MixingCopilotAudioProcessorEditor (*this);
}

bool MixingCopilotAudioProcessor::hasEditor() const { return true; }

const juce::String MixingCopilotAudioProcessor::getName() const { return "MIXING COPILOT"; }
bool MixingCopilotAudioProcessor::acceptsMidi() const { return false; }
bool MixingCopilotAudioProcessor::producesMidi() const { return false; }
bool MixingCopilotAudioProcessor::isMidiEffect() const { return false; }
double MixingCopilotAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int MixingCopilotAudioProcessor::getNumPrograms() { return 1; }
int MixingCopilotAudioProcessor::getCurrentProgram() { return 0; }
void MixingCopilotAudioProcessor::setCurrentProgram (int) {}
const juce::String MixingCopilotAudioProcessor::getProgramName (int) { return {}; }
void MixingCopilotAudioProcessor::changeProgramName (int, const juce::String&) {}

void MixingCopilotAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void MixingCopilotAudioProcessor::setStateInformation (const void*, int) {}

double MixingCopilotAudioProcessor::getCurrentBpm() const noexcept
{
    return currentBpm.load (std::memory_order_relaxed);
}

bool MixingCopilotAudioProcessor::getIsPlaying() const noexcept
{
    return isPlaying.load (std::memory_order_relaxed);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MixingCopilotAudioProcessor();
}
