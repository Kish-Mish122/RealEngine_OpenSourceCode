#include "system_status_bar.h"
#include "core/os/os.h"
#include "main/performance.h"
#include "editor/themes/editor_scale.h"
#include "scene/resources/style_box_flat.h"

void SystemStatusBar::_bind_methods() {
}

SystemStatusBar::SystemStatusBar() {
    set_name("SystemStatusBar");
    set_custom_minimum_size(Size2(0, 28) * EDSCALE);
    set_v_size_flags(0);

    Ref<StyleBoxFlat> style;
    style.instantiate();
    style->set_bg_color(Color(0, 0, 0, 0.9));
    add_theme_style_override("panel", style);

    HBoxContainer *info = memnew(HBoxContainer);
    info->set_alignment(BoxContainer::ALIGNMENT_CENTER);
    info->add_theme_constant_override("separation", 20 * EDSCALE);
    add_child(info);

    fps_label = memnew(Label);
    ram_label = memnew(Label);
    fps_label->add_theme_color_override("font_color", Color(1, 1, 1));
    ram_label->add_theme_color_override("font_color", Color(1, 1, 1));
    info->add_child(fps_label);
    info->add_child(ram_label);

    update_timer = memnew(Timer);
    update_timer->set_wait_time(1.0);
    update_timer->connect("timeout", callable_mp(this, &SystemStatusBar::_update_stats));
    add_child(update_timer);
}

void SystemStatusBar::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        update_timer->start();
        _update_stats();
    } else if (p_what == NOTIFICATION_EXIT_TREE) {
        update_timer->stop();
    }
}

void SystemStatusBar::_update_stats() {
    float fps = Engine::get_singleton()->get_frames_per_second();
    fps_label->set_text("FPS: " + String::num(fps, 1));

    uint64_t available_ram = OS::get_singleton()->get_memory_info()["available"];
    int ram_mb = available_ram / (1024 * 1024);
    ram_label->set_text("RAM Free: " + itos(ram_mb) + " MB");
}
