juce_add_gui_app(ScycloneInstaller
        ICON_BIG "${CMAKE_CURRENT_LIST_DIR}/assets/logo.png"
        ICON_SMALL "${CMAKE_CURRENT_LIST_DIR}/assets/logo.png"
        PRODUCT_NAME "Scyclone Installer")

juce_generate_juce_header(ScycloneInstaller)

target_sources(ScycloneInstaller
        PRIVATE
        source/Main.cpp

        source/operations/OperationManager.cpp
        source/operations/singleOperations/FileDownload.cpp
        source/operations/singleOperations/RepoInspector.cpp
        source/operations/singleOperations/ZipExtractor.cpp
        source/operations/singleOperations/PluginInstaller.cpp

        source/ui/MainWindow.cpp
        source/ui/MainComponent.cpp
        source/ui/components/WelcomePage.cpp
        source/ui/components/FormatPage.cpp
        source/ui/components/PathPage.cpp
        source/ui/components/ProcessingPage.cpp
        source/ui/components/EndPage.cpp

        source/ui/components/customComponent/CustomButton.cpp
        source/ui/components/customComponent/CustomLabel.cpp
)

target_compile_definitions(ScycloneInstaller
        PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:ScycloneInstaller,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:ScycloneInstaller,JUCE_VERSION>"
)

juce_add_binary_data(InstallBinaryData SOURCES
        assets/background.svg
        assets/button_off.svg
        assets/button_on.svg
)

if(MSVC)
    SET_TARGET_PROPERTIES(ScycloneInstaller PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS")
endif()

target_link_libraries(ScycloneInstaller
        PRIVATE
        InstallBinaryData
        juce::juce_gui_extra
        juce::juce_core
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)