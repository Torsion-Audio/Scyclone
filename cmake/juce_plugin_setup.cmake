# If you are building a VST2 or AAX plugin, CMake needs to be told where to find these SDKs on your
# system. This setup should be done before calling `juce_add_plugin`.
# juce_set_vst2_sdk_path(...)
# juce_set_aax_sdk_path(...)


# `juce_add_plugin` adds a static library target with the name passed as the first argument
# (Scyclone here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. As well as this shared code static library, this function adds targets for each of
# the formats specified by the FORMATS arguments. This function accepts many optional arguments.
# Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_plugin(${TARGET_NAME}
        # VERSION ...                               # Set this if the plugin version is different to the project version
        # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
        # ICON_SMALL ...
        COMPANY_NAME "Torsion Audio"
        # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
        # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
        # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
        # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
        # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
        # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
        PLUGIN_MANUFACTURER_CODE TORA               # A four-character manufacturer id with at least one upper-case character
        PLUGIN_CODE SCYC                            # A unique four-character plugin id with exactly one upper-case character
        # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case

        if(APPLE)
        HARDENED_RUNTIME_ENABLED TRUE
        HARDENED_RUNTIME_OPTIONS "com.apple.security.device.audio-input"
        MICROPHONE_PERMISSION_ENABLED TRUE
        MICROPHONE_PERMISSION_TEXT "Need access to your audio interface"
        endif()

        FORMATS ${FORMATS_TO_BUILD}                    # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME "Scyclone"			            # The name of the final executable, which can differ from the target name
)

# JUCE Headers and definitions
juce_generate_juce_header(${TARGET_NAME})

# Now we can set the properties
set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON

        # Export only public symbols
        # We set this option for all our libraries since the onnxruntime lib and the google benchmark lib are setup with hidden visibility this otherwise we get linker warnings that say ... means the weak symbol cannot be overridden at runtime. This was likely caused by different translation units being compiled with different visibility settings
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
)