#include "project_timer.h"
#include "core/string/print_string.h"
#include "core/io/dir_access.h"
#include "core/string/ustring.h"
#include "core/string/string_builder.h"
#include "core/os/time.h"

ProjectTimer *ProjectTimer::singleton = nullptr;

void ProjectTimer::_bind_methods() {
}

ProjectTimer::ProjectTimer() {
    singleton = this;

    String data_dir = OS::get_singleton()->get_environment("APPDATA");
    if (data_dir.is_empty()) {
        data_dir = OS::get_singleton()->get_user_data_dir();
    }

    String real_dir = data_dir + "/RLEngine/app_userdata/project_times"; // Насильно задаём путь, чтобы был путь C:/Users/NAME/AppData/Roaming/RLEngine
    DirAccess::make_dir_recursive_absolute(real_dir);

    file_path = real_dir + "/times.json"; // Название файла
    load();
}

ProjectTimer::~ProjectTimer() {
    if (!current_project.is_empty()) {
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
        Dictionary proj_data = data[path];

        ProjectData pd;
        pd.total_seconds = proj_data.get("seconds", 0);
        pd.last_modified = proj_data.get("modified", 0);
        pd.last_opened = proj_data.get("opened", 0);

        projects_data[path] = pd;
    }
}

void ProjectTimer::save() {
    Dictionary data;
    for (KeyValue<String, ProjectData> E : projects_data) {
        Dictionary proj_dict;
        proj_dict["seconds"] = E.value.total_seconds;
        proj_dict["modified"] = E.value.last_modified;
        proj_dict["opened"] = E.value.last_opened;
        data[E.key] = proj_dict;
    }

    Ref<FileAccess> f = FileAccess::open(file_path, FileAccess::WRITE);
    if (f.is_null()) return;

    f->store_string(JSON::stringify(data));
}

void ProjectTimer::project_opened(const String &p_path) {
    print_line("[REAL ENGINE]: Opening project: " + p_path);

    if (!current_project.is_empty()) {
        project_closed();
    }

    current_project = p_path;
    session_start = OS::get_singleton()->get_ticks_msec() / 1000;

    // Обновляем время последнего открытия
    if (projects_data.has(p_path)) {
        projects_data[p_path].last_opened = OS::get_singleton()->get_unix_time();
    } else {
        ProjectData pd;
        pd.total_seconds = 0;
        pd.last_modified = 0;
        pd.last_opened = OS::get_singleton()->get_unix_time();
        projects_data[p_path] = pd;
    }

    save();
}

// Функция которая следит, чтобы если пользователь закрыл проект
void ProjectTimer::project_closed() {
    if (current_project.is_empty()) return; // Если нет информации о проекте, от возращаем

    // Если иначе то...
    update(); // Обнавляем
    save(); // Сохраняем
    current_project = ""; // String текущего проекта
}

void ProjectTimer::update() {
    if (current_project.is_empty()) return;

    uint64_t now = OS::get_singleton()->get_ticks_msec() / 1000;
    uint64_t diff = now - session_start;

    if (diff > 0) {
        projects_data[current_project].total_seconds += diff;
        session_start = now;
    }
}

// Принудительное сохранение - вроде не нужная функция, но лучше не удалять, а то вдруг нужная...
void ProjectTimer::force_save() {
    save();
}

String ProjectTimer::format_seconds(uint64_t p_seconds) {
    uint64_t days = p_seconds / 86400;
    uint64_t hours = (p_seconds % 86400) / 3600;
    uint64_t minutes = (p_seconds % 3600) / 60;
    uint64_t seconds = p_seconds % 60;

    char buf[16];
    snprintf(buf, sizeof(buf), "%02llu.%02llu.%02llu.%02llu", days, hours, minutes, seconds);
    return String(buf);
}

String ProjectTimer::get_time(const String &p_path) {
    if (!projects_data.has(p_path)) {
        return "00.00.00.00";
    }

    uint64_t total = projects_data[p_path].total_seconds;

    if (p_path == current_project) {
        uint64_t now = OS::get_singleton()->get_ticks_msec() / 1000;
        total += (now - session_start);
    }

    return format_seconds(total);
}

String ProjectTimer::format_timestamp(uint64_t p_timestamp) {
    if (p_timestamp == 0) {
        return "Not known"; // Если не известна дата последнего изменения, то возращаем "Не известно"
    }

    // Создаём экземпляр Time
    Time *time = Time::get_singleton();
    if (!time) {
        return "The file is broken!"; // Если файл сломан или он не читается, то пишем "Файл сломан!"
    }

    Dictionary datetime = time->get_datetime_dict_from_unix_time(p_timestamp);

    int day = datetime["day"];
    int month = datetime["month"];
    int year = datetime["year"];
    int hour = datetime["hour"];
    int minute = datetime["minute"];

    return vformat("%02d.%02d.%04d %02d:%02d", day, month, year, hour, minute);
}

String ProjectTimer::get_last_modified(const String &p_path) {
    if (!projects_data.has(p_path)) {
        return "Not known"; // Если не известна дата последнего изменения, то возращаем "Не известно"
    }

    return format_timestamp(projects_data[p_path].last_modified);
}

String ProjectTimer::get_last_opened(const String &p_path) {
    if (!projects_data.has(p_path)) {
        return "Not known"; // Если не известна дата последнего изменения, то возращаем "Не известно"
    }

    return format_timestamp(projects_data[p_path].last_opened);
}

void ProjectTimer::update_last_modified(const String &p_path) {
    if (projects_data.has(p_path)) {
        projects_data[p_path].last_modified = OS::get_singleton()->get_unix_time();
        save();
    }
}
