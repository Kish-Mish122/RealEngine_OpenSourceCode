#pragma once

#include "editor/plugins/editor_plugin.h"

class RealEnginePlugin : public EditorPlugin {
    GDCLASS(RealEnginePlugin, EditorPlugin);

protected:
    static void _bind_methods();

public:
    RealEnginePlugin();
    ~RealEnginePlugin();

    // Убираем override для этих методов, так как они не виртуальные в EditorPlugin
    virtual void _enter_tree();
    virtual void _exit_tree();

    // Эти методы с override, потому что они виртуальные
    virtual String get_plugin_name() const override;
    virtual String get_plugin_version() const override;
    virtual bool has_main_screen() const override;

    // Наши методы
    void apply_real_engine_theme();
    void setup_editor_settings();
};
