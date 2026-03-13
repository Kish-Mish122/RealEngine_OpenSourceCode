#include "real_engine_plugin.h"
#include "real_engine_theme.h"
#include "real_engine_theme_light.h"
#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "core/string/print_string.h"

void RealEnginePlugin::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_apply_theme_later"), &RealEnginePlugin::_apply_theme_later);
}

RealEnginePlugin::RealEnginePlugin() {
    print_line("[REAL ENGINE]: Plugin created");
    callable_mp(this, &RealEnginePlugin::_apply_theme_later).call_deferred();
}

RealEnginePlugin::~RealEnginePlugin() {
    print_line("[REAL ENGINE]: Plugin destroyed");
}

void RealEnginePlugin::_enter_tree() {
    print_line("[REAL ENGINE]: Plugin entered tree");
}

void RealEnginePlugin::_exit_tree() {
    print_line("[REAL ENGINE]: Plugin exited tree");
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

void RealEnginePlugin::_apply_theme() {
    EditorNode *editor = EditorNode::get_singleton();
    EditorSettings *settings = EditorSettings::get_singleton();

    if (!editor || !settings) return;

    String preset = "Real Engine";
    if (settings->has_setting("interface/theme/color_preset")) {
        preset = settings->get_setting("interface/theme/color_preset");
    }

    Ref<Theme> theme;

    if (preset == "Real Engine Light") {
        Ref<RealEngineThemeLight> light_theme;
        light_theme.instantiate();
        light_theme->apply_light_theme();
        theme = light_theme;
        print_line("[REAL ENGINE]: Light theme created");
    } else {
        Ref<RealEngineTheme> dark_theme;
        dark_theme.instantiate();
        dark_theme->apply_real_engine_theme();
        theme = dark_theme;
        print_line("[REAL ENGINE]: Dark theme created");
    }

    editor->get_gui_base()->set_theme(theme);
    editor->get_gui_base()->queue_redraw();
}

void RealEnginePlugin::_apply_theme_later() {
    EditorNode *editor = EditorNode::get_singleton();
    if (!editor || !editor->get_gui_base()) {
        callable_mp(this, &RealEnginePlugin::_apply_theme_later).call_deferred();
        return;
    }
    _apply_theme();
}
