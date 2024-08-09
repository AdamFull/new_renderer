module;

#include <imgui.h>

export module ingui;

// Scalar data types
export using ImGuiID;// A unique ID used by widgets (typically the result of hashing a stack of string)
export using ImS8;   // 8-bit signed integer
export using ImU8;   // 8-bit unsigned integer
export using ImS16;  // 16-bit signed integer
export using ImU16;  // 16-bit unsigned integer
export using ImS32;  // 32-bit signed integer == int
export using ImU32;  // 32-bit unsigned integer (often used to store packed colors)
export using ImS64;  // 64-bit signed integer
export using ImU64;  // 64-bit unsigned integer

// Forward declarations
export using ImDrawChannel;               // Temporary storage to output draw commands out of order, used by ImDrawListSplitter and ImDrawList::ChannelsSplit()
export using ImDrawCmd;                   // A single draw command within a parent ImDrawList (generally maps to 1 GPU draw call, unless it is a callback)
export using ImDrawData;                  // All draw command lists required to render the frame + pos/size coordinates to use for the projection matrix.
export using ImDrawList;                  // A single draw command list (generally one per window, conceptually you may see this as a dynamic "mesh" builder)
export using ImDrawListSharedData;        // Data shared among multiple draw lists (typically owned by parent ImGui context, but you may create one yourself)
export using ImDrawListSplitter;          // Helper to split a draw list into different layers which can be drawn into out of order, then flattened back.
export using ImDrawVert;                  // A single vertex (pos + uv + col = 20 bytes by default. Override layout with IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
export using ImFont;                      // Runtime data for a single font within a parent ImFontAtlas
export using ImFontAtlas;                 // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
export using ImFontBuilderIO;             // Opaque interface to a font builder (stb_truetype or FreeType).
export using ImFontConfig;                // Configuration data when adding a font or merging fonts
export using ImFontGlyph;                 // A single font glyph (code point + coordinates within in ImFontAtlas + offset)
export using ImFontGlyphRangesBuilder;    // Helper to build glyph ranges from text/string data
export using ImColor;                     // Helper functions to create a color that can be converted to either u32 or float4 (*OBSOLETE* please avoid using)
export using ImGuiContext;                // Dear ImGui context (opaque structure, unless including imgui_internal.h)
export using ImGuiIO;                     // Main configuration and I/O between your application and ImGui
export using ImGuiInputTextCallbackData;  // Shared state of InputText() when using custom ImGuiInputTextCallback (rare/advanced use)
export using ImGuiKeyData;                // Storage for ImGuiIO and IsKeyDown(), IsKeyPressed() etc functions.
export using ImGuiListClipper;            // Helper to manually clip large list of items
export using ImGuiMultiSelectIO;          // Structure to interact with a BeginMultiSelect()/EndMultiSelect() block
export using ImGuiOnceUponAFrame;         // Helper for running a block of code not more than once a frame
export using ImGuiPayload;                // User data payload for drag and drop operations
export using ImGuiPlatformIO;             // Multi-viewport support: interface for Platform/Renderer backends + viewports to render
export using ImGuiPlatformMonitor;        // Multi-viewport support: user-provided bounds for each connected monitor/display. Used when positioning popups and tooltips to avoid them straddling monitors
export using ImGuiPlatformImeData;        // Platform IME data for io.PlatformSetImeDataFn() function.
export using ImGuiSelectionBasicStorage;  // Optional helper to store multi-selection state + apply multi-selection requests.
export using ImGuiSelectionExternalStorage;//Optional helper to apply multi-selection requests to existing randomly accessible storage.
export using ImGuiSelectionRequest;       // A selection request (stored in ImGuiMultiSelectIO)
export using ImGuiSizeCallbackData;       // Callback data when using SetNextWindowSizeConstraints() (rare/advanced use)
export using ImGuiStorage;                // Helper for key->value storage (container sorted by key)
export using ImGuiStoragePair;            // Helper for key->value storage (pair)
export using ImGuiStyle;                  // Runtime data for styling/colors
export using ImGuiTableSortSpecs;         // Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
export using ImGuiTableColumnSortSpecs;   // Sorting specification for one column of a table
export using ImGuiTextBuffer;             // Helper to hold and append into a text buffer (~string builder)
export using ImGuiTextFilter;             // Helper to parse and apply text filters (e.g. "aaaaa[,bbbbb][,ccccc]")
export using ImGuiViewport;               // A Platform Window (always 1 unless multi-viewport are enabled. One per platform window to output to). In the future may represent Platform Monitor
export using ImGuiWindowClass;            // Window class (rare/advanced uses: provide hints to the platform backend via altered viewport flags and parent/child info)

// Enumerations
// - We don't use strongly typed enums much because they add constraints (can't extend in private code, can't store typed in bit fields, extra casting on iteration)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   - In Visual Studio: CTRL+comma ("Edit.GoToAll") can follow symbols inside comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
//   - In Visual Studio w/ Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//   - In VS Code, CLion, etc.: CTRL+click can follow symbols inside comments.
enum ImGuiDir : int;                // -> enum ImGuiDir              // Enum: A cardinal direction (Left, Right, Up, Down)
enum ImGuiKey : int;                // -> enum ImGuiKey              // Enum: A key identifier (ImGuiKey_XXX or ImGuiMod_XXX value)
enum ImGuiMouseSource : int;        // -> enum ImGuiMouseSource      // Enum; A mouse input source identifier (Mouse, TouchScreen, Pen)
enum ImGuiSortDirection : ImU8;     // -> enum ImGuiSortDirection    // Enum: A sorting direction (ascending or descending)
typedef int ImGuiCol;               // -> enum ImGuiCol_             // Enum: A color identifier for styling
typedef int ImGuiCond;              // -> enum ImGuiCond_            // Enum: A condition for many Set*() functions
typedef int ImGuiDataType;          // -> enum ImGuiDataType_        // Enum: A primary data type
typedef int ImGuiMouseButton;       // -> enum ImGuiMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
typedef int ImGuiMouseCursor;       // -> enum ImGuiMouseCursor_     // Enum: A mouse cursor shape
typedef int ImGuiStyleVar;          // -> enum ImGuiStyleVar_        // Enum: A variable identifier for styling
typedef int ImGuiTableBgTarget;     // -> enum ImGuiTableBgTarget_   // Enum: A color target for TableSetBgColor()

// Flags (declared as int to allow using as flags without overhead, and to not pollute the top of this file)
// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//   - In Visual Studio: CTRL+comma ("Edit.GoToAll") can follow symbols inside comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
//   - In Visual Studio w/ Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//   - In VS Code, CLion, etc.: CTRL+click can follow symbols inside comments.
export using ImDrawFlags;            // -> enum ImDrawFlags_          // Flags: for ImDrawList functions
export using ImDrawListFlags;        // -> enum ImDrawListFlags_      // Flags: for ImDrawList instance
export using ImFontAtlasFlags;       // -> enum ImFontAtlasFlags_     // Flags: for ImFontAtlas build
export using ImGuiBackendFlags;      // -> enum ImGuiBackendFlags_    // Flags: for io.BackendFlags
export using ImGuiButtonFlags;       // -> enum ImGuiButtonFlags_     // Flags: for InvisibleButton()
export using ImGuiChildFlags;        // -> enum ImGuiChildFlags_      // Flags: for BeginChild()
export using ImGuiColorEditFlags;    // -> enum ImGuiColorEditFlags_  // Flags: for ColorEdit4(), ColorPicker4() etc.
export using ImGuiConfigFlags;       // -> enum ImGuiConfigFlags_     // Flags: for io.ConfigFlags
export using ImGuiComboFlags;        // -> enum ImGuiComboFlags_      // Flags: for BeginCombo()
export using ImGuiDockNodeFlags;     // -> enum ImGuiDockNodeFlags_   // Flags: for DockSpace()
export using ImGuiDragDropFlags;     // -> enum ImGuiDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
export using ImGuiFocusedFlags;      // -> enum ImGuiFocusedFlags_    // Flags: for IsWindowFocused()
export using ImGuiHoveredFlags;      // -> enum ImGuiHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.
export using ImGuiInputFlags;        // -> enum ImGuiInputFlags_      // Flags: for Shortcut(), SetNextItemShortcut()
export using ImGuiInputTextFlags;    // -> enum ImGuiInputTextFlags_  // Flags: for InputText(), InputTextMultiline()
export using ImGuiItemFlags;         // -> enum ImGuiItemFlags_       // Flags: for PushItemFlag(), shared by all items
export using ImGuiKeyChord;          // -> ImGuiKey | ImGuiMod_XXX    // Flags: for IsKeyChordPressed(), Shortcut() etc. an ImGuiKey optionally OR-ed with one or more ImGuiMod_XXX values.
export using ImGuiPopupFlags;        // -> enum ImGuiPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
export using ImGuiMultiSelectFlags;  // -> enum ImGuiMultiSelectFlags_// Flags: for BeginMultiSelect()
export using ImGuiSelectableFlags;   // -> enum ImGuiSelectableFlags_ // Flags: for Selectable()
export using ImGuiSliderFlags;       // -> enum ImGuiSliderFlags_     // Flags: for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
export using ImGuiTabBarFlags;       // -> enum ImGuiTabBarFlags_     // Flags: for BeginTabBar()
export using ImGuiTabItemFlags;      // -> enum ImGuiTabItemFlags_    // Flags: for BeginTabItem()
export using ImGuiTableFlags;        // -> enum ImGuiTableFlags_      // Flags: For BeginTable()
export using ImGuiTableColumnFlags;  // -> enum ImGuiTableColumnFlags_// Flags: For TableSetupColumn()
export using ImGuiTableRowFlags;     // -> enum ImGuiTableRowFlags_   // Flags: For TableNextRow()
export using ImGuiTreeNodeFlags;     // -> enum ImGuiTreeNodeFlags_   // Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
export using ImGuiViewportFlags;     // -> enum ImGuiViewportFlags_   // Flags: for ImGuiViewport
export using ImGuiWindowFlags;       // -> enum ImGuiWindowFlags_     // Flags: for Begin(), BeginChild()

// ImTexture: user data for renderer backend to identify a texture [Compile-time configurable type]
// - To use something else than an opaque void* pointer: override with e.g. '#define ImTextureID MyTextureType*' in your imconfig.h file.
// - This can be whatever to you want it to be! read the FAQ about ImTextureID for details.
export using ImTextureID;          // Default: store a pointer or an integer fitting in a pointer (most renderer backends are ok with that)

// ImDrawIdx: vertex index. [Compile-time configurable type]
// - To use 16-bit indices + allow large meshes: backend need to set 'io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset' and handle ImDrawCmd::VtxOffset (recommended).
// - To use 32-bit indices: override with '#define ImDrawIdx unsigned int' in your imconfig.h file.
export using ImDrawIdx;   // Default: 16-bit (for maximum compatibility with renderer backends)

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded character used for keyboard input and display)
export using ImWchar32;     // A single decoded U32 character/code point. We encode them as multi bytes UTF-8 when used in strings.
export using ImWchar16;   // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
export using ImWchar;

// Multi-Selection item index or identifier when using BeginMultiSelect()
// - Used by SetNextItemSelectionUserData() + and inside ImGuiMultiSelectIO structure.
// - Most users are likely to use this store an item INDEX but this may be used to store a POINTER/ID as well. Read comments near ImGuiMultiSelectIO for details.
typedef ImS64 ImGuiSelectionUserData;

// Callback and functions types
typedef int     (*ImGuiInputTextCallback)(ImGuiInputTextCallbackData* data);    // Callback function for ImGui::InputText()
typedef void    (*ImGuiSizeCallback)(ImGuiSizeCallbackData* data);              // Callback function for ImGui::SetNextWindowSizeConstraints()
typedef void* (*ImGuiMemAllocFunc)(size_t sz, void* user_data);               // Function signature for ImGui::SetAllocatorFunctions()
typedef void    (*ImGuiMemFreeFunc)(void* ptr, void* user_data);                // Function signature for ImGui::SetAllocatorFunctions()

// ImVec2: 2D vector used to store positions, sizes etc. [Compile-time configurable type]
// This is a frequently used type in the API. Consider using IM_VEC2_CLASS_EXTRA to create implicit cast from/to our preferred type.
// Add '#define IMGUI_DEFINE_MATH_OPERATORS' in your imconfig.h file to benefit from courtesy maths operators for those types.
IM_MSVC_RUNTIME_CHECKS_OFF
struct ImVec2
{
    float                                   x, y;
    constexpr ImVec2() : x(0.0f), y(0.0f) { }
    constexpr ImVec2(float _x, float _y) : x(_x), y(_y) { }
    float& operator[] (size_t idx) { IM_ASSERT(idx == 0 || idx == 1); return ((float*)(void*)(char*)this)[idx]; } // We very rarely use this [] operator, so the assert overhead is fine.
    float  operator[] (size_t idx) const { IM_ASSERT(idx == 0 || idx == 1); return ((const float*)(const void*)(const char*)this)[idx]; }
#ifdef IM_VEC2_CLASS_EXTRA
    IM_VEC2_CLASS_EXTRA     // Define additional constructors and implicit cast operators in imconfig.h to convert back and forth between your math types and ImVec2.
#endif
};

// ImVec4: 4D vector used to store clipping rectangles, colors etc. [Compile-time configurable type]
struct ImVec4
{
    float                                                     x, y, z, w;
    constexpr ImVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
    constexpr ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }
#ifdef IM_VEC4_CLASS_EXTRA
    IM_VEC4_CLASS_EXTRA     // Define additional constructors and implicit cast operators in imconfig.h to convert back and forth between your math types and ImVec4.
#endif
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

export namespace ImGui
{

}