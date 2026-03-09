#include "real_engine_plugin.h"
#include "real_engine_theme.h"
#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "core/string/print_string.h"
#include "scene/main/node.h"

void RealEnginePlugin::_bind_methods() {
}

RealEnginePlugin::RealEnginePlugin() {
    print_line("[REAL ENGINE]: Plugin created");
}

RealEnginePlugin::~RealEnginePlugin() {
    print_line("[REAL ENGINE]: Plugin destroyed");
}

void RealEnginePlugin::_enter_tree() {
    print_line("[REAL ENGINE]: Plugin initialized");
    setup_editor_settings();
    apply_real_engine_theme();
}

void RealEnginePlugin::_exit_tree() {
    print_line("[REAL ENGINE]: Plugin cleanup");

    if (EditorNode::get_singleton()) {
        EditorNode::get_singleton()->get_gui_base()->set_theme(Ref<Theme>());
    }
}

String RealEnginePlugin::get_plugin_name() const {
    return "RealEngine";
}

String RealEnginePlugin::get_plugin_version() const {
    return "1.0";
}

bool RealEnginePlugin::has_main_screen() const {
    return false;
}

void RealEnginePlugin::apply_real_engine_theme() {
    EditorSettings *settings = EditorSettings::get_singleton();
    if (!settings) {
        ERR_PRINT("[REAL ENGINE]: EditorSettings not available");
        return;
    }

    bool apply_theme = false;

    if (settings->has_setting("interface/theme/color_preset")) {
        String preset = settings->get("interface/theme/color_preset");
        if (preset == "Real Engine") {
            apply_theme = true;
        }
    }

    if (apply_theme && EditorNode::get_singleton()) {
        print_line("[REAL ENGINE]: Applying Real Engine theme");

        // Создаем тему
        Ref<RealEngineTheme> real_theme;
        real_theme.instantiate();
        real_theme->apply_real_engine_theme();

        // Применяем
        EditorNode::get_singleton()->get_gui_base()->set_theme(real_theme);

        EditorNode::get_singleton()->get_gui_base()->queue_redraw();

        print_line("[REAL ENGINE]: Theme applied successfully");
    }
}

void RealEnginePlugin::setup_editor_settings() {
    EditorSettings *settings = EditorSettings::get_singleton();
    if (!settings) return;

    print_line("[REAL ENGINE]: Editor settings configured");
}
