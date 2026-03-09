#pragma once

#include "editor/themes/editor_theme.h"
#include "scene/resources/style_box_flat.h"

class RealEngineTheme : public EditorTheme {
    GDCLASS(RealEngineTheme, EditorTheme);

protected:
    static void _bind_methods();

    struct RealEngineColors {
        // Основные цвета
        Color primary = Color(0.3, 0.8, 0.3);      // Зелёный
        Color secondary = Color(0.3, 0.8, 0.3);    // Зеленый (для галочек и иконок)
        Color success = Color(0.2, 0.8, 0.3);      // Зеленый (успех)
        Color warning = Color(0.9, 0.6, 0.1);      // Оранжевый
        Color error = Color(0.9, 0.2, 0.2);        // Красный

        // Фоны (темно-синие)
        Color background_dark = Color(0.08, 0.08, 0.15);    // Очень темно-синий
        Color background_medium = Color(0.12, 0.12, 0.2);   // Темно-синий
        Color background_light = Color(0.18, 0.18, 0.25);   // Светло-синий

        // Текст
        Color text_primary = Color(0.95, 0.95, 0.95);
        Color text_secondary = Color(0.7, 0.7, 0.7);
        Color text_disabled = Color(0.4, 0.4, 0.4);

        // Панели
        Color panel_header = Color(0.15, 0.15, 0.25);
        Color panel_border = Color(0.25, 0.25, 0.35);

        // Подсветка синтаксиса
        Color syntax_keyword = Color(1.0, 0.5, 0.5);
        Color syntax_function = Color(0.5, 0.8, 1.0);
        Color syntax_comment = Color(0.4, 0.5, 0.4);
        Color syntax_string = Color(0.7, 0.9, 0.5);
        Color syntax_number = Color(0.8, 0.7, 0.4);

        // Цвета для иконок и галочек
        Color icon_normal = Color(0.3, 0.8, 0.3);     // Зеленый
        Color icon_hover = Color(0.4, 0.9, 0.4);      // Светло-зеленый
        Color icon_pressed = Color(0.2, 0.7, 0.2);    // Темно-зеленый
        Color icon_disabled = Color(0.3, 0.5, 0.3);   // Приглушенный зеленый

        Color checkbox_checked = Color(0.3, 0.8, 0.3);    // Зеленый
        Color checkbox_unchecked = Color(0.5, 0.5, 0.5);  // Серый
        Color checkbox_hover = Color(0.4, 0.9, 0.4);      // Светло-зеленый
    };

    RealEngineColors colors;

public:
    RealEngineTheme();

    void apply_real_engine_theme();
    void update_colors_from_settings();

    const RealEngineColors& get_colors() const { return colors; }
};
