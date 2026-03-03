#include "project_timer.h"
#include "core/string/print_string.h"
#include "core/io/dir_access.h"

ProjectTimer *ProjectTimer::singleton = nullptr;

void ProjectTimer::_bind_methods() {
}

ProjectTimer::ProjectTimer() {
    singleton = this;

    String data_dir = OS::get_singleton()->get_environment("APPDATA");
    if (data_dir.is_empty()) {
        data_dir = OS::get_singleton()->get_user_data_dir();
    }

    String real_dir = data_dir + "/RLEngine/app_userdata/project_times"; // Насильно задаём путь
    DirAccess::make_dir_recursive_absolute(real_dir);

    file_path = real_dir + "/times.json"; // Название
    load();
}

ProjectTimer::~ProjectTimer() {
    if (current_project != "") {
        project_closed();
    }
    if (singleton == this) {
        singleton = nullptr;
    }
}

void ProjectTimer::load() {
    if (!FileAccess::exists(file_path)) return;

    Ref<FileAccess> f = FileAccess::open(file_path, FileAccess::READ);
    if (f.is_null()) return;

    JSON json;
    Error err = json.parse(f->get_as_text());
    if (err != OK) return;

    Dictionary data = json.get_data();
    Array keys = data.keys();
    for (int i = 0; i < keys.size(); i++) {
        String path = keys[i];
        projects_time[path] = data[path];
    }
}

void ProjectTimer::save() {

    for (KeyValue<String, uint64_t> E : projects_time) {
    }

    Dictionary data;
    for (KeyValue<String, uint64_t> E : projects_time) {
        data[E.key] = E.value;
    }

    Ref<FileAccess> f = FileAccess::open(file_path, FileAccess::WRITE);
    if (f.is_null()) {
        return;
    }

    String json = JSON::stringify(data);
    f->store_string(json);
    f->close();
}

void ProjectTimer::project_opened(const String &p_path) {
    current_project = p_path;
    session_start = OS::get_singleton()->get_ticks_msec() / 1000;

    if (!projects_time.has(p_path)) {
        projects_time[p_path] = 0;
    }
}

void ProjectTimer::project_closed() {
    print_line("[REAL ENGINE]: Stopped timer...");
    if (current_project == "") return;

    update();
    print_line("[REAL ENGINE]: Save...");
    save();
    current_project = "";
}

void ProjectTimer::update() {
    if (current_project.is_empty()) {
        return;
    }

    uint64_t now = OS::get_singleton()->get_ticks_msec() / 1000;
    uint64_t diff = now - session_start;

    if (diff > 0) {
        projects_time[current_project] += diff;
        session_start = now;
    }
}

void ProjectTimer::force_save() {
    if (current_project != "") {
        update();
    }
    save();
}

String ProjectTimer::get_time(const String &p_path) {
    uint64_t total = projects_time.has(p_path) ? projects_time[p_path] : 0;

    if (p_path == current_project) {
        uint64_t now = OS::get_singleton()->get_ticks_msec() / 1000;
        total += (now - session_start);
    }

    uint64_t days = total / 86400;
    uint64_t hours = (total % 86400) / 3600;
    uint64_t minutes = (total % 3600) / 60;
    uint64_t seconds = total % 60;

    char buf[16];
    snprintf(buf, sizeof(buf), "%02llu.%02llu.%02llu.%02llu", days, hours, minutes, seconds);
    return String(buf);
}
