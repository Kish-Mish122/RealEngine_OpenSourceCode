#include "real_stickers_plugin.h"
#include "real_stickers_dock.h"
#include "editor/editor_node.h"

void RealStickersPlugin::_bind_methods() {
}

RealStickersPlugin::RealStickersPlugin() {
}

RealStickersPlugin::~RealStickersPlugin() {
}

void RealStickersPlugin::_enter_tree() {
    dock = memnew(RealStickersDock);
    add_control_to_dock(EditorPlugin::DOCK_SLOT_LEFT_UL, dock);
}

void RealStickersPlugin::_exit_tree() {
    if (dock) {
        remove_control_from_docks(dock);
        dock->queue_free();
        dock = nullptr;
    }
}

String RealStickersPlugin::get_plugin_name() const {
    return "RealStickers";
}
