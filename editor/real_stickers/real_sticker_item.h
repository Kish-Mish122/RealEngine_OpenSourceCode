#pragma once

#include "scene/gui/panel_container.h"
#include "scene/gui/button.h"
#include "scene/gui/text_edit.h"
#include "scene/gui/popup.h"
#include "scene/gui/color_picker.h"

class RealStickerItem : public PanelContainer {
    GDCLASS(RealStickerItem, PanelContainer);

private:
    TextEdit *content_edit;
    Button *color_button;
    Button *delete_button;
    Color sticker_color;

    Popup *color_popup;
    ColorPicker *color_picker;

    void _update_style();
    void _on_color_pressed();
    void _on_color_changed(const Color &p_color);
    void _on_delete_pressed();
    void _on_edit_pressed();

protected:
    static void _bind_methods();

public:
    RealStickerItem();
    ~RealStickerItem();

    void set_text(const String &p_text);
    void set_color(const Color &p_color);
    Dictionary serialize() const;
    void deserialize(const Dictionary &p_data);
};
