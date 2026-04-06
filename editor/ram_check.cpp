/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "ram_check.h"
#include "core/os/os.h"
#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/progress_bar.h"
#include "core/string/print_string.h"

#ifdef WINDOWS_ENABLED
#include <windows.h>
#endif

RAMCheck *RAMCheck::singleton = nullptr;
bool RAMCheck::settings_initialized = false; // Статическая переменная

void RAMCheck::_init_settings() {
    if (settings_initialized) return;
    settings_initialized = true;

    Ref<EditorSettings> es = EditorSettings::get_singleton();
    if (!es->has_setting("application/check_ram_on_startup")) {
        es->set_setting("application/check_ram_on_startup", true);
        // Не нужно add_property_info для bool
        es->save(); // Сохраняем при первом запуске
    }
}

void RAMCheck::_bind_methods() {}

RAMCheck::RAMCheck() {
    singleton = this;
    _init_settings();
}

RAMCheck::~RAMCheck() {
    singleton = nullptr;
}

uint64_t RAMCheck::get_total_ram_mb() {
#ifdef WINDOWS_ENABLED
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    uint64_t total = status.ullTotalPhys / (1024 * 1024);
    print_line("[REAL CHECK RAM]: Total RAM: " + itos(total) + " MB");
    return total;
#else
    return 8192;
#endif
}

uint64_t RAMCheck::get_available_ram_mb() {
#ifdef WINDOWS_ENABLED
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    uint64_t available = status.ullAvailPhys / (1024 * 1024);
    print_line("[REAL CHECK RAM]: Available RAM: " + itos(available) + " MB");
    return available;
#else
    return 4096;
#endif
}

void RAMCheck::show_ram_warning(Control *p_parent) {
    uint64_t total = get_total_ram_mb();
    uint64_t available = get_available_ram_mb();

    if (total >= min_ram_mb && available >= min_ram_mb / 2) {
        return;
    }

    AcceptDialog *dialog = memnew(AcceptDialog);
    dialog->set_title("System Memory Warning");
    dialog->set_min_size(Size2(500, 300) * EDSCALE);
    dialog->set_exclusive(false);

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    RichTextLabel *info = memnew(RichTextLabel);
    info->set_use_bbcode(true);
    info->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    vb->add_child(info);

    String message;
    if (total < min_ram_mb) {
        message += "[b][color=red]CRITICAL:[/color][/b]\n";
        message += "Your system has less than the minimum required RAM (" + itos(min_ram_mb) + " MB).\n";
        message += "Real Engine may not run properly.\n\n";
    } else if (available < min_ram_mb / 2) {
        message += "[b][color=orange]WARNING:[/color][/b]\n";
        message += "Your system is low on available RAM.\n";
        message += "Please close other applications to free up memory.\n\n";
    }

    message += "[b]Current Memory Status:[/b]\n";
    message += "  • Total RAM: " + itos(total) + " MB\n";
    message += "  • Available: " + itos(available) + " MB\n";

    info->append_text(message);

    CheckBox *cb = memnew(CheckBox);
    cb->set_text("Do not show this warning again");
    vb->add_child(cb);

    dialog->set_meta("dont_show_checkbox", Variant(cb));

    dialog->connect("confirmed", callable_mp(this, &RAMCheck::_on_ram_warning_confirmed));

    if (p_parent) {
        p_parent->add_child(dialog);
    }

    dialog->call_deferred("popup_centered");
}

void RAMCheck::_on_ram_warning_confirmed(Object *p_button) {
    AcceptDialog *dialog = Object::cast_to<AcceptDialog>(p_button);
    if (!dialog) return;

    // Исправлено: передаём Variant(), а не nullptr
    Variant default_variant;
    CheckBox *cb = Object::cast_to<CheckBox>(dialog->get_meta("dont_show_checkbox", default_variant));
    if (cb && cb->is_pressed()) {
        EditorSettings::get_singleton()->set_setting("application/check_ram_on_startup", false);
        EditorSettings::get_singleton()->save();
    }

    dialog->queue_free();
}

void RAMCheck::check_ram_at_startup(Control *p_parent) {
    bool check_enabled = (bool)EDITOR_GET("application/check_ram_on_startup");
    if (!check_enabled) {
        return;
    }
    show_ram_warning(p_parent);
}
