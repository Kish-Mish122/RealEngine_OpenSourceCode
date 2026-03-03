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

    HashMap<String, uint64_t> projects_time;
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
};

#endif // PROJECT_TIMER_H
