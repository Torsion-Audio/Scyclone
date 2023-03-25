#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : juce::AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       parameters (*this, nullptr, juce::Identifier ("VAESynth"), PluginParameters::createParameterLayout())
{

    network1Name = "Funk";
    network2Name = "Djembe";

    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.addParameterListener(parameterID, this);
    }
    processorTransientSplitter1 = std::make_unique<ProcessorTransientSplitter>(parameters, 1);
    processorTransientSplitter2 = std::make_unique<ProcessorTransientSplitter>(parameters, 2);

    iirCutoffFilter1 = std::make_unique<IIRCutoffFilter>(parameters, 1);
    iirCutoffFilter2 = std::make_unique<IIRCutoffFilter>(parameters, 2);

    onnxProcessor1 = std::make_unique<OnnxProcessor>(parameters, 1, FunkDrum);
    onnxProcessor2 = std::make_unique<OnnxProcessor>(parameters, 2, Djembe);

    grainDelay1 = std::make_unique<GrainDelay>(1);
    grainDelay2 = std::make_unique<GrainDelay>(2);

    processorCompressor = std::make_unique<ProcessorCompressor>(parameters);

    parameters.state.addChild(PluginParameters::createNotAutomatableParameterLayout(), 0, nullptr);
    
    dryWetMixer.setDryWetProportion(parameters.getRawParameterValue(PluginParameters::DRY_WET_ID)->load());
    compMixer.setDryWetProportion(parameters.getRawParameterValue(PluginParameters::COMP_DRY_WET_ID)->load());
    fadeMixer.setDryWetProportion(parameters.getRawParameterValue(PluginParameters::FADE_ID)->load());
    
    onnxProcessor1->onOnnxModelLoad = [this] (bool initLoading, juce::String modelName) {
        this->suspendProcessing(initLoading);
        
        if (!initLoading && modelName != "") {
            setExternalModelName(1, modelName);
        }
    };
    onnxProcessor2->onOnnxModelLoad = [this] (bool initLoading, juce::String modelName) {
        this->suspendProcessing(initLoading);
        
        if (!initLoading && modelName != "") {
            setExternalModelName(2, modelName);
        }
    };

    setInitialMuteParameters();
    initialiseRnbo();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.removeParameterListener(parameterID, this);
    }
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() {
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index) {
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index) {
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName) {
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec {sampleRate,
                                 static_cast<juce::uint32>(samplesPerBlock),
                                 static_cast<juce::uint32>(getTotalNumInputChannels())};

    dryBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    network1Buffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    network2Buffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    fadeBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    grain1DryBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    grain2DryBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    monoBuffer.setSize(1, (int)spec.maximumBlockSize);

    dryWetMixer.prepare(spec);
    fadeMixer.prepare(spec);
    compMixer.prepare(spec);
    grain1DryWetMixer.prepare(spec);
    grain2DryWetMixer.prepare(spec);
    onnxProcessor1->prepare(spec);
    onnxProcessor2->prepare(spec);
    iirCutoffFilter1->prepare(spec);
    iirCutoffFilter2->prepare(spec);
    processorTransientSplitter1->prepare(spec);
    processorTransientSplitter2->prepare(spec);
    processorCompressor->prepare(spec);
    audioVisualiser.prepare(spec);
    grainDelay1->prepare(spec);
    grainDelay2->prepare(spec);

    
    if (onnxProcessor1->getLatency() == onnxProcessor2->getLatency()) {
        setLatencySamples(onnxProcessor1->getLatency());
        dryWetMixer.setWetLatency(onnxProcessor1->getLatency());
//        std::cout << "latency 1: " << onnxProcessor1->getLatency() << std::endl;
//        std::cout << "latency 2: " << onnxProcessor2->getLatency() << std::endl
    } else {
        setLatencySamples(0);
        dryWetMixer.setWetLatency(0);
    }
}

void AudioPluginAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())

        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& ) {
    dryWetMixer.setDrySamples(buffer);
    stereoToMono(buffer, buffer);

    processorGain.processInputBlock(buffer);

    copyBuffer(network1Buffer, buffer);
    copyBuffer(network2Buffer, buffer);

    processorTransientSplitter1->processBlock(network1Buffer);
    processorTransientSplitter2->processBlock(network2Buffer);

    iirCutoffFilter1->processFilters(network1Buffer);
    iirCutoffFilter2->processFilters(network2Buffer);

    audioVisualiser.processSample(network1Buffer, network2Buffer);

    onnxProcessor1->processBlock(network1Buffer);
    onnxProcessor2->processBlock(network2Buffer);


    copyBuffer(grain1DryBuffer, network1Buffer);
    grain1DryWetMixer.setDrySamples(grain1DryBuffer);
    grainDelay1->processBlock(network1Buffer);
    grain1DryWetMixer.setWetSamples(network1Buffer);

    copyBuffer(grain2DryBuffer, network2Buffer);
    grain2DryWetMixer.setDrySamples(grain2DryBuffer);
    grainDelay2->processBlock(network2Buffer);
    grain2DryWetMixer.setWetSamples(network2Buffer);

    if (parameters.getRawParameterValue(PluginParameters::ON_OFF_NETWORK1_ID)->load() == 0.f)
        network1Buffer.clear();
    if (parameters.getRawParameterValue(PluginParameters::ON_OFF_NETWORK2_ID)->load() == 0.f)
        network2Buffer.clear();

    copyBuffer(buffer, network1Buffer);
    auto fadeSpec = juce::dsp::ProcessSpec{getSampleRate(), (uint32_t)network2Buffer.getNumSamples(), (uint32_t)network2Buffer.getNumChannels()};
    fadeMixer.prepare(fadeSpec);
    fadeMixer.setDrySamples(network2Buffer);
    fadeMixer.setWetSamples(buffer);

    compMixer.setDrySamples(buffer);
    processorCompressor->processBlock(buffer);
    compMixer.setWetSamples(buffer);

    processorGain.processOutputBlock(buffer);
    dryWetMixer.setWetSamples(buffer);
}

juce::AudioVisualiserComponent &AudioPluginAudioProcessor::getAudioVisualiser1() {
    return audioVisualiser.getAudioVisualiser(1);
}
juce::AudioVisualiserComponent &AudioPluginAudioProcessor::getAudioVisualiser2() {
    return audioVisualiser.getAudioVisualiser(2);
}

float AudioPluginAudioProcessor::getCurrentLevel() {
    return levelAnalyser.getCurrentLevel();
}

void AudioPluginAudioProcessor::setLevelType(LevelType newLevelType) {
    levelAnalyser.setLevelType(newLevelType);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
    return new AudioPluginAudioProcessorEditor (*this, parameters);
    //return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            advancedParameterControlVisible.referTo(parameters.state.getChildWithName("Settings")
            .getPropertyAsValue(PluginParameters::ADVANCED_PARAMETER_CONTROL_VISIBLE_ID, nullptr));
            network1Name.referTo(parameters.state.getChildWithName("Settings")
                                                            .getPropertyAsValue(PluginParameters::NETWORK1_NAME_ID, nullptr));
            network2Name.referTo(parameters.state.getChildWithName("Settings")
                                                            .getPropertyAsValue(PluginParameters::NETWORK2_NAME_ID, nullptr));
        }
}

void AudioPluginAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue) {
    processorCompressor->parameterChanged(parameterID, newValue);
    onnxProcessor1->parameterChanged(parameterID, newValue);
    onnxProcessor2->parameterChanged(parameterID, newValue);
    grainDelay1->parameterChanged(parameterID, newValue);
    grainDelay2->parameterChanged(parameterID, newValue);
    processorGain.parameterChanged(parameterID, newValue);
    iirCutoffFilter1->parameterChanged(parameterID, newValue);
    iirCutoffFilter2->parameterChanged(parameterID, newValue);
    processorTransientSplitter1->parameterChanged(parameterID, newValue);
    processorTransientSplitter2->parameterChanged(parameterID, newValue);

    if (parameterID == PluginParameters::DRY_WET_ID) {
        dryWetMixer.parameterChanged(parameterID, newValue);
    } else if (parameterID == PluginParameters::COMP_DRY_WET_ID) {
        compMixer.parameterChanged(parameterID, newValue);
    } else if (parameterID == PluginParameters::FADE_ID) {
        fadeMixer.parameterChanged(parameterID, newValue);
    } else if (parameterID == PluginParameters::GRAIN_NETWORK1_MIX_ID){
        grain1DryWetMixer.setDryWetProportion(newValue);
    } else if (parameterID == PluginParameters::GRAIN_NETWORK2_MIX_ID){
        grain2DryWetMixer.setDryWetProportion(newValue);
    }
}

void AudioPluginAudioProcessor::setInitialMuteParameters() {
    auto onOffGrain1 = parameters.getRawParameterValue(PluginParameters::GRAIN_ON_OFF_NETWORK1_ID)->load();
    auto onOffGrain2 = parameters.getRawParameterValue(PluginParameters::GRAIN_ON_OFF_NETWORK2_ID)->load();

    auto onOffNetwork1 = parameters.getRawParameterValue(PluginParameters::ON_OFF_NETWORK1_ID)->load();
    auto onOffNetwork2 = parameters.getRawParameterValue(PluginParameters::ON_OFF_NETWORK1_ID)->load();


    parameterChanged(PluginParameters::ON_OFF_NETWORK1_ID, onOffNetwork1);
    parameterChanged(PluginParameters::ON_OFF_NETWORK2_ID, onOffNetwork2);

    parameterChanged(PluginParameters::GRAIN_ON_OFF_NETWORK1_ID, onOffGrain1);
    parameterChanged(PluginParameters::GRAIN_ON_OFF_NETWORK2_ID, onOffGrain2);
}

void AudioPluginAudioProcessor::initialiseRnbo(){
    grainDelay1->setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK1_PITCH_ID), 2);
    grainDelay1->setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK1_INTERVAL_ID), 3);
    grainDelay1->setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK1_SIZE_ID), 1);
    grainDelay2->setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK2_PITCH_ID), 2);
    grainDelay2->setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK2_INTERVAL_ID), 3);
    grainDelay2->setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK2_SIZE_ID), 1);
}

void AudioPluginAudioProcessor::copyBuffer(juce::AudioBuffer<float>& target, juce::AudioBuffer<float>& source) {

    for (int channel = 0; channel < source.getNumChannels(); channel++){
        auto readPointer = source.getReadPointer(channel);
        auto writePointer = target.getWritePointer(channel);
        for (int sample = 0; sample < source.getNumSamples(); sample++){
            writePointer[sample] = readPointer[sample];
        }
    }
}

void AudioPluginAudioProcessor::stereoToMono(juce::AudioBuffer<float> &targetMonoBlock,
                                             juce::AudioBuffer<float> &sourceBlock)
{
    if (sourceBlock.getNumChannels() == 1)
        copyBuffer(targetMonoBlock, sourceBlock);
    else
    {
        auto monoWrite = targetMonoBlock.getWritePointer(0);

        auto lRead = sourceBlock.getReadPointer(0);
        auto rRead = sourceBlock.getReadPointer(1);
        for (int sample = 0; sample < sourceBlock.getNumSamples(); sample++) {
            monoWrite[sample] = lRead[sample] + rRead[sample];
        }
    }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AudioPluginAudioProcessor();
}


