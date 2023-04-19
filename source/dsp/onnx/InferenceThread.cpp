//
// Created by valentin.ackva on 16.03.2023.
//

#include "InferenceThread.h"

InferenceThread::InferenceThread(RaveModel raveModel) : juce::Thread("OnnxInference"), session(nullptr), currentLevel(raveModel){
    modelInputSizeChanged(modelInputSize);
    setInternalModel();
}

InferenceThread::~InferenceThread() {
    session.release();
    stopThread(100);
}

void InferenceThread::prepare(const juce::dsp::ProcessSpec &spec) {
    // allocate enough memory
    receiveRingBuffer.initialise(1, (int) spec.sampleRate);
    
    last_spec = spec;
    init = true;
    init_samples = 0;
}

void InferenceThread::sendAudio(juce::AudioBuffer<float> &buffer) {
    auto readPointer = buffer.getReadPointer(0);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        receiveRingBuffer.pushSample(readPointer[sample], 0);
        if (init) init_samples++;
    }

    if (!isThreadRunning() && receiveRingBuffer.getAvailableSamples(0) >= modelInputSize && !loadingModel) {

        for (float & sample : onnxInputData) {
            sample = receiveRingBuffer.popSample(0);
        }

        startThread(juce::Thread::Priority::highest);
    }
    if (init && init_samples >= modelInputSize + maxModelCalcSize) init = false;
}

void InferenceThread::run() {

    auto start = std::chrono::high_resolution_clock::now();

    //process
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    // define shape
    std::array<int64_t, 3> inputShape = {1, 1, modelInputSize};
    std::array<int64_t, 3> outputShape = {1, 1, modelInputSize};

    auto inputTensor = std::make_shared<Ort::Value>(Ort::Value::CreateTensor<float>(memory_info,
                                                                                    onnxInputData.data(),
                                                                                    modelInputSize,
                                                                                    inputShape.data(),
                                                                                    inputShape.size()));

    auto outputTensor = std::make_shared<Ort::Value>(Ort::Value::CreateTensor<float>(memory_info,
                                                                                     onnxOutputData.data(),
                                                                                     modelInputSize,
                                                                                     outputShape.data(),
                                                                                     outputShape.size()));

    Ort::AllocatorWithDefaultOptions ort_alloc;

    Ort::AllocatedStringPtr inputName = session.GetInputNameAllocated(0, ort_alloc);
    Ort::AllocatedStringPtr outputName = session.GetOutputNameAllocated(0, ort_alloc);
    const std::array<const char *, 1> inputNames = {(char*) inputName.get()};
    const std::array<const char *, 1> outputNames = {(char*) outputName.get()};

    // run inference
    try {
        session.Run(runOptions, inputNames.data(), inputTensor.get(), 1, outputNames.data(), outputTensor.get(), 1);
    } catch (Ort::Exception &e) {
        std::cout << e.what() << std::endl;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    // std::cout << duration.count() << "ms" << std::endl;

    for (int i = 0; i < processedBuffer.getNumSamples(); ++i) {
        processedBuffer.setSample(0, i, onnxOutputData[i]);
    }

    onNewProcessedBuffer(processedBuffer);
//    ort_alloc.Free(inputName.get());
}

void InferenceThread::setExternalModel(juce::File modelPath) {
    loadExternalModel(modelPath);
}

void InferenceThread::modelInputSizeChanged(int newModelInputSize) {
    modelInputSize = newModelInputSize;
    processedBuffer.setSize(1, newModelInputSize);

    onnxInputData.resize(newModelInputSize, 0.0f);
    onnxOutputData.resize(newModelInputSize, 0.0f);
}

void InferenceThread::loadExternalModel(juce::File modelPath) {
    loadingModel = true;

    if (isThreadRunning()) {
        stopThread(10);

        while (!isThreadRunning()) {
            juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 1);
        }
    }

    Ort::SessionOptions sessionOptions;



#if JUCE_WINDOWS
    auto modelPathToLoad = modelPath.getFullPathName().toStdString();
    std::wstring modelWideStr = std::wstring(modelPathToLoad.begin(), modelPathToLoad.end());
    const wchar_t* modelWideCStr = modelWideStr.c_str();

    session = Ort::Session(env,
                       modelWideCStr,
                       sessionOptions);
#else
    auto modelPathToLoad = modelPath.getFullPathName().toStdString();
    const char* modelCStr = modelPathToLoad.c_str();

    session = Ort::Session(env,
                           modelCStr,
                           sessionOptions);
#endif

    prepare(last_spec);

    auto shape = getInputShape(&session);

    for (int i = 0; i < shape.size(); ++i) {
//        std::cout << "shape[" <<  i << "]: " << shape[i] << std::endl;
    }
    onModelLoaded(modelPath.getFileNameWithoutExtension());
    loadingModel = false;
}

void InferenceThread::loadInternalModel(RaveModel modelToLoad) {
    loadingModel = true;

    if (isThreadRunning()) {
        stopThread(10);

        while (!isThreadRunning()) {
            juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 1);
        }
    }

    Ort::SessionOptions sessionOptions;

    switch (modelToLoad) {
        default:
            //not implemented
        case FunkDrum:
            session = Ort::Session(env,
                                   BinaryData::funk_drums_ort,
                                   BinaryData::funk_drums_ortSize,
                                   sessionOptions);
            break;
        case Djembe:
            session = Ort::Session(env,
                                   BinaryData::djembe_ort,
                                   BinaryData::djembe_ortSize,
                                   sessionOptions);
            break;
    }
    if (! startUp){
        prepare(last_spec);
    }

    auto shape = getInputShape(&session);

    for (int i = 0; i < shape.size(); ++i) {
//        std::cout << "shape[" <<  i << "]: " << shape[i] << std::endl;
    }
    if (! startUp){
        onModelLoaded("");
    }
    loadingModel = false;
    startUp = false;
}

std::vector<int> InferenceThread::getInputShape(Ort::Session *sess) {
    std::vector<int> returnVec;
    std::vector<int64_t> inputShape = sess->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    for (long long dim : inputShape) {
        returnVec.push_back(static_cast<int>(dim));
    }
    return returnVec;
}

int InferenceThread::getLatency(){
    return modelInputSize + maxModelCalcSize;
}

void InferenceThread::setInternalModel() {
    switch(currentLevel) {
        case FunkDrum:
            loadInternalModel(FunkDrum);
            break;
        case Djembe:
            loadInternalModel(Djembe);
            break;
        default:
            loadInternalModel(FunkDrum);
    }
}
