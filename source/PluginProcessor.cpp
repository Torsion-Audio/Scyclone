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
        parameters (*this, nullptr, juce::Identifier ("Scyclone"), PluginParameters::createParameterLayout()),
        processorTransientSplitter1(parameters, 1),
        processorTransientSplitter2(parameters, 2),
        iirCutoffFilter1(parameters, 1),
        iirCutoffFilter2(parameters, 2),
        onnxProcessor1(parameters, 1, FunkDrum),
        onnxProcessor2(parameters, 2, Djembe),
        grainDelay1(1),
        grainDelay2(2),
        processorCompressor(parameters)
{

    network1Name = "Funk";
    network2Name = "Djembe";

    for (auto & parameterID : PluginParameters::getPluginParameterList()) {
        parameters.addParameterListener(parameterID, this);
    }

    parameters.state.addChild(PluginParameters::createNotAutomatableParameterLayout(), 0, nullptr);
    
    dryWetMixer.setDryWetProportion(parameters.getRawParameterValue(PluginParameters::DRY_WET_ID.getParamID())->load());
    compMixer.setDryWetProportion(parameters.getRawParameterValue(PluginParameters::COMP_DRY_WET_ID.getParamID())->load());
    fadeMixer.setDryWetProportion(parameters.getRawParameterValue(PluginParameters::FADE_ID.getParamID())->load());
    
    onnxProcessor1.onOnnxModelLoad = [this] (bool initLoading, juce::String modelName) {
        this->suspendProcessing(initLoading);
//        std::cout << "Onnx proc 1 suspend:" << initLoading << std::endl; //DBG
        if (!initLoading && modelName != "") {
            setExternalModelName(1, modelName);
        }
    };
    onnxProcessor2.onOnnxModelLoad = [this] (bool initLoading, juce::String modelName) {
        this->suspendProcessing(initLoading);
//        std::cout << "Onnx proc 2 suspend:" << initLoading << std::endl; //DBG
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
    PluginParameters::removeNotAutomatableParameterLayout(parameters.state.getChild(0));
    parameters.state.removeChild(0, nullptr);
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
    juce::dsp::ProcessSpec monoSpec {sampleRate,
                                 static_cast<juce::uint32>(samplesPerBlock),
                                 static_cast<juce::uint32>(1)};

    network1Buffer.setSize((int) monoSpec.numChannels, (int) monoSpec.maximumBlockSize);
    network2Buffer.setSize((int) monoSpec.numChannels, (int) monoSpec.maximumBlockSize);
    fadeBuffer.setSize((int) monoSpec.numChannels, (int) monoSpec.maximumBlockSize);
    grain1DryBuffer.setSize((int) monoSpec.numChannels, (int) monoSpec.maximumBlockSize);
    grain2DryBuffer.setSize((int) monoSpec.numChannels, (int) monoSpec.maximumBlockSize);
    monoBuffer.setSize((int) monoSpec.numChannels, (int) monoSpec.maximumBlockSize);

    dryWetMixer.prepare(spec);
    
    fadeMixer.prepare(monoSpec);
    compMixer.prepare(monoSpec);
    grain1DryWetMixer.prepare(monoSpec);
    grain2DryWetMixer.prepare(monoSpec);
    onnxProcessor1.prepare(monoSpec);
    onnxProcessor2.prepare(monoSpec);
    iirCutoffFilter1.prepare(monoSpec);
    iirCutoffFilter2.prepare(monoSpec);
    processorTransientSplitter1.prepare(monoSpec);
    processorTransientSplitter2.prepare(monoSpec);
    processorCompressor.prepare(monoSpec);
    audioVisualiser.prepare(monoSpec);
    grainDelay1.prepare(monoSpec);
    grainDelay2.prepare(monoSpec);

    
    if (onnxProcessor1.getLatency() == onnxProcessor2.getLatency()) {
        setLatencySamples(onnxProcessor1.getLatency());
        dryWetMixer.setWetLatency(onnxProcessor1.getLatency());
//        std::cout << "latency 1: " << onnxProcessor1.getLatency() << std::endl; //DBG
//        std::cout << "latency 2: " << onnxProcessor2.getLatency() << std::endl; //DBG
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
    stereoToMono(monoBuffer, buffer);
    
    processorGain.processInputBlock(monoBuffer);

    network1Buffer.makeCopyOf(monoBuffer);
    network2Buffer.makeCopyOf(monoBuffer);

    processorTransientSplitter1.processBlock(network1Buffer);
    processorTransientSplitter2.processBlock(network2Buffer);

    iirCutoffFilter1.processFilters(network1Buffer);
    iirCutoffFilter2.processFilters(network2Buffer);

    audioVisualiser.processSample(network1Buffer, network2Buffer);

    onnxProcessor1.processBlock(network1Buffer);
    onnxProcessor2.processBlock(network2Buffer);

    levelAnalyser1.processBlock(network1Buffer);
    levelAnalyser2.processBlock(network2Buffer);

    grain1DryBuffer.makeCopyOf(network1Buffer);
    grain1DryWetMixer.setDrySamples(grain1DryBuffer);
    grainDelay1.processBlock(network1Buffer);
    grain1DryWetMixer.setWetSamples(network1Buffer);

    grain2DryBuffer.makeCopyOf(network2Buffer);
    grain2DryWetMixer.setDrySamples(grain2DryBuffer);
    grainDelay2.processBlock(network2Buffer);
    grain2DryWetMixer.setWetSamples(network2Buffer);

    if (parameters.getRawParameterValue(PluginParameters::ON_OFF_NETWORK1_ID.getParamID())->load() == 0.f)
        network1Buffer.clear();
    if (parameters.getRawParameterValue(PluginParameters::ON_OFF_NETWORK2_ID.getParamID())->load() == 0.f)
        network2Buffer.clear();

    monoBuffer.makeCopyOf(network1Buffer);
    fadeMixer.setDrySamples(network2Buffer);
    fadeMixer.setWetSamples(monoBuffer);

    compMixer.setDrySamples(monoBuffer);
    processorCompressor.processBlock(monoBuffer);
    compMixer.setWetSamples(monoBuffer);

    processorGain.processOutputBlock(monoBuffer);
    monoToStereo(buffer, monoBuffer);
    dryWetMixer.setWetSamples(buffer);
}

juce::AudioVisualiserComponent &AudioPluginAudioProcessor::getAudioVisualiser1() {
    return audioVisualiser.getAudioVisualiser(1);
}
juce::AudioVisualiserComponent &AudioPluginAudioProcessor::getAudioVisualiser2() {
    return audioVisualiser.getAudioVisualiser(2);
}

float AudioPluginAudioProcessor::getCurrentLevel(int index) {
    if (index == 1) return levelAnalyser1.getCurrentLevel();
    else if (index == 2) return levelAnalyser2.getCurrentLevel();
    else return (levelAnalyser1.getCurrentLevel() + levelAnalyser2.getCurrentLevel()) / 2.f;
}

void AudioPluginAudioProcessor::setLevelType(LevelType newLevelType) {
    levelAnalyser1.setLevelType(newLevelType);
    levelAnalyser2.setLevelType(newLevelType);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
    return new AudioPluginAudioProcessorEditor (*this, parameters);
//    return new juce::GenericAudioProcessorEditor (*this);
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
            .getPropertyAsValue(PluginParameters::ADVANCED_PARAMETER_CONTROL_VISIBLE_NAME, nullptr));
            network1Name.referTo(parameters.state.getChildWithName("Settings")
                                                            .getPropertyAsValue(PluginParameters::NETWORK1_NAME_NAME, nullptr));
            network2Name.referTo(parameters.state.getChildWithName("Settings")
                                                            .getPropertyAsValue(PluginParameters::NETWORK2_NAME_NAME, nullptr));
        }
}

void AudioPluginAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue) {
    processorCompressor.parameterChanged(parameterID, newValue);
    onnxProcessor1.parameterChanged(parameterID, newValue);
    onnxProcessor2.parameterChanged(parameterID, newValue);
    grainDelay1.parameterChanged(parameterID, newValue);
    grainDelay2.parameterChanged(parameterID, newValue);
    processorGain.parameterChanged(parameterID, newValue);
    iirCutoffFilter1.parameterChanged(parameterID, newValue);
    iirCutoffFilter2.parameterChanged(parameterID, newValue);
    processorTransientSplitter1.parameterChanged(parameterID, newValue);
    processorTransientSplitter2.parameterChanged(parameterID, newValue);

    if (parameterID == PluginParameters::DRY_WET_ID.getParamID()) {
        dryWetMixer.parameterChanged(parameterID, newValue);
    } else if (parameterID == PluginParameters::COMP_DRY_WET_ID.getParamID()) {
        compMixer.parameterChanged(parameterID, newValue);
    } else if (parameterID == PluginParameters::FADE_ID.getParamID()) {
        fadeMixer.parameterChanged(parameterID, newValue);
    } else if (parameterID == PluginParameters::GRAIN_NETWORK1_MIX_ID.getParamID()){
        grain1DryWetMixer.setDryWetProportion(newValue);
    } else if (parameterID == PluginParameters::GRAIN_NETWORK2_MIX_ID.getParamID()){
        grain2DryWetMixer.setDryWetProportion(newValue);
    }
}

void AudioPluginAudioProcessor::setInitialMuteParameters() {
    auto onOffGrain1 = parameters.getRawParameterValue(PluginParameters::GRAIN_ON_OFF_NETWORK1_ID.getParamID())->load();
    auto onOffGrain2 = parameters.getRawParameterValue(PluginParameters::GRAIN_ON_OFF_NETWORK2_ID.getParamID())->load();

    auto onOffNetwork1 = parameters.getRawParameterValue(PluginParameters::ON_OFF_NETWORK1_ID.getParamID())->load();
    auto onOffNetwork2 = parameters.getRawParameterValue(PluginParameters::ON_OFF_NETWORK1_ID.getParamID())->load();


    parameterChanged(PluginParameters::ON_OFF_NETWORK1_ID.getParamID(), onOffNetwork1);
    parameterChanged(PluginParameters::ON_OFF_NETWORK2_ID.getParamID(), onOffNetwork2);

    parameterChanged(PluginParameters::GRAIN_ON_OFF_NETWORK1_ID.getParamID(), onOffGrain1);
    parameterChanged(PluginParameters::GRAIN_ON_OFF_NETWORK2_ID.getParamID(), onOffGrain2);
}

void AudioPluginAudioProcessor::initialiseRnbo(){
    grainDelay1.setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK1_PITCH_ID.getParamID()), 2);
    grainDelay1.setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK1_INTERVAL_ID.getParamID()), 3);
    grainDelay1.setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK1_SIZE_ID.getParamID()), 1);
    grainDelay2.setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK2_PITCH_ID.getParamID()), 2);
    grainDelay2.setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK2_INTERVAL_ID.getParamID()), 3);
    grainDelay2.setParameterValue(parameters.getRawParameterValue(PluginParameters::GRAIN_NETWORK2_SIZE_ID.getParamID()), 1);
}

void AudioPluginAudioProcessor::stereoToMono(juce::AudioBuffer<float> &targetMonoBlock, juce::AudioBuffer<float> &sourceBlock) {
    if (sourceBlock.getNumChannels() == 1) {
        targetMonoBlock.makeCopyOf(sourceBlock);
    } else {
        auto nSamples = sourceBlock.getNumSamples();

        auto monoWrite = targetMonoBlock.getWritePointer(0);
        auto lRead = sourceBlock.getReadPointer(0);
        auto rRead = sourceBlock.getReadPointer(1);

        juce::FloatVectorOperations::copy(monoWrite, lRead, nSamples);
        juce::FloatVectorOperations::add(monoWrite, rRead, nSamples);
        juce::FloatVectorOperations::multiply(monoWrite, 0.5f, nSamples);
    }
}

void AudioPluginAudioProcessor::monoToStereo(juce::AudioBuffer<float> &targetStereoBlock, juce::AudioBuffer<float> &sourceBlock) {
    if (sourceBlock.getNumChannels() == 2) {
        targetStereoBlock.makeCopyOf(sourceBlock);
    } else {
        auto nSamples = sourceBlock.getNumSamples();

        auto lWrite = targetStereoBlock.getWritePointer(0);
        auto rWrite = targetStereoBlock.getWritePointer(1);
        auto monoRead = sourceBlock.getReadPointer(0);

        juce::FloatVectorOperations::copy(lWrite, monoRead, nSamples);
        juce::FloatVectorOperations::copy(rWrite, monoRead, nSamples);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AudioPluginAudioProcessor();
}


