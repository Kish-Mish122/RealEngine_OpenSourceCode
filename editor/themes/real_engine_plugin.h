#pragma once

#include "editor/plugins/editor_plugin.h"

class RealEnginePlugin : public EditorPlugin {
    GDCLASS(RealEnginePlugin, EditorPlugin);

protected:
    static void _bind_methods();

public:
    RealEnginePlugin();
    ~RealEnginePlugin();

    // БЕЗ override - эти методы не виртуальные в EditorPlugin
    void _enter_tree();
    void _exit_tree();

    // Эти методы виртуальные - можно оставить
    virtual String get_plugin_name() const override;
    virtual String get_plugin_version() const override;
    virtual bool has_main_screen() const override;

private:
    void _apply_theme();
    void _apply_theme_later();
};
