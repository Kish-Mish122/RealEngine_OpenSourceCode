/* Real Engine - K1sh-M1sh Studio */
/* Licnese - MIT */

#ifndef PROJECT_TIMER_H
#define PROJECT_TIMER_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/os/os.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/templates/hash_map.h"

class ProjectTimer : public RefCounted {
    GDCLASS(ProjectTimer, RefCounted);

    struct ProjectData {
        uint64_t total_seconds = 0;
        uint64_t last_modified = 0;
        uint64_t last_opened = 0;
    };

    HashMap<String, ProjectData> projects_data;
    String current_project;
    uint64_t session_start = 0;
    String file_path;

    static ProjectTimer *singleton;

protected:
    static void _bind_methods();

public:
    static ProjectTimer *get_singleton() { return singleton; }

    ProjectTimer();
    ~ProjectTimer();

    void load();
    void save();

    void project_opened(const String &p_path);
    void project_closed();
    void update();
    void force_save();

    String get_time(const String &p_path);
    String get_last_modified(const String &p_path);
    String get_last_opened(const String &p_path);
    void update_last_modified(const String &p_path);

    String format_seconds(uint64_t p_seconds);
    String format_timestamp(uint64_t p_timestamp);
};

#endif // PROJECT_TIMER_H
