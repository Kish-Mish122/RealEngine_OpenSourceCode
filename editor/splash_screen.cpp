#include "splash_screen.h"
#include "scene/gui/panel.h"
#include "scene/gui/box_container.h"
#include "scene/resources/style_box_flat.h"
#include "scene/main/window.h"

void SplashScreen::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY: {
            Window *win = get_window();
            if (win) {loading_label->set_text("Loading...");

                win->set_flag(Window::FLAG_BORDERLESS, true);
                win->set_flag(Window::FLAG_RESIZE_DISABLED, true);
                win->set_size(Size2(600, 400));
            }
            set_anchors_and_offsets_preset(PRESET_FULL_RECT);
        } break;

        case NOTIFICATION_PROCESS: {
            time_elapsed += get_process_delta_time();
            if (can_close_flag && time_elapsed >= min_display_time) {
                get_tree()->quit();
            }
        } break;
    }
}

void SplashScreen::_bind_methods() {}

SplashScreen::SplashScreen() {
    set_process(true);

    Panel *bg = memnew(Panel);
    add_child(bg);
    bg->set_anchors_and_offsets_preset(PRESET_FULL_RECT);

    Ref<StyleBoxFlat> style = memnew(StyleBoxFlat);
    style->set_bg_color(Color(0.1, 0.1, 0.1));
    bg->add_theme_style_override("panel", style);

    VBoxContainer *main_vb = memnew(VBoxContainer);
    add_child(main_vb);
    main_vb->set_anchors_and_offsets_preset(PRESET_FULL_RECT);

    VBoxContainer *center = memnew(VBoxContainer);
    main_vb->add_child(center);
    center->set_h_size_flags(SIZE_EXPAND_FILL);
    center->set_v_size_flags(SIZE_EXPAND_FILL);
    center->set_alignment(BoxContainer::ALIGNMENT_CENTER);

    logo = memnew(TextureRect);
    center->add_child(logo);
    logo->set_custom_minimum_size(Size2(200, 200));

    version_label = memnew(Label);
    center->add_child(version_label);
    version_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);

    loading_label = memnew(Label);
    center->add_child(loading_label);
    loading_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
}

void SplashScreen::set_logo(const Ref<Texture2D> &p_logo) {
    if (p_logo.is_valid()) logo->set_texture(p_logo);
}

void SplashScreen::set_version(const String &p_version) {
    version_label->set_text(p_version);
}

void SplashScreen::set_loading_text(const String &p_text) {
    loading_label->set_text(p_text);
}

void SplashScreen::set_progress(float p_progress) {}

void SplashScreen::close_splash() {
    can_close_flag = true;
}
