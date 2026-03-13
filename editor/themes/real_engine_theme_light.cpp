#include "real_engine_theme_light.h"
#include "core/string/print_string.h"

void RealEngineThemeLight::_bind_methods() {
    ClassDB::bind_method(D_METHOD("apply_light_theme"), &RealEngineThemeLight::apply_light_theme);
}

RealEngineThemeLight::RealEngineThemeLight() {
}

void RealEngineThemeLight::apply_light_theme() {
    print_line("[REAL ENGINE]: Applying light theme");

    set_color("base_color", "Editor", Color(0.95, 0.95, 0.95));
    set_color("dark_color", "Editor", Color(0.85, 0.85, 0.85));
    set_color("contrast_color", "Editor", Color(0.98, 0.98, 0.98));
    set_color("accent_color", "Editor", Color(0.2, 0.6, 0.3));
    set_color("font_color", "Editor", Color(0.1, 0.1, 0.1));
}
