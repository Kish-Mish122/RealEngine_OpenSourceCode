/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#pragma once

#include "core/object/ref_counted.h"

class RealMemoryRecorder : public RefCounted {
    GDCLASS(RealMemoryRecorder, RefCounted);

private:
    struct RecordedAction {
        String node_type;
        String parent_path;
        Dictionary properties;
        uint64_t timestamp;
    };

    Vector<RecordedAction> session_actions;
    HashMap<String, int> action_count;

public:
    void record_node_creation(Node *p_node, Node *p_parent);
    void record_property_change(Object *p_obj, const String &p_property, const Variant &p_value);
    void record_duplicate(Node *p_original, Node *p_copy);
    Vector<Dictionary> get_frequent_patterns();
};
