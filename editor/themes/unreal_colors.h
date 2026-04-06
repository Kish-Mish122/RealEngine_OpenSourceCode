#pragma once

#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "core/string/print_string.h"

static void apply_unreal_colors() {
    EditorNode *editor = EditorNode::get_singleton();
    if (!editor) return;
    
    EditorSettings *settings = EditorSettings::get_singleton();
    if (!settings) return;
    
    String preset = settings->get_setting("interface/theme/color_preset");
    if (preset != "Unreal Style") return;
    
    print_line("[REAL ENGINE]: Applying Unreal colors");
    
    Ref<Theme> theme = editor->get_gui_base()->get_theme();
    if (theme.is_null()) return;
    
    // Только цвета! Стили не трогаем
    theme->set_color("base_color", "Editor", Color(0.08, 0.08, 0.10));
    theme->set_color("dark_color", "Editor", Color(0.08, 0.08, 0.10));
    theme->set_color("contrast_color", "Editor", Color(0.12, 0.12, 0.15));
    theme->set_color("accent_color", "Editor", Color(0.15, 0.55, 0.85));
    theme->set_color("font_color", "Editor", Color(0.95, 0.95, 0.98));
    theme->set_color("font_secondary_color", "Editor", Color(0.70, 0.70, 0.75));
    theme->set_color("font_disabled_color", "Editor", Color(0.45, 0.45, 0.50));
    
    editor->get_gui_base()->set_theme(theme);
    editor->get_gui_base()->queue_redraw();
    
    print_line("[REAL ENGINE]: Unreal colors applied");
}
