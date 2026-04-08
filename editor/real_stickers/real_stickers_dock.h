#pragma once

#include "scene/gui/box_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/button.h"
#include "scene/main/timer.h"

class RealStickersDock : public VBoxContainer {
    GDCLASS(RealStickersDock, VBoxContainer);

private:
    ScrollContainer *scroll;
    VBoxContainer *stickers_container;
    Button *add_button;

    void _load_stickers();
    void _save_stickers();
    void _add_sticker();
    void _on_sticker_deleted(Node *node);

protected:
    static void _bind_methods();
    virtual void _notification(int p_what);

public:
    RealStickersDock();
    ~RealStickersDock();

    void add_sticker(const String &p_text, const Color &p_color);
    void remove_sticker(class RealStickerItem *item);
};
