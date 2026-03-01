#include "system_monitor.h"
#include "editor/editor_node.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/check_box.h"
#include "scene/gui/texture_rect.h"
#include "scene/gui/separator.h"
#include "scene/gui/label.h"
#include "core/string/print_string.h"
#include "core/object/class_db.h"

#ifdef WINDOWS_ENABLED
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <pdh.h>
#include <psapi.h>
#endif

SystemMonitor *SystemMonitor::singleton = nullptr;

// Автоматическая регистрация класса
static bool registered = false;
static void register_system_monitor_class() {
    if (!registered) {
        ClassDB::register_class<SystemMonitor>();
        registered = true;
    }
}

void SystemMonitor::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_cpu_usage"), &SystemMonitor::get_cpu_usage);
    ClassDB::bind_method(D_METHOD("get_available_ram"), &SystemMonitor::get_available_ram);
    ClassDB::bind_method(D_METHOD("get_total_ram"), &SystemMonitor::get_total_ram);
    ClassDB::bind_method(D_METHOD("get_disk_usage"), &SystemMonitor::get_disk_usage);
    ClassDB::bind_method(D_METHOD("is_system_unstable"), &SystemMonitor::is_system_unstable);
    ClassDB::bind_method(D_METHOD("start_monitoring"), &SystemMonitor::start_monitoring);
    ClassDB::bind_method(D_METHOD("stop_monitoring"), &SystemMonitor::stop_monitoring);
    ClassDB::bind_method(D_METHOD("check_system_stability"), &SystemMonitor::check_system_stability);
    ClassDB::bind_method(D_METHOD("reset_unstable_counter"), &SystemMonitor::reset_unstable_counter);
}

SystemMonitor *SystemMonitor::create_singleton() {
    register_system_monitor_class();

    if (!singleton) {
        singleton = memnew(SystemMonitor);
        if (singleton) {
            singleton->initialize();
        }
    }
    return singleton;
}

SystemMonitor *SystemMonitor::get_singleton() {
    return singleton;
}

SystemMonitor::SystemMonitor() {
    monitoring_active = false;
    unstable_detected = false;
    stats_mutex = nullptr;
    monitor_thread = nullptr;
    is_initialized = false;
    initialization_attempted = false;

    current_stats = SystemStats();
    last_stats = SystemStats();
}

bool SystemMonitor::initialize() {
    if (is_initialized) return true;
    if (initialization_attempted) return false;

    initialization_attempted = true;

    if (!OS::get_singleton()) {
        return false;
    }

    stats_mutex = memnew(Mutex);
    if (!stats_mutex) {
        return false;
    }

    is_initialized = true;
    return true;
}

SystemMonitor::~SystemMonitor() {
    stop_monitoring();

    if (stats_mutex) {
        // Убеждаемся, что мутекс не используется, а то будет происходить крах с сигналом 11
        stats_mutex->lock();
        stats_mutex->unlock();
        memdelete(stats_mutex);
        stats_mutex = nullptr;
    }

    if (singleton == this) {
        singleton = nullptr;
    }
}

#ifdef WINDOWS_ENABLED
float SystemMonitor::_get_cpu_usage() {
    static PDH_HQUERY cpuQuery = NULL;
    static PDH_HCOUNTER cpuTotal = NULL;
    static bool initialized = false;
    static bool init_failed = false;

    if (init_failed) return 0.0f;

    if (!initialized) {
        PDH_STATUS status;

        status = PdhOpenQueryA(NULL, 0, &cpuQuery);
        if (status == ERROR_SUCCESS) {
            status = PdhAddCounterA(cpuQuery, "\\Processor(_Total)\\% Processor Time", 0, &cpuTotal);
            if (status == ERROR_SUCCESS) {
                PdhCollectQueryData(cpuQuery);
                initialized = true;
            } else {
                init_failed = true;
                PdhCloseQuery(cpuQuery);
                cpuQuery = NULL;
                return 0.0f;
            }
        } else {
            init_failed = true;
            return 0.0f;
        }
    }

    if (!initialized || !cpuQuery || !cpuTotal) return 0.0f;

    PDH_FMT_COUNTERVALUE counterVal;
    if (PdhCollectQueryData(cpuQuery) == ERROR_SUCCESS) {
        if (PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS) {
            return (float)counterVal.doubleValue;
        }
    }

    return 0.0f;
}

uint64_t SystemMonitor::_get_available_ram() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullAvailPhys;
    }
    return 0;
}

float SystemMonitor::_get_disk_usage() {
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        uint64_t total = totalNumberOfBytes.QuadPart;
        uint64_t free = totalNumberOfFreeBytes.QuadPart;
        uint64_t used = total - free;

        if (total > 0) {
            return (float)((used * 100.0) / total);
        }
    }

    return 0.0f;
}

float SystemMonitor::_get_temperature() {
    return 0.0f;
}
#endif

void SystemMonitor::_update_stats() {
    if (!is_initialized || !stats_mutex) return;

#ifdef WINDOWS_ENABLED
    stats_mutex->lock();

    last_stats = current_stats;

    current_stats.cpu_usage = _get_cpu_usage();
    current_stats.ram_available = _get_available_ram();
    current_stats.ram_total = OS::get_singleton()->get_static_memory_usage();
    current_stats.disk_usage = _get_disk_usage();
    current_stats.temperature = _get_temperature();
    current_stats.last_update = OS::get_singleton()->get_ticks_usec() / 1000;

    if (current_stats.cpu_usage > CPU_THRESHOLD) {
        current_stats.unstable_count++;
    } else if (current_stats.ram_available < RAM_THRESHOLD) {
        current_stats.unstable_count++;
    } else {
        if (current_stats.unstable_count > 0) {
            current_stats.unstable_count--;
        }
    }

    stats_mutex->unlock();
#endif
}

bool SystemMonitor::_is_system_unstable() {
    if (!is_initialized || !stats_mutex) return false;

    stats_mutex->lock();
    bool high_cpu = current_stats.cpu_usage > CPU_THRESHOLD;
    bool low_ram = current_stats.ram_available < RAM_THRESHOLD;
    bool count_exceeded = current_stats.unstable_count >= UNSTABLE_COUNT_THRESHOLD;
    stats_mutex->unlock();

    return (high_cpu || low_ram) && count_exceeded;
}

float SystemMonitor::get_cpu_usage() {
    if (!is_initialized || !stats_mutex) return 0.0f;

    stats_mutex->lock();
    float value = current_stats.cpu_usage;
    stats_mutex->unlock();

    return value;
}

uint64_t SystemMonitor::get_available_ram() {
    if (!is_initialized || !stats_mutex) return 0;

    stats_mutex->lock();
    uint64_t value = current_stats.ram_available;
    stats_mutex->unlock();

    return value;
}

uint64_t SystemMonitor::get_total_ram() {
    if (!is_initialized || !stats_mutex) return 0;

    stats_mutex->lock();
    uint64_t value = current_stats.ram_total;
    stats_mutex->unlock();

    return value;
}

float SystemMonitor::get_disk_usage() {
    if (!is_initialized || !stats_mutex) return 0.0f;

    stats_mutex->lock();
    float value = current_stats.disk_usage;
    stats_mutex->unlock();

    return value;
}

bool SystemMonitor::is_system_unstable() {
    if (!is_initialized || !stats_mutex) return false;

    stats_mutex->lock();
    bool high_cpu = current_stats.cpu_usage > CPU_THRESHOLD;
    bool low_ram = current_stats.ram_available < RAM_THRESHOLD;
    bool count_exceeded = current_stats.unstable_count >= UNSTABLE_COUNT_THRESHOLD;
    stats_mutex->unlock();

    return (high_cpu || low_ram) && count_exceeded;
}

void SystemMonitor::start_monitoring() {
    if (!is_initialized) {
        return;
    }

    if (monitoring_active) return;

    monitoring_active = true;
    unstable_detected = false;

    monitor_thread = memnew(Thread);
    if (monitor_thread) {
        monitor_thread->start(_thread_func, this);
    }
}

void SystemMonitor::stop_monitoring() {
    if (!monitoring_active) return;

    monitoring_active = false;

    if (monitor_thread) {
        monitor_thread->wait_to_finish();
        memdelete(monitor_thread);
        monitor_thread = nullptr;
    }
}

void SystemMonitor::reset_unstable_counter() {
    if (!is_initialized || !stats_mutex) return;

    stats_mutex->lock();
    current_stats.unstable_count = 0;
    unstable_detected = false;
    stats_mutex->unlock();
}

void SystemMonitor::check_system_stability() {
    if (!is_initialized || !unstable_detected) return;

    stats_mutex->lock();
    SystemStats stats_copy = current_stats;
    stats_mutex->unlock();

    EditorNode *editor = EditorNode::get_singleton();
    if (editor && editor->get_gui_base() && !editor->get_gui_base()->is_queued_for_deletion()) {
        String reason;

        if (stats_copy.cpu_usage > CPU_THRESHOLD) {
            reason = "High CPU usage: " + String::num(stats_copy.cpu_usage, 1) + "%";
        } else if (stats_copy.ram_available < RAM_THRESHOLD) {
            float ram_mb = stats_copy.ram_available / (1024.0f * 1024.0f);
            reason = "Low memory available: " + String::num(ram_mb, 1) + " MB";
        } else {
            reason = "System instability detected";
        }

        show_unstable_warning(editor->get_gui_base(), reason);
    }
}

void SystemMonitor::show_unstable_warning(Control *p_parent, const String &p_reason) {
    if (!is_initialized || !p_parent || p_parent->is_queued_for_deletion()) return;

    AcceptDialog *dialog = memnew(AcceptDialog);
    if (!dialog) return;

    dialog->set_title("⚠ Unstable System Operation Detected");
    dialog->set_min_size(Size2(500, 300) * EDSCALE);

    VBoxContainer *vb = memnew(VBoxContainer);
    dialog->add_child(vb);

    HBoxContainer *hb = memnew(HBoxContainer);
    vb->add_child(hb);

    TextureRect *icon = memnew(TextureRect);
    EditorNode *editor = EditorNode::get_singleton();
    if (editor && editor->get_gui_base()) {
        Ref<Texture2D> warning_icon = editor->get_gui_base()->get_theme_icon("Warning", "EditorIcons");
        if (warning_icon.is_valid()) {
            icon->set_texture(warning_icon);
        }
    }
    icon->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);
    hb->add_child(icon);

    Label *title = memnew(Label);
    title->set_text("Real Engine detected unstable system operation");
    title->set_theme_type_variation("HeaderSmall");
    hb->add_child(title);

    vb->add_child(memnew(HSeparator));

    RichTextLabel *info = memnew(RichTextLabel);
    info->set_use_bbcode(true);
    info->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    vb->add_child(info);

    stats_mutex->lock();
    SystemStats stats_copy = current_stats;
    stats_mutex->unlock();

    String message = "[b]Reason:[/b] " + p_reason + "\n\n";
    message += "[b]Current system status:[/b]\n";
    message += "• CPU Usage: [color=" + String(stats_copy.cpu_usage > 90 ? "red" : "yellow") + "]" +
               String::num(stats_copy.cpu_usage, 1) + "%[/color]\n";

    float ram_mb = stats_copy.ram_available / (1024.0f * 1024.0f);
    message += "• Available RAM: [color=" + String(ram_mb < 200 ? "red" : "yellow") + "]" +
               String::num(ram_mb, 1) + " MB[/color]\n";

    message += "\n[b]Recommendations:[/b]\n";
    message += "• Save your work immediately\n";
    message += "• Close other applications\n";
    message += "• Consider restarting Real Engine\n";

    info->append_text(message);

    CheckBox *dont_show = memnew(CheckBox);
    dont_show->set_text("Don't show this warning again this session");
    vb->add_child(dont_show);

    dialog->connect("confirmed", Callable(dialog, "queue_free"), CONNECT_DEFERRED);
    dialog->popup_centered();

    reset_unstable_counter();
}

void SystemMonitor::_thread_func(void *p_userdata) {
    SystemMonitor *self = (SystemMonitor *)p_userdata;

    if (!self || !self->is_initialized) return;

    while (self->monitoring_active && self->is_initialized) {
        self->_update_stats();

        if (self->_is_system_unstable() && !self->unstable_detected) {
            self->unstable_detected = true;
            if (EditorNode::get_singleton() && EditorNode::get_singleton()->get_gui_base()) {
                self->call_deferred("check_system_stability");
            }
        }

        OS::get_singleton()->delay_usec(CHECK_INTERVAL_MS * 1000);
    }
}
