#ifndef DISCORD_RPC_H
#define DISCORD_RPC_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/os/os.h"
#include "core/object/class_db.h"
#include "core/version.h"

class DiscordRPC : public RefCounted {
    GDCLASS(DiscordRPC, RefCounted);

    String application_id;
    String current_project;
    String current_scene;
    uint64_t start_time;
    bool enabled;
    bool discord_connected;

    static DiscordRPC *singleton;

protected:
    static void _bind_methods();

public:
    static DiscordRPC *get_singleton() { return singleton; }

    DiscordRPC();
    ~DiscordRPC();

    void initialize();
    void shutdown();
    void update_presence(const String &p_state, const String &p_details);
    void update_project(const String &p_project);
    void update_scene(const String &p_scene);
    void clear_presence();

    bool is_enabled() const { return enabled; }
    bool is_discord_connected() const { return discord_connected; }

    void _update_callback();
    void _delayed_init();
};

#endif // DISCORD_RPC_H
