# Ad-hoc code-signing of plugin bundles on macOS.
#
# Apple's linker auto-stamps binaries with a minimal ad-hoc signature, but it
# does not seal the bundle's resources or bind Info.plist. Strict hosts (e.g.
# Ableton Live 12's VST3 scanner) reject bundles in that state.
# Re-signing the whole bundle with `codesign --sign -` produces a proper
# resource seal so those hosts accept the plugin. The release pipeline overrides this with 
# a real Developer ID signature afterwards.

if(APPLE)
    foreach(fmt IN ITEMS VST3 AU Standalone)
        set(_tgt ${TARGET_NAME}_${fmt})
        if(TARGET ${_tgt})
            add_custom_command(TARGET ${_tgt} POST_BUILD
                COMMAND codesign --force --deep --sign - --timestamp=none
                        "$<TARGET_BUNDLE_DIR:${_tgt}>"
                COMMENT "Ad-hoc signing ${_tgt} bundle"
                VERBATIM)
        endif()
    endforeach()
endif()
