/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#pragma once

#include "core/object/ref_counted.h"
#include "scene/main/node.h"

class RealMemory : public Node {
    GDCLASS(RealMemory, Node);

private:
    struct Action {
        String type;           // "create_node", "set_property", "duplicate"
        String target;         // "Node3D", "Sprite2D", etc.
        Dictionary properties;  // параметры действия
        uint64_t timestamp;     // когда сделано
        int frequency;          // как часто делается
    };

    Vector<Action> action_history;
    HashMap<String, Vector<Action>> patterns;
    
    void _detect_patterns();
    void _show_suggestion(const String &message, const Dictionary &action);

protected:
    static void _bind_methods();

public:
    RealMemory();
    ~RealMemory();

    void record_action(const String &p_type, const String &p_target, const Dictionary &p_props);
    void analyze_patterns();
    void apply_suggestion(const Dictionary &p_suggestion);
};
