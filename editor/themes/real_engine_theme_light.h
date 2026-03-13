#pragma once

#include "editor/themes/editor_theme.h"

class RealEngineThemeLight : public EditorTheme {
    GDCLASS(RealEngineThemeLight, EditorTheme);

protected:
    static void _bind_methods();

public:
    RealEngineThemeLight();
    void apply_light_theme();
};
