// This file is part of the Godot Orchestrator project.
//
// Copyright (c) 2023-present Vahera Studios LLC and its contributors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//		http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef ORCHESTRATOR_SCRIPT_NODE_H
#define ORCHESTRATOR_SCRIPT_NODE_H

#include "context/execution_context.h"
#include "instances/node_instance.h"
#include "language.h"
#include "node_pin.h"
#include "script.h"

#include <optional>

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;

/// Forward declarations
class OScript;
class OScriptInstance;

/// A context object used to initialize OScriptNode instances.
///
/// The context provides either a reference to a MethodInfo or PropertyInfo combined with details
/// about a target class type, variable name, or custom data attributes.
///
struct OScriptNodeInitContext
{
    // Method details
    std::optional<MethodInfo> method;

    // Various property types
    std::optional<PropertyInfo> property;
    std::optional<NodePath> node_path;

    std::optional<StringName> class_name;

    std::optional<String> variable_name;

    std::optional<String> resource_path;

    std::optional<Dictionary> user_data;
};

/// The base class for all script node resources used in an Orchestration.
///
/// An Orchestration is a collection of script nodes that allow the user to create visual script like
/// implementations of code. All script nodes that are possible derive from this base class.
///
class OScriptNode : public Resource
{
    friend class OScript;

    ORCHESTRATOR_NODE_CLASS_BASE(OScriptNode, Resource);

protected:
    /// Godot bindings
    static void _bind_methods();

public:
    /// Flags for script nodes
    enum ScriptNodeFlags
    {
        NONE = 1 << 0,              //! No specific flags
        CATALOGABLE = 1 << 1,       //! Node should appear in the action catalog
        DEVELOPMENT_ONLY = 1 << 2,  //! Node should be marked in the UI as development only
        EXPERIMENTAL = 1 << 3       //! Node is experimental and may change
    };

protected:
    bool _initialized{ false };         //! Manages whether the node is initialized
    int _id{ -1 };                      //! Unique node id, assigned by the owning script
    Vector2 _size;                      //! Size of the node
    Vector2 _position;                  //! Position of the node
    BitField<ScriptNodeFlags> _flags;   //! Flags
    Vector<Ref<OScriptNodePin>> _pins;  //! Pins
    OScript* _script{ nullptr };        //! Owning script
    bool _reconstructing{ false };      //! Tracks if the node is in reconstruction

private:

    // Serialization for pins
    // Dictionaries are used to minimize the sub-resource footprint in the script file.
    TypedArray<Dictionary> _get_pin_data() const;
    void _set_pin_data(const TypedArray<Dictionary>& p_pin_data);

protected:
    // Registration
    static void register_custom_orchestrator_data_to_otdb() { }

    static bool _is_in_editor();

public:
    OScriptNode();

    /// Get the owning Orchestrator script
    /// @return the orchestrator script
    OScript* get_owning_script() const { return _script; }

    /// Set the script that owns this node
    /// @param p_script the owning script
    void set_owning_script(OScript* p_script);

    /// Get the node's unique identifier
    /// @return the node's unique identifer
    int get_id() const { return _id; }

    /// Set the node's unique id
    /// @param p_id the node's unique identifier
    void set_id(int p_id);

    /// Get the node's size
    /// @return the node's size as a Vector2
    Vector2 get_size() const { return _size; }

    /// Sets the node's size
    /// @param p_size the node's size, width and height
    void set_size(const Vector2& p_size);

    /// Get the node's position on the graph canvas.
    /// @return the node's position coordinates
    Vector2 get_position() const { return _position; }

    /// Set the node's position
    /// @param p_position the node's position coordinates
    void set_position(const Vector2& p_position);

    /// Get the node's flags.
    /// @return flags, defaults to none.
    virtual BitField<ScriptNodeFlags> get_flags() const { return _flags; }

    /// Set the node's flags
    /// @param p_flags the flags
    virtual void set_flags(BitField<ScriptNodeFlags> p_flags);

    /// Get the node's top left icon to be shown.
    /// @return the icon, defaults to Object
    virtual String get_icon() const { return "Object"; }

    /// Get the node's tooltip text when the user hovers the node.
    /// @return the tooltip-text
    virtual String get_tooltip_text() const { return {}; }

    /// Get keywords that should also be matched when performing action lookups
    /// @return keywords that are additional matches beyond the node name
    virtual PackedStringArray get_keywords() const { return {}; }

    /// Get all node-specific actions that will be appended to the node context menu.
    /// @param p_action_list the list of actions to append actions
    virtual void get_actions(List<Ref<OScriptAction>>& p_action_list);

    /// Callback to perform operations before node is saved.
    virtual void pre_save();

    /// Callback to perform operations after node is saved.
    virtual void post_save() { }

    /// Callback to perform operations before node is removed.
    virtual void pre_remove();

    /// Callback after the node has been loaded and initialized by Godot.
    virtual void post_initialize();

    /// Allocates the node's default pins
    /// @details This is called when the node is created and after its loaded
    virtual void allocate_default_pins() { }

    /// Handle pin reallocation based on previous pin state.
    /// @param p_old_pins the old pin state
    /// @details The default behavior is to call "allocate_default_pins".
    virtual void reallocate_pins_during_reconstruction(const Vector<Ref<OScriptNodePin>>& p_old_pins);

    virtual void rewire_old_pins_to_new_pins(const Vector<Ref<OScriptNodePin>>& p_old_pins,
                                             const Vector<Ref<OScriptNodePin>>& p_new_pins);

    /// Recreates the node from its internal state.
    virtual void reconstruct_node();

    /// Callback after the node has been reconstructed.
    virtual void post_reconstruct_node() { }

    /// Specifies whether pin types can be changed
    /// @return true if the node's pin context-menu should allow changing type, false otherwise.
    virtual bool can_change_pin_type() { return false; }

    /// Get the possible pin type names for pins.
    /// @return string array of possible pin types that can be changed to
    virtual Vector<Variant::Type> get_possible_pin_types() const { return {}; }

    /// Changes the pin types for this node.
    /// @param p_type the new pin type
    virtual void change_pin_types(Variant::Type p_type) { }

    /// Get whether the user can delete this node.
    /// @return true if the node can be deleted, false otherwise
    virtual bool can_user_delete_node() const { return true; }

    /// Get the node's title bar color name, looked up from project settings
    /// @return the title bar color name, defaults to empty to use standard theme color
    virtual String get_node_title_color_name() const { return {}; }

    /// Get the node's title text
    /// @return the title bar text, the default is the node name.
    virtual String get_node_title() const { return get_class(); }

    /// Get the node's compact title text
    /// @return the title bar text, the default is the node name.
    virtual String get_compact_node_title() const { return get_class(); };

    /// Callback to perform operations after the node has been pasted.
    virtual void post_paste_node() { }

    /// Callback to perform operations after the node has been created.
    virtual void post_placed_new_node();

    /// Callback to perform operations after the node has been autowired.
    /// @param p_other the other, existing node this node was autowired with
    /// @param p_direction the autowire direction
    virtual void post_node_autowired(const Ref<OScriptNode>& p_other, EPinDirection p_direction) { }

    /// Whether to draw the node as an entry node
    /// @return true if its to be drawn as an entry node, false otherwise
    virtual bool draw_node_as_entry() const { return false; }

    /// Whether to draw the node as an exist node
    /// @return true if its an exit node, false otehrwise
    virtual bool draw_node_as_exit() const { return false; }

    /// Whether to draw the node compact.
    /// @return true if its to be drawn compact, false otherwise
    virtual bool should_draw_compact() const { return false; }

    /// Whether to draw the node as a bead.
    /// @return true to draw the node as a bead, false otherwise.
    virtual bool should_draw_as_bead() const { return false; }

    /// Get the object to be inspected when clicking this node.
    /// @return the inspectable object, typically this, the node.
    virtual Ref<Resource> get_inspect_object() { return this; }

    /// Get whether the node's properties should be visible in the inspector dock.
    /// @return true if the inspector shows the node's properties, false otherwise.
    virtual bool can_inspect_node_properties() const { return true; }

    /// Returns whether this node is compatible with the given graph.
    /// @param p_graph the graph to check compatibility for
    /// @return true if the node can exist in the graph, false otherwise
    virtual bool is_compatible_with_graph(const Ref<OScriptGraph>& p_graph) const { return true; }

    /// Get the jump target object when the node is double-clicked.
    /// @return the object, may be null.
    virtual Object* get_jump_target_for_double_click() const { return nullptr; }

    /// Get whether the node acts as a jump to another graph or view.
    /// @return true if the node acts as a jump; false otherwise.
    virtual bool can_jump_to_definition() const { return false; }

    /// Callback when a pin associated with this node changes its default value.
    /// @param p_pin the pin that was changed
    virtual void pin_default_value_changed(const Ref<OScriptNodePin>& p_pin) { }

    /// Get whether a user-defined pin can be created.
    /// @param p_direction the pin direction, input or output.
    /// @param r_message the message to be shown if return value is false.
    /// @return true if the user-defined pin can be created, false otherwise
    virtual bool can_create_user_defined_pin(EPinDirection p_direction, String& r_message) { return false; }

    /// Callback to perform node validation during build step.
    /// @return true if the node is valid, false otherwise
    virtual bool validate_node_during_build() const { return true; }

    /// Instantiate the script node's runtime instance.
    /// @param p_instance the owning runtime script instance
    /// @return node's runtime instance
    virtual OScriptNodeInstance* instantiate(OScriptInstance* p_instance);

    /// Initializes the node from spawner data
    /// @param p_context the initialization context
    virtual void initialize(const OScriptNodeInitContext& p_context);

    /// Resolves the type class based on the specified pin, defaulting to Object.
    /// @param p_pin the pin
    /// @return the resolved class type
    virtual StringName resolve_type_class(const Ref<OScriptNodePin>& p_pin) const { return ""; }

    /// Resolves the target object of the specified pin
    /// @param p_pin the pin
    /// @return the resolved target object
    virtual Object* resolve_target(const Ref<OScriptNodePin>& p_pin) const { return nullptr; }

    /// Create a pin associated with this node.
    /// @param p_direction the pin direction, input or output
    /// @param p_name the pin name
    /// @param p_type the pin type, defaults to Variant::NIL
    /// @param p_default_value the default value, defaults to null
    /// @param p_index the slot index
    /// @return the newly created pin reference
    Ref<OScriptNodePin> create_pin(EPinDirection p_direction, const String& p_name, Variant::Type p_type = Variant::NIL,
                                   const Variant& p_default_value = Variant(), int p_index = -1);

    /// Find the specified pin.
    /// @param p_pin_name the pin name to locate
    /// @param p_direction the pin's direction, using the default searches both inputs and outputs.
    /// @return the pin reference if found or an invalid reference if the pin is not found
    Ref<OScriptNodePin> find_pin(const String& p_pin_name, EPinDirection p_direction = PD_MAX) const;

    /// Find a pin by slot index and direction.
    /// @param p_index the slot index
    /// @param p_direction the pin's direction, should not be PD_MAX
    /// @return the pin reference if found or an invalid reference if the pin is not found
    Ref<OScriptNodePin> find_pin(int p_index, EPinDirection p_direction) const;

    /// Find all lines for a given direction.
    /// @param p_direction the pin direction, defaults to PD_MAX which returns all pins
    /// @return vector of pin references for this node
    Vector<Ref<OScriptNodePin>> find_pins(EPinDirection p_direction = PD_MAX);

    /// Removes the specified pin from this node
    /// @param p_pin the pin to be removed
    bool remove_pin(const Ref<OScriptNodePin>& p_pin);

    /// Get an immutable collection of all node pins.
    /// @return an immutable collection of node pins
    const Vector<Ref<OScriptNodePin>>& get_all_pins() const { return _pins; }

    /// Check whether the node has any connections
    /// @return true if at least one pin is connected, false otherwise
    bool has_any_connections() const;

    /// Called when a pin connection is made.
    /// @param p_pin the pin that was connected
    virtual void on_pin_connected(const Ref<OScriptNodePin>& p_pin);

    /// Called when a pin disconnection is made.
    /// @param p_pin the pin that was disconnected
    virtual void on_pin_disconnected(const Ref<OScriptNodePin>& p_pin);

protected:

    /// Notify that node pins have been changed.
    void _notify_pins_changed();

    /// Validate the input default values for this node
    void _validate_input_default_values();

    // needs to be protected
    // this allows nodes that use the editable pin contract to recalculate indices
    void _cache_pin_indices();

};

VARIANT_BITFIELD_CAST(OScriptNode::ScriptNodeFlags);

#define DECLARE_SCRIPT_NODE_INSTANCE(x) /*************************************/ \
    friend class x;                                                             \
    x* _node = nullptr;                                                         \
    OScriptInstance* _instance = nullptr;

#endif  // ORCHESTRATOR_SCRIPT_NODE_H