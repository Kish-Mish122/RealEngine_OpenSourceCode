/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "settings_migrator.h"
#include "editor/settings/editor_settings.h"
#include "core/io/resource_loader.h"
#include "core/string/print_string.h"
#include "core/version.h"
#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "core/io/dir_access.h"

SettingsMigrator *SettingsMigrator::singleton = nullptr;

void SettingsMigrator::_bind_methods() {
}

SettingsMigrator::SettingsMigrator() {
    singleton = this;
}

SettingsMigrator::~SettingsMigrator() {
    if (singleton == this) {
        singleton = nullptr;
    }
}

int SettingsMigrator::get_current_version_major() const {
    return GODOT_VERSION_MAJOR;
}

int SettingsMigrator::get_current_version_minor() const {
    return GODOT_VERSION_MINOR;
}

Vector<SettingsMigrator::OldSettingsFile> SettingsMigrator::find_old_settings() {
    Vector<OldSettingsFile> old_files;

    String config_dir;
    if (OS::get_singleton()->has_environment("APPDATA")) {
        config_dir = OS::get_singleton()->get_environment("APPDATA") + "/RLEngine";
    } else {
        config_dir = OS::get_singleton()->get_user_data_dir() + "/RLEngine";
    }

    Ref<DirAccess> da = DirAccess::open(config_dir);
    if (da.is_null()) return old_files;

    da->list_dir_begin();
    String file = da->get_next();

    while (!file.is_empty()) {
        if (file.begins_with("editor_settings-") && file.ends_with(".tres")) {
            String version_str = file.replace("editor_settings-", "").replace(".tres", "");

            OldSettingsFile osf;
            osf.path = config_dir.path_join(file);
            osf.version = version_str;

            Vector<String> parts = version_str.split(".");
            if (parts.size() >= 2) {
                osf.version_major = parts[0].to_int();
                osf.version_minor = parts[1].to_int();
            }

            osf.modified_time = FileAccess::get_modified_time(osf.path);

            // Не включаем текущую версию
            if (version_str != vformat("%d.%d", get_current_version_major(), get_current_version_minor())) {
                old_files.push_back(osf);
            }
        }
        file = da->get_next();
    }

    da->list_dir_end();

    // Сортировка по дате (новые первыми)
    for (int i = 0; i < old_files.size(); i++) {
        for (int j = i + 1; j < old_files.size(); j++) {
            if (old_files[i].modified_time < old_files[j].modified_time) {
                OldSettingsFile temp = old_files[i];
                old_files.write[i] = old_files[j];
                old_files.write[j] = temp;
            }
        }
    }

    return old_files;
}

void SettingsMigrator::migrate_editor_settings(const String &p_old_path) {
    print_line("[REAL MIGRATOR]: Migrating ALL editor settings from: " + p_old_path);

    // Загружаем старый файл как ресурс
    Ref<Resource> old_resource = ResourceLoader::load(p_old_path);
    if (old_resource.is_null()) {
        print_line("[REAL MIGRATOR]: Failed to load old settings (invalid .tres file)");
        return;
    }

    EditorSettings *es = EditorSettings::get_singleton();
    if (!es) {
        print_line("[REAL MIGRATOR]: EditorSettings singleton not available");
        return;
    }

    // Получаем список всех свойств старого ресурса
    List<PropertyInfo> props;
    old_resource->get_property_list(&props);

    int migrated_count = 0;
    for (const PropertyInfo &pi : props) {
        String name = pi.name;
        // Пропускаем служебные свойства ресурса
        if (name.begins_with("resource_") || name == "__meta") {
            continue;
        }

        Variant value = old_resource->get(name);
        // Если в текущих настройках такого ключа нет - переносим
        if (!es->has_setting(name)) {
            es->set_setting(name, value);
            migrated_count++;
            print_line("[REAL MIGRATOR]: Migrated setting: " + name);
        }
    }

    // Сохраняем новые настройки
    es->save();
}

void SettingsMigrator::migrate_project_times() {
    // Сохраняем старый метод (он рабочий, но использует JSON)
    String data_dir;
    if (OS::get_singleton()->has_environment("APPDATA")) {
        data_dir = OS::get_singleton()->get_environment("APPDATA");
    } else {
        data_dir = OS::get_singleton()->get_user_data_dir();
    }

    String new_times_path = data_dir + "/RLEngine/app_userdata/project_times/times.json";
    DirAccess::make_dir_recursive_absolute(data_dir + "/RLEngine/app_userdata/project_times");

    Ref<DirAccess> da = DirAccess::open(data_dir);
    if (da.is_null()) return;

    da->list_dir_begin();
    String file = da->get_next();

    Dictionary all_times;
    if (FileAccess::exists(new_times_path)) {
        Ref<FileAccess> f = FileAccess::open(new_times_path, FileAccess::READ);
        if (f.is_valid()) {
            JSON json;
            json.parse(f->get_as_text());
            all_times = json.get_data();
        }
    }

    while (!file.is_empty()) {
        if (file.begins_with("project_times_") && file.ends_with(".json")) {
            String old_path = data_dir + "/" + file;

            Ref<FileAccess> f_old = FileAccess::open(old_path, FileAccess::READ);
            if (f_old.is_valid()) {
                JSON json;
                json.parse(f_old->get_as_text());
                Dictionary old_data = json.get_data();

                Array keys = old_data.keys();
                for (int i = 0; i < keys.size(); i++) {
                    String key = keys[i];
                    if (!all_times.has(key)) {
                        all_times[key] = old_data[key];
                    }
                }
            }
        }
        file = da->get_next();
    }

    Ref<FileAccess> f_save = FileAccess::open(new_times_path, FileAccess::WRITE);
    if (f_save.is_valid()) {
        f_save->store_string(JSON::stringify(all_times));
    }
}

void SettingsMigrator::migrate_from_version(const String &p_old_path) {
    migrate_editor_settings(p_old_path);
    migrate_project_times();
}

void SettingsMigrator::cleanup_old_settings(bool p_keep_latest) {
    Vector<OldSettingsFile> old_files = find_old_settings();

    if (p_keep_latest && old_files.size() > 0) {
        old_files.remove_at(0);
    }

    for (const OldSettingsFile &osf : old_files) {
        if (FileAccess::exists(osf.path)) {
            DirAccess::remove_file_or_error(osf.path);
        }
    }
}

void SettingsMigrator::migrate_all() {
    print_line("[REAL MIGRATOR]: Starting migration...");

    Vector<OldSettingsFile> old_files = find_old_settings();

    if (old_files.size() == 0) {
        return;
    }

    OldSettingsFile latest = old_files[0];

    migrate_from_version(latest.path);

    cleanup_old_settings(true); // false — удалить все найденные старые настройки
}
