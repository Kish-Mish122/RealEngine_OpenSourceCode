#include "real_engine_theme.h"
#include "core/string/print_string.h"

void RealEngineTheme::_bind_methods() {
    ClassDB::bind_method(D_METHOD("apply_real_engine_theme"), &RealEngineTheme::apply_real_engine_theme);
}

RealEngineTheme::RealEngineTheme() {
}

void RealEngineTheme::apply_real_engine_theme() {
    print_line("[REAL ENGINE]: Applying dark theme");

    set_color("base_color", "Editor", Color(0.12, 0.12, 0.2));
    set_color("dark_color", "Editor", Color(0.08, 0.08, 0.15));
    set_color("contrast_color", "Editor", Color(0.18, 0.18, 0.25));
    set_color("accent_color", "Editor", Color(0.3, 0.8, 0.3));
    set_color("font_color", "Editor", Color(0.95, 0.95, 0.95));
}
