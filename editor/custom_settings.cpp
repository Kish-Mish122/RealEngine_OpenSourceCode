#include "custom_settings.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/os/os.h"

CustomSettings *CustomSettings::singleton = nullptr;

void CustomSettings::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_value", "key", "value"), &CustomSettings::set_value);
    ClassDB::bind_method(D_METHOD("get_value", "key", "default"), &CustomSettings::get_value, DEFVAL(Variant()));
}

CustomSettings::CustomSettings() {
    singleton = this;
    load();
}

CustomSettings::~CustomSettings() {
    singleton = nullptr;
}

void CustomSettings::set_value(const String &p_key, const Variant &p_value) {
    data[p_key] = p_value;
    save();
}

Variant CustomSettings::get_value(const String &p_key, const Variant &p_default) {
    if (data.has(p_key)) {
        return data[p_key];
    }
    return p_default;
}

void CustomSettings::save() {
    String path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("custom_settings.json");

    Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE);
    if (file.is_valid()) {
        file->store_line(JSON::stringify(data));
        file.unref();
        print_line("[CUSTOM SETTINGS] Saved");
    }
}

void CustomSettings::load() {
    String path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("custom_settings.json");

    if (!FileAccess::exists(path)) return;

    Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
    if (file.is_valid()) {
        String json_str = file->get_as_text();
        JSON json;
        json.parse(json_str);
        data = json.get_data();
        file.unref();
        print_line("[CUSTOM SETTINGS] Loaded");
    }
}
