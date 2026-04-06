/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "scene/gui/dialogs.h"
#include "core/os/os.h"
#include "core/os/thread.h"
#include "core/os/mutex.h"

// Защита от Windows макросов
#ifdef CreateDialog
#undef CreateDialog
#endif

#ifdef Font
#undef Font
#endif

class SystemMonitor : public RefCounted {
    GDCLASS(SystemMonitor, RefCounted)

    struct SystemStats {
        float cpu_usage = 0.0f;
        uint64_t ram_available = 0;
        uint64_t ram_total = 0;
        float disk_usage = 0.0f;
        float temperature = 0.0f;
        uint64_t last_update = 0;
        int unstable_count = 0;
    };

    SystemStats current_stats;
    SystemStats last_stats;

    bool monitoring_active = false;
    bool unstable_detected = false;
    Mutex *stats_mutex = nullptr;
    Thread *monitor_thread = nullptr;

    static SystemMonitor *singleton;

    // Константы
    static constexpr float CPU_THRESHOLD = 95.0f;
    static constexpr uint64_t RAM_THRESHOLD = 100 * 1024 * 1024; // 100 MB
    static constexpr int UNSTABLE_COUNT_THRESHOLD = 5;
    static constexpr int CHECK_INTERVAL_MS = 1000;

    bool is_initialized = false;
    bool initialization_attempted = false;

protected:
    static void _bind_methods();

public:
    static SystemMonitor *get_singleton();
    static SystemMonitor *create_singleton();

    SystemMonitor();
    ~SystemMonitor();

    bool initialize();
    void start_monitoring();
    void stop_monitoring();

    // Публичные методы
    float get_cpu_usage();
    uint64_t get_available_ram();
    uint64_t get_total_ram();
    float get_disk_usage();
    bool is_system_unstable();

    void check_system_stability();
    void show_unstable_warning(Control *p_parent, const String &p_reason);
    void reset_unstable_counter();

#ifdef WINDOWS_ENABLED
    // Windows-специфичные методы
    float _get_cpu_usage();
    uint64_t _get_available_ram();
    float _get_disk_usage();
    float _get_temperature();
#endif

    static void _thread_func(void *p_userdata);

private:
    void _update_stats();
    bool _is_system_unstable();
};

#endif // SYSTEM_MONITOR_H
