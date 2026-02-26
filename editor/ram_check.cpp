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

void RAMCheck::_bind_methods() {}

RAMCheck::RAMCheck() {
    singleton = this;
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
    print_line("[REAL CHECK RAM]: total RAM: " + itos(total) + " MB");
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
    print_line("[REAL CHECK RAM]: available RAM: " + itos(available) + " MB");
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

    dialog->connect(SceneStringName(confirmed), Callable(dialog, "queue_free"));

    if (p_parent) {
        p_parent->add_child(dialog);
    }

    dialog->call_deferred("popup_centered");
}

void RAMCheck::check_ram_at_startup(Control *p_parent) {
    show_ram_warning(p_parent);
}
