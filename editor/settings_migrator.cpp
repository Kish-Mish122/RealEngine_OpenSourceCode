/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "settings_migrator.h"
#include "core/string/print_string.h"
#include "core/version.h"
#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "core/io/dir_access.h"
#include "core/io/config_file.h"

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

    // Путь к папке с настройками
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
                print_line("[Migrator] Found old settings: " + osf.path + " (v" + osf.version + ")");
            }
        }
        file = da->get_next();
    }

    da->list_dir_end();

    // Ручная сортировка по дате (новые первыми)
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
    print_line("[Migrator] Migrating editor settings from: " + p_old_path);

    // Правильное создание Ref<ConfigFile>
    Ref<ConfigFile> old_settings;
    old_settings.instantiate();
    Error err = old_settings->load(p_old_path);

    if (err != OK) {
        print_line("[Migrator] Failed to load old settings");
        return;
    }

    // Новый файл настроек
    String new_config_dir;
    if (OS::get_singleton()->has_environment("APPDATA")) {
        new_config_dir = OS::get_singleton()->get_environment("APPDATA") + "/RLEngine";
    } else {
        new_config_dir = OS::get_singleton()->get_user_data_dir() + "/RLEngine";
    }

    String new_path = new_config_dir.path_join(vformat("editor_settings-%d.%d.tres",
        get_current_version_major(), get_current_version_minor()));

    Ref<ConfigFile> new_settings;
    new_settings.instantiate();

    Vector<String> sections = old_settings->get_sections();
    if (!sections.has("init")) {
        print_line("[Migrator] Old settings have no 'init' section, skipping");
        return;
    }

    // Загружаем текущие настройки, если есть
    if (FileAccess::exists(new_path)) {
        new_settings->load(new_path);
    }

    print_line("[REAL MIGRATOR]: Loading the list of all important settings...");
    // Список важных настроек для переноса
    Vector<String> important_settings = {
        "github/token",
        "gitlab/token",
        "gitlab/url",
        "project_timer/show_in_project_list",
        "project_timer/show_last_modified",
        "filesystem/directories/default_project_path",
        "interface/theme/icon_and_font_color",
        "interface/theme/base_color",
        "interface/theme/accent_color",
        "text_editor/theme/highlighting/symbol_color",
	     "text_editor/theme/highlighting/keyword_color",
	     "text_editor/theme/highlighting/control_flow_keyword_color",
		  "text_editor/theme/highlighting/base_type_color",
		  "text_editor/theme/highlighting/engine_type_color",
		  "text_editor/theme/highlighting/user_type_color",
		  "text_editor/theme/highlighting/comment_color",
		  "text_editor/theme/highlighting/doc_comment_color",
		  "text_editor/theme/highlighting/string_color",
		  "text_editor/theme/highlighting/string_placeholder_color",
		  "text_editor/theme/highlighting/background_color",
		  "text_editor/theme/highlighting/text_color",
		  "text_editor/theme/highlighting/line_number_color",
		  "text_editor/theme/highlighting/safe_line_number_color",
		  "text_editor/theme/highlighting/caret_color",
		  "text_editor/theme/highlighting/caret_background_color",
		  "text_editor/theme/highlighting/text_selected_color",
		  "text_editor/theme/highlighting/selection_color",
		  "text_editor/theme/highlighting/brace_mismatch_color",
		  "text_editor/theme/highlighting/current_line_color",
		  "text_editor/theme/highlighting/line_length_guideline_color",
		  "text_editor/theme/highlighting/word_highlighted_color",
		  "text_editor/theme/highlighting/number_color",
		  "text_editor/theme/highlighting/function_color",
		  "text_editor/theme/highlighting/member_variable_color",
		  "text_editor/theme/highlighting/mark_color",
        "real_coffee/enable",
        "real_coffee/interval",
        "interface/editor/autosave_enabled",
        "interface/editor/autosave_interval",
        "interface/editor/autosave_notification",
        "real_coffee/show_notifications",
        "editor/driver_warning_disabled",
        "application/check_ram_on_startup",
        "filesystem/import/dont_ask_large_files"
    };

    Vector<String> keys = old_settings->get_section_keys("init");

    for (int i = 0; i < keys.size(); i++) {
        const String &key = keys[i];
        if (important_settings.has(key) || !new_settings->has_section_key("init", key)) {
            Variant value = old_settings->get_value("init", key);
            new_settings->set_value("init", key, value);
            print_line("[Migrator] Migrated: " + key);
        }
    }

    // Сохраняем
    new_settings->save(new_path);
    print_line("[Migrator] Editor settings migration complete");
}

void SettingsMigrator::migrate_project_times() {
    String data_dir;
    if (OS::get_singleton()->has_environment("APPDATA")) {
        data_dir = OS::get_singleton()->get_environment("APPDATA");
    } else {
        data_dir = OS::get_singleton()->get_user_data_dir();
    }

    String new_times_path = data_dir + "/RLEngine/app_userdata/project_times/times.json";

    // Создаём папку если нет
    DirAccess::make_dir_recursive_absolute(data_dir + "/RLEngine/app_userdata/project_times");

    // Ищем старые файлы
    Ref<DirAccess> da = DirAccess::open(data_dir);
    if (da.is_null()) return;

    da->list_dir_begin();
    String file = da->get_next();

    Dictionary all_times;

    // Загружаем текущие, если есть
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
            print_line("[Migrator] Found old project times: " + old_path);

            Ref<FileAccess> f_old = FileAccess::open(old_path, FileAccess::READ);
            if (f_old.is_valid()) {
                JSON json;
                json.parse(f_old->get_as_text());
                Dictionary old_data = json.get_data();

                // Объединяем
                Array keys = old_data.keys();
                for (int i = 0; i < keys.size(); i++) {
                    String key = keys[i];
                    if (!all_times.has(key)) {
                        all_times[key] = old_data[key];
                        print_line("[Migrator] Migrated project time: " + key);
                    }
                }
            }
        }
        file = da->get_next();
    }

    // Сохраняем объединённые данные
    Ref<FileAccess> f_save = FileAccess::open(new_times_path, FileAccess::WRITE);
    if (f_save.is_valid()) {
        f_save->store_string(JSON::stringify(all_times));
        print_line("[Migrator] Project times migration complete");
    }
}

void SettingsMigrator::migrate_from_version(const String &p_old_path) {
    migrate_editor_settings(p_old_path);
    migrate_project_times();
}

void SettingsMigrator::cleanup_old_settings(bool p_keep_latest) {
    Vector<OldSettingsFile> old_files = find_old_settings();

    if (p_keep_latest && old_files.size() > 0) {
        // Оставляем самую новую старую версию (на всякий случай)
        old_files.remove_at(0);
    }

    for (const OldSettingsFile &osf : old_files) {
        if (FileAccess::exists(osf.path)) {
            DirAccess::remove_file_or_error(osf.path);
            print_line("[Migrator] Removed old settings: " + osf.path);
        }
    }
}

void SettingsMigrator::migrate_all() {
    print_line("[Migrator] Starting migration...");

    Vector<OldSettingsFile> old_files = find_old_settings();

    if (old_files.size() == 0) {
        print_line("[Migrator] No old settings found");
        return;
    }

    OldSettingsFile latest = old_files[0];
    print_line("[Migrator] Migrating from version: " + latest.version);

    migrate_from_version(latest.path);

    cleanup_old_settings(false); // true = оставить самую новую старую версию

    print_line("[Migrator] Migration complete");
}
/*
Для чего нужен мигратор настроек:
Изначально, в Godot нет функции импортирования всех настроек в новую версию, ну или как минимум, я не замечал.
Решил я сделать мигратор собственными ручками, которые и так уже устали, но всё же.
Для чего он нужен:
1. Импортирование старых настроек в новую версию - нахождение старой версии editor_settings-** и импортирование в новую версию editor_settings
2. Удаление или наоборот сохранение самой новой старой версии - после импоритрования, вы можете либо оставить, либо удалить самую новую старую версию editor_settings.
Где же это найти:
Строчка: 293 есть комментарий чтобы вы уж точно поняли
*/
