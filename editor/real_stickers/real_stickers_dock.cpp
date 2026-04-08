#include "real_stickers_dock.h"
#include "real_sticker_item.h"
#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "core/io/config_file.h"

void RealStickersDock::_bind_methods() {
}

RealStickersDock::RealStickersDock() {
    set_name("Stickers");

    scroll = memnew(ScrollContainer);
    scroll->set_v_size_flags(SIZE_EXPAND_FILL);
    add_child(scroll);

    stickers_container = memnew(VBoxContainer);
    stickers_container->set_h_size_flags(SIZE_EXPAND_FILL);
    scroll->add_child(stickers_container);

    add_button = memnew(Button);
    add_button->set_text("+ Add Sticker");
    add_button->connect("pressed", callable_mp(this, &RealStickersDock::_add_sticker));
    add_child(add_button);

    _load_stickers();
}

RealStickersDock::~RealStickersDock() {
    _save_stickers();
}

void RealStickersDock::_notification(int p_what) {
    if (p_what == NOTIFICATION_PREDELETE) {
        _save_stickers();
    }
}

void RealStickersDock::_load_stickers() {
    Ref<ConfigFile> config;
    config.instantiate();
    String path = "res://project.rlengine";
    if (config->load(path) == OK) {
        if (config->has_section("stickers")) {
            Array stickers = config->get_value("stickers", "list", Array());
            for (int i = 0; i < stickers.size(); i++) {
                Dictionary sticker_data = stickers[i];   // переименовано
                String text = sticker_data["text"];
                Color color = sticker_data["color"];
                add_sticker(text, color);
            }
        }
    }
}

void RealStickersDock::_save_stickers() {
    Ref<ConfigFile> config;
    config.instantiate();
    String path = "res://project.rlengine";
    config->load(path);

    Array stickers_data;
    for (int i = 0; i < stickers_container->get_child_count(); i++) {
        RealStickerItem *item = Object::cast_to<RealStickerItem>(stickers_container->get_child(i));
        if (item) {
            stickers_data.append(item->serialize());
        }
    }
    config->set_value("stickers", "list", stickers_data);
    config->save(path);
}

void RealStickersDock::_add_sticker() {
    add_sticker("New sticker", Color(0.3, 0.6, 0.9));
}

void RealStickersDock::add_sticker(const String &p_text, const Color &p_color) {
    RealStickerItem *item = memnew(RealStickerItem);
    item->set_text(p_text);
    item->set_color(p_color);
    item->connect("tree_exited", callable_mp(this, &RealStickersDock::_on_sticker_deleted).bind(item));
    stickers_container->add_child(item);
    _save_stickers();
}

void RealStickersDock::remove_sticker(RealStickerItem *item) {
    if (item) {
        item->queue_free();
        _save_stickers();
    }
}

void RealStickersDock::_on_sticker_deleted(Node *node) {
    _save_stickers();
}
