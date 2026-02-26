#ifndef CUSTOM_SETTINGS_H
#define CUSTOM_SETTINGS_H

#include "core/object/ref_counted.h"
#include "core/io/json.h"

class CustomSettings : public RefCounted {
    GDCLASS(CustomSettings, RefCounted);

    static CustomSettings *singleton;
    Dictionary data;

protected:
    static void _bind_methods();

public:
    static CustomSettings *get_singleton() { return singleton; }

    void set_value(const String &p_key, const Variant &p_value);
    Variant get_value(const String &p_key, const Variant &p_default = Variant());
    void save();
    void load();

    CustomSettings();
    ~CustomSettings();
};

#endif // CUSTOM_SETTINGS_H
