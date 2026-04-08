#include "real_sticker_item.h"
#include "scene/resources/style_box_flat.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"

void RealStickerItem::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_color"), &RealStickerItem::set_color);
}

RealStickerItem::RealStickerItem() {
    set_custom_minimum_size(Size2(0, 100) * EDSCALE);

    VBoxContainer *vbox = memnew(VBoxContainer);
    add_child(vbox);

    content_edit = memnew(TextEdit);
    content_edit->set_placeholder("Write something...");
    content_edit->set_editable(true);
    content_edit->set_h_size_flags(SIZE_EXPAND_FILL);
    content_edit->set_custom_minimum_size(Size2(150, 60) * EDSCALE);
    content_edit->connect("text_changed", callable_mp(this, &RealStickerItem::_on_edit_pressed));
    vbox->add_child(content_edit);

    HBoxContainer *hbox = memnew(HBoxContainer);
    vbox->add_child(hbox);

    color_button = memnew(Button);
    color_button->set_text("Color");
    color_button->connect("pressed", callable_mp(this, &RealStickerItem::_on_color_pressed));
    hbox->add_child(color_button);

    delete_button = memnew(Button);
    delete_button->set_text("Delete");
    delete_button->connect("pressed", callable_mp(this, &RealStickerItem::_on_delete_pressed));
    hbox->add_child(delete_button);

    // Создаём всплывающее окно для выбора цвета
    color_popup = memnew(Popup);
    color_popup->set_title("Pick Color");
    add_child(color_popup);

    color_picker = memnew(ColorPicker);
    color_picker->set_pick_color(Color(0.8, 0.8, 0.9));
    color_picker->connect("color_changed", callable_mp(this, &RealStickerItem::_on_color_changed));
    color_popup->add_child(color_picker);

    sticker_color = Color(0.8, 0.8, 0.9);
    _update_style();
}

RealStickerItem::~RealStickerItem() {
    // Очистка (не обязательна)
}

void RealStickerItem::set_text(const String &p_text) {
    content_edit->set_text(p_text);
}

void RealStickerItem::set_color(const Color &p_color) {
    sticker_color = p_color;
    color_picker->set_pick_color(p_color);
    _update_style();
}

void RealStickerItem::_update_style() {
    Ref<StyleBoxFlat> style;
    style.instantiate();
    style->set_bg_color(sticker_color);
    style->set_border_width_all(1);
    style->set_border_color(sticker_color.darkened(0.3));
    style->set_corner_radius_all(6);
    add_theme_style_override("panel", style);
}

void RealStickerItem::_on_edit_pressed() {
    EditorNode::get_singleton()->get_editor_data().save_editor_external_data();
}

void RealStickerItem::_on_color_pressed() {
    // Показываем попап в центре мыши
    Vector2 mouse_pos = DisplayServer::get_singleton()->mouse_get_position();
    color_popup->set_position(mouse_pos);
    color_popup->popup();
}

void RealStickerItem::_on_color_changed(const Color &p_color) {
    set_color(p_color);
}

void RealStickerItem::_on_delete_pressed() {
    queue_free();
}

Dictionary RealStickerItem::serialize() const {
    Dictionary d;
    d["text"] = content_edit->get_text();
    d["color"] = sticker_color;
    return d;
}

void RealStickerItem::deserialize(const Dictionary &p_data) {
    set_text(p_data["text"]);
    set_color(p_data["color"]);
}
