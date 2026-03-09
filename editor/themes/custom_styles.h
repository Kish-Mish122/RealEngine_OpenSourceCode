// custom_styles.h
#ifndef CUSTOM_STYLES_H
#define CUSTOM_STYLES_H

#include "scene/resources/style_box_flat.h"
#include "scene/resources/font.h"

class RealEngineStyles {
public:
    static Ref<StyleBoxFlat> create_panel_style() {
        Ref<StyleBoxFlat> style;
        style.instance();

        style->set_bg_color(Color(0.2, 0.23, 0.28));
        style->set_border_color(Color(0.3, 0.34, 0.4));
        style->set_border_width_all(1);
        style->set_corner_radius_all(4);

        return style;
    }

    static Ref<StyleBoxFlat> create_button_style() {
        Ref<StyleBoxFlat> style;
        style.instance();

        style->set_bg_color(Color(0.25, 0.28, 0.33));
        style->set_border_color(Color(0.35, 0.4, 0.45));
        style->set_border_width_all(1);
        style->set_corner_radius_all(3);

        return style;
    }
};

#endif // CUSTOM_STYLES_H
