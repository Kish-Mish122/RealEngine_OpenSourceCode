#include "real_engine_theme.h"
#include "editor/settings/editor_settings.h"
#include "editor/themes/editor_icons.h"
#include "core/string/print_string.h"

void RealEngineTheme::_bind_methods() {
    ClassDB::bind_method(D_METHOD("apply_real_engine_theme"), &RealEngineTheme::apply_real_engine_theme);
}

RealEngineTheme::RealEngineTheme() {
    update_colors_from_settings();
}

void RealEngineTheme::apply_real_engine_theme() {

    // ===== БАЗОВЫЕ ЦВЕТА =====
    set_color("base_color", "Editor", colors.background_medium);
    set_color("dark_color", "Editor", colors.background_dark);
    set_color("contrast_color", "Editor", colors.background_light);
    set_color("accent_color", "Editor", colors.primary);

    // ===== ЦВЕТА ТЕКСТА =====
    set_color("font_color", "Editor", colors.text_primary);
    set_color("font_secondary_color", "Editor", colors.text_secondary);
    set_color("font_disabled_color", "Editor", colors.text_disabled);

    // ===== ЦВЕТА ПАНЕЛЕЙ =====
    set_color("background_color", "Panel", colors.background_medium);
    set_color("border_color", "Panel", colors.panel_border);

    // ===== ЦВЕТА КНОПОК =====
    set_color("font_color", "Button", colors.text_primary);
    set_color("font_hover_color", "Button", colors.secondary);
    set_color("font_pressed_color", "Button", colors.secondary.lightened(0.2));
    set_color("font_disabled_color", "Button", colors.text_disabled);

    // ===== ЦВЕТА ГАЛОЧЕК (CHECKBOX) =====
    set_color("font_color", "CheckBox", colors.text_primary);
    set_color("font_hover_color", "CheckBox", colors.secondary);
    set_color("font_pressed_color", "CheckBox", colors.secondary.lightened(0.2));

    // Цвет самой галочки
    set_color("check_vbox", "CheckBox", colors.checkbox_checked);

    // ===== ЦВЕТА РАДИОКНОПОК =====
    set_color("font_color", "CheckButton", colors.text_primary);
    set_color("font_hover_color", "CheckButton", colors.secondary);
    set_color("font_pressed_color", "CheckButton", colors.secondary.lightened(0.2));

    // ===== ЦВЕТА ДЛЯ ИКОНОК В ДЕРЕВЕ (TREE) =====
    set_color("icon_color", "Tree", colors.icon_normal);
    set_color("icon_hover_color", "Tree", colors.icon_hover);
    set_color("icon_selected_color", "Tree", colors.icon_pressed);

    // ===== ЦВЕТА ДЛЯ ИКОНОК В СПИСКЕ (ITEM LIST) =====
    set_color("icon_color", "ItemList", colors.icon_normal);
    set_color("icon_hover_color", "ItemList", colors.icon_hover);
    set_color("icon_selected_color", "ItemList", colors.icon_pressed);

    // ===== ЦВЕТА ДЛЯ ИКОНОК В МЕНЮ =====
    set_color("icon_color", "PopupMenu", colors.icon_normal);
    set_color("icon_hover_color", "PopupMenu", colors.icon_hover);
    set_color("icon_disabled_color", "PopupMenu", colors.icon_disabled);

    // ===== ЦВЕТА ДЛЯ ИКОНОК НА ПАНЕЛЯХ ИНСТРУМЕНТОВ =====
    set_color("icon_color", "ToolButton", colors.icon_normal);
    set_color("icon_hover_color", "ToolButton", colors.icon_hover);
    set_color("icon_pressed_color", "ToolButton", colors.icon_pressed);
    set_color("icon_disabled_color", "ToolButton", colors.icon_disabled);

    // ===== ЦВЕТА ДЛЯ ИКОНОК ВО ВКЛАДКАХ =====
    set_color("icon_color", "TabContainer", colors.icon_normal);
    set_color("icon_selected_color", "TabContainer", colors.icon_hover);

    // ===== ЦВЕТА ВЫДЕЛЕНИЯ =====
    set_color("selection_color", "Editor", colors.secondary * Color(1, 1, 1, 0.3));
    set_color("selection_color", "Tree", colors.secondary * Color(1, 1, 1, 0.3));
    set_color("selection_color", "ItemList", colors.secondary * Color(1, 1, 1, 0.3));

    // ===== ЦВЕТА ПОЛОС ПРОКРУТКИ =====
    set_color("scroll_color", "ScrollBar", colors.secondary * Color(1, 1, 1, 0.5));
    set_color("scroll_hovered_color", "ScrollBar", colors.secondary);

    // ===== ЦВЕТА ДЛЯ ПРОГРЕСС БАРОВ =====
    set_color("progress_color", "ProgressBar", colors.secondary);

    // ===== ЦВЕТА ДЛЯ СЛАЙДЕРОВ =====
    set_color("slider_color", "HSlider", colors.secondary);
    set_color("slider_color", "VSlider", colors.secondary);

    // ===== ЦВЕТА ДЛЯ СПИННЕРОВ =====
    set_color("spinner_color", "SpinBox", colors.secondary);

    // ===== ЦВЕТА ДЛЯ РЕДАКТОРА ТЕКСТА =====
    set_color("background_color", "ScriptTextEditor", colors.background_dark);
    set_color("text_color", "ScriptTextEditor", colors.text_primary);
    set_color("line_number_color", "ScriptTextEditor", colors.text_secondary * Color(1, 1, 1, 0.5));

    // ===== ЦВЕТА ПОДСВЕТКИ СИНТАКСИСА =====
    set_color("keyword_color", "ScriptTextEditor", colors.syntax_keyword);
    set_color("function_color", "ScriptTextEditor", colors.syntax_function);
    set_color("comment_color", "ScriptTextEditor", colors.syntax_comment);
    set_color("string_color", "ScriptTextEditor", colors.syntax_string);
    set_color("number_color", "ScriptTextEditor", colors.syntax_number);

    // ===== 3D РЕДАКТОР =====
    set_color("primary_grid_color", "SpatialEditor", Color(0.56, 0.56, 0.56, 0.3));
    set_color("secondary_grid_color", "SpatialEditor", Color(0.38, 0.38, 0.38, 0.2));
    set_color("selection_box_color", "SpatialEditor", colors.secondary);
    set_color("gizmo_color", "SpatialEditor", colors.secondary);

    // ===== 2D РЕДАКТОР =====
    set_color("grid_color", "CanvasItemEditor", Color(1.0, 1.0, 1.0, 0.07));
    set_color("guides_color", "CanvasItemEditor", colors.secondary);
    set_color("selection_rect_color", "CanvasItemEditor", colors.secondary);

    print_line("[REAL ENGINE]: Theme applied with green icons and dark blue background");

    // ===== ПЕРЕОПРЕДЕЛЯЕМ ИКОНКИ =====
    /*
    // Переопределяем иконку галочки
    Ref<Texture2D> check_icon = create_custom_check_icon(colors.checkbox_checked);
    set_icon("checked", "CheckBox", check_icon);
    set_icon("checked_disabled", "CheckBox", check_icon);

    // Переопределяем иконку радио
    Ref<Texture2D> radio_icon = create_custom_radio_icon(colors.secondary);
    set_icon("checked", "CheckButton", radio_icon);
    */
}

void RealEngineTheme::update_colors_from_settings() {
    EditorSettings *settings = EditorSettings::get_singleton();
    if (!settings) return;

    // Загружаем цвета из настроек, если они там есть
    if (settings->has_setting("real_engine/theme/primary_color")) {
        colors.primary = settings->get("real_engine/theme/primary_color");
    }
    if (settings->has_setting("real_engine/theme/secondary_color")) {
        colors.secondary = settings->get("real_engine/theme/secondary_color");
    }
    if (settings->has_setting("real_engine/theme/background_dark")) {
        colors.background_dark = settings->get("real_engine/theme/background_dark");
    }
    if (settings->has_setting("real_engine/theme/background_medium")) {
        colors.background_medium = settings->get("real_engine/theme/background_medium");
    }
    if (settings->has_setting("real_engine/theme/background_light")) {
        colors.background_light = settings->get("real_engine/theme/background_light");
    }

    // Обновляем цвета иконок
    colors.icon_normal = colors.secondary;
    colors.icon_hover = colors.secondary.lightened(0.2);
    colors.icon_pressed = colors.secondary.darkened(0.2);
    colors.icon_disabled = Color(colors.secondary.r * 0.5, colors.secondary.g * 0.5, colors.secondary.b * 0.5, 0.5);

    colors.checkbox_checked = colors.secondary;
    colors.checkbox_hover = colors.secondary.lightened(0.2);

    print_line("[REAL ENGINE]: Theme colors updated from settings");
}

// Вспомогательная функция для создания кастомной иконки галочки (опционально)
/*
Ref<ImageTexture> RealEngineTheme::create_custom_check_icon(Color p_color) {
    Ref<Image> img;
    img.instantiate();
    img->create(16, 16, false, Image::FORMAT_RGBA8);
    img->fill(Color(0, 0, 0, 0));

    // Рисуем галочку
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if ((i > 3 && i < 13) && (j > 3 && j < 13)) {
                if (i == j || i == 15 - j) {
                    img->set_pixel(i, j, p_color);
                }
            }
        }
    }

    Ref<ImageTexture> texture;
    texture.instantiate();
    texture->set_image(img);
    return texture;
}
*/
