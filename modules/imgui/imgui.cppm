module;

#include <imgui.h>

export module imgui;

//export
//{
//	// Scalar data types
//	using ImGuiID = ::ImGuiID;// A unique ID used by widgets (typically the result of hashing a stack of string)
//	using ImS8 = ::ImS8;   // 8-bit signed integer
//	using ImU8 = ::ImU8;   // 8-bit unsigned integer
//	using ImS16 = ::ImS16;  // 16-bit signed integer
//	using ImU16 = ::ImU16;  // 16-bit unsigned integer
//	using ImS32 = ::ImS32;  // 32-bit signed integer == int
//	using ImU32 = ::ImU32;  // 32-bit unsigned integer (often used to store packed colors)
//	using ImS64 = ::ImS64;  // 64-bit signed integer
//	using ImU64 = ::ImU64;  // 64-bit unsigned integer
//	
//	// Other data types
//	using ImDrawChannel = ::ImDrawChannel;               // Temporary storage to output draw commands out of order, used by ImDrawListSplitter and ImDrawList::ChannelsSplit()
//	using ImDrawCmd = ::ImDrawCmd;                   // A single draw command within a parent ImDrawList (generally maps to 1 GPU draw call, unless it is a callback)
//	using ImDrawData = ::ImDrawData;                  // All draw command lists required to render the frame + pos/size coordinates to use for the projection matrix.
//	using ImDrawList = ::ImDrawList;                  // A single draw command list (generally one per window, conceptually you may see this as a dynamic "mesh" builder)
//	using ImDrawListSharedData = ::ImDrawListSharedData;        // Data shared among multiple draw lists (typically owned by parent ImGui context, but you may create one yourself)
//	using ImDrawListSplitter = ::ImDrawListSplitter;          // Helper to split a draw list into different layers which can be drawn into out of order, then flattened back.
//	using ImDrawVert = ::ImDrawVert;                  // A single vertex (pos + uv + col = 20 bytes by default. Override layout with IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
//	using ImFont = ::ImFont;                      // Runtime data for a single font within a parent ImFontAtlas
//	using ImFontAtlas = ::ImFontAtlas;                 // Runtime data for multiple fonts, bake multiple fonts into a single texture, TTF/OTF font loader
//	using ImFontBuilderIO = ::ImFontBuilderIO;             // Opaque interface to a font builder (stb_truetype or FreeType).
//	using ImFontConfig = ::ImFontConfig;                // Configuration data when adding a font or merging fonts
//	using ImFontGlyph = ::ImFontGlyph;                 // A single font glyph (code point + coordinates within in ImFontAtlas + offset)
//	using ImFontGlyphRangesBuilder = ::ImFontGlyphRangesBuilder;    // Helper to build glyph ranges from text/string data
//	using ImColor = ::ImColor;                     // Helper functions to create a color that can be converted to either uint32_t or float4 (*OBSOLETE* please avoid using)
//	using ImGuiContext = ::ImGuiContext;                // Dear ImGui context (opaque structure, unless including imgui_internal.h)
//	using ImGuiIO = ::ImGuiIO;                     // Main configuration and I/O between your application and ImGui
//	using ImGuiInputTextCallbackData = ::ImGuiInputTextCallbackData;  // Shared state of InputText() when using custom ImGuiInputTextCallback (rare/advanced use)
//	using ImGuiKeyData = ::ImGuiKeyData;                // Storage for ImGuiIO and IsKeyDown(), IsKeyPressed() etc functions.
//	using ImGuiListClipper = ::ImGuiListClipper;            // Helper to manually clip large list of items
//	using ImGuiMultiSelectIO = ::ImGuiMultiSelectIO;          // Structure to interact with a BeginMultiSelect()/EndMultiSelect() block
//	using ImGuiOnceUponAFrame = ::ImGuiOnceUponAFrame;         // Helper for running a block of code not more than once a frame
//	using ImGuiPayload = ::ImGuiPayload;                // User data payload for drag and drop operations
//	using ImGuiPlatformIO = ::ImGuiPlatformIO;             // Multi-viewport support: interface for Platform/Renderer backends + viewports to render
//	using ImGuiPlatformMonitor = ::ImGuiPlatformMonitor;        // Multi-viewport support: user-provided bounds for each connected monitor/display. Used when positioning popups and tooltips to avoid them straddling monitors
//	using ImGuiPlatformImeData = ::ImGuiPlatformImeData;        // Platform IME data for io.PlatformSetImeDataFn() function.
//	using ImGuiSelectionBasicStorage = ::ImGuiSelectionBasicStorage;  // Optional helper to store multi-selection state + apply multi-selection requests.
//	using ImGuiSelectionExternalStorage = ::ImGuiSelectionExternalStorage;//Optional helper to apply multi-selection requests to existing randomly accessible storage.
//	using ImGuiSelectionRequest = ::ImGuiSelectionRequest;       // A selection request (stored in ImGuiMultiSelectIO)
//	using ImGuiSizeCallbackData = ::ImGuiSizeCallbackData;       // Callback data when using SetNextWindowSizeConstraints() (rare/advanced use)
//	using ImGuiStorage = ::ImGuiStorage;                // Helper for key->value storage (container sorted by key)
//	using ImGuiStoragePair = ::ImGuiStoragePair;            // Helper for key->value storage (pair)
//	using ImGuiStyle = ::ImGuiStyle;                  // Runtime data for styling/colors
//	using ImGuiTableSortSpecs = ::ImGuiTableSortSpecs;         // Sorting specifications for a table (often handling sort specs for a single column, occasionally more)
//	using ImGuiTableColumnSortSpecs = ::ImGuiTableColumnSortSpecs;   // Sorting specification for one column of a table
//	using ImGuiTextBuffer = ::ImGuiTextBuffer;             // Helper to hold and append into a text buffer (~string builder)
//	using ImGuiTextFilter = ::ImGuiTextFilter;             // Helper to parse and apply text filters (e.g. "aaaaa[,bbbbb][,ccccc]")
//	using ImGuiViewport = ::ImGuiViewport;               // A Platform Window (always 1 unless multi-viewport are enabled. One per platform window to output to). In the future may represent Platform Monitor
//	using ImGuiWindowClass = ::ImGuiWindowClass;            // Window class (rare/advanced uses: provide hints to the platform backend via altered viewport flags and parent/child info)
//
//	// Enumerations
//// - We don't use strongly typed enums much because they add constraints (can't extend in private code, can't store typed in bit fields, extra casting on iteration)
//// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
////   - In Visual Studio: CTRL+comma ("Edit.GoToAll") can follow symbols inside comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
////   - In Visual Studio w/ Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
////   - In VS Code, CLion, etc.: CTRL+click can follow symbols inside comments.
//	using ImGuiDir = ::ImGuiDir;                // -> enum ImGuiDir              // Enum: A cardinal direction (Left, Right, Up, Down)
//	using ImGuiKey = ::ImGuiKey;                // -> enum ImGuiKey              // Enum: A key identifier (ImGuiKey_XXX or ImGuiMod_XXX value)
//	using ImGuiMouseSource = ::ImGuiMouseSource;        // -> enum ImGuiMouseSource      // Enum; A mouse input source identifier (Mouse, TouchScreen, Pen)
//	using ImGuiSortDirection = ::ImGuiSortDirection;     // -> enum ImGuiSortDirection    // Enum: A sorting direction (ascending or descending)
//	using ImGuiCol = ::ImGuiCol;               // -> enum ImGuiCol_             // Enum: A color identifier for styling
//	using ImGuiCond = ::ImGuiCond;              // -> enum ImGuiCond_            // Enum: A condition for many Set*() functions
//	using ImGuiDataType = ::ImGuiDataType;          // -> enum ImGuiDataType_        // Enum: A primary data type
//	using ImGuiMouseButton = ::ImGuiMouseButton;       // -> enum ImGuiMouseButton_     // Enum: A mouse button identifier (0=left, 1=right, 2=middle)
//	using ImGuiMouseCursor = ::ImGuiMouseCursor;       // -> enum ImGuiMouseCursor_     // Enum: A mouse cursor shape
//	using ImGuiStyleVar = ::ImGuiStyleVar;          // -> enum ImGuiStyleVar_        // Enum: A variable identifier for styling
//	using ImGuiTableBgTarget = ::ImGuiTableBgTarget;     // -> enum ImGuiTableBgTarget_   // Enum: A color target for TableSetBgColor()
//
//	// Flags (declared as int to allow using as flags without overhead, and to not pollute the top of this file)
//	// - Tip: Use your programming IDE navigation facilities on the names in the _central column_ below to find the actual flags/enum lists!
//	//   - In Visual Studio: CTRL+comma ("Edit.GoToAll") can follow symbols inside comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
//	//   - In Visual Studio w/ Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
//	//   - In VS Code, CLion, etc.: CTRL+click can follow symbols inside comments.
//	using ImDrawFlags = ::ImDrawFlags;            // -> enum ImDrawFlags_          // Flags: for ImDrawList functions
//	using ImDrawListFlags = ::ImDrawListFlags;        // -> enum ImDrawListFlags_      // Flags: for ImDrawList instance
//	using ImFontAtlasFlags = ::ImFontAtlasFlags;       // -> enum ImFontAtlasFlags_     // Flags: for ImFontAtlas build
//	using ImGuiBackendFlags = ::ImGuiBackendFlags;      // -> enum ImGuiBackendFlags_    // Flags: for io.BackendFlags
//	using ImGuiButtonFlags = ::ImGuiButtonFlags;       // -> enum ImGuiButtonFlags_     // Flags: for InvisibleButton()
//	using ImGuiChildFlags = ::ImGuiChildFlags;        // -> enum ImGuiChildFlags_      // Flags: for BeginChild()
//	using ImGuiColorEditFlags = ::ImGuiColorEditFlags;    // -> enum ImGuiColorEditFlags_  // Flags: for ColorEdit4(), ColorPicker4() etc.
//	using ImGuiConfigFlags = ::ImGuiConfigFlags;       // -> enum ImGuiConfigFlags_     // Flags: for io.ConfigFlags
//	using ImGuiComboFlags = ::ImGuiComboFlags;        // -> enum ImGuiComboFlags_      // Flags: for BeginCombo()
//	using ImGuiDockNodeFlags = ::ImGuiDockNodeFlags;     // -> enum ImGuiDockNodeFlags_   // Flags: for DockSpace()
//	using ImGuiDragDropFlags = ::ImGuiDragDropFlags;     // -> enum ImGuiDragDropFlags_   // Flags: for BeginDragDropSource(), AcceptDragDropPayload()
//	using ImGuiFocusedFlags = ::ImGuiFocusedFlags;      // -> enum ImGuiFocusedFlags_    // Flags: for IsWindowFocused()
//	using ImGuiHoveredFlags = ::ImGuiHoveredFlags;      // -> enum ImGuiHoveredFlags_    // Flags: for IsItemHovered(), IsWindowHovered() etc.
//	using ImGuiInputFlags = ::ImGuiInputFlags;        // -> enum ImGuiInputFlags_      // Flags: for Shortcut(), SetNextItemShortcut()
//	using ImGuiInputTextFlags = ::ImGuiInputTextFlags;    // -> enum ImGuiInputTextFlags_  // Flags: for InputText(), InputTextMultiline()
//	using ImGuiItemFlags = ::ImGuiItemFlags;         // -> enum ImGuiItemFlags_       // Flags: for PushItemFlag(), shared by all items
//	using ImGuiKeyChord = ::ImGuiKeyChord;          // -> ImGuiKey | ImGuiMod_XXX    // Flags: for IsKeyChordPressed(), Shortcut() etc. an ImGuiKey optionally OR-ed with one or more ImGuiMod_XXX values.
//	using ImGuiPopupFlags = ::ImGuiPopupFlags;        // -> enum ImGuiPopupFlags_      // Flags: for OpenPopup*(), BeginPopupContext*(), IsPopupOpen()
//	using ImGuiMultiSelectFlags = ::ImGuiMultiSelectFlags;  // -> enum ImGuiMultiSelectFlags_// Flags: for BeginMultiSelect()
//	using ImGuiSelectableFlags = ::ImGuiSelectableFlags;   // -> enum ImGuiSelectableFlags_ // Flags: for Selectable()
//	using ImGuiSliderFlags = ::ImGuiSliderFlags;       // -> enum ImGuiSliderFlags_     // Flags: for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
//	using ImGuiTabBarFlags = ::ImGuiTabBarFlags;       // -> enum ImGuiTabBarFlags_     // Flags: for BeginTabBar()
//	using ImGuiTabItemFlags = ::ImGuiTabItemFlags;      // -> enum ImGuiTabItemFlags_    // Flags: for BeginTabItem()
//	using ImGuiTableFlags = ::ImGuiTableFlags;        // -> enum ImGuiTableFlags_      // Flags: For BeginTable()
//	using ImGuiTableColumnFlags = ::ImGuiTableColumnFlags;  // -> enum ImGuiTableColumnFlags_// Flags: For TableSetupColumn()
//	using ImGuiTableRowFlags = ::ImGuiTableRowFlags;     // -> enum ImGuiTableRowFlags_   // Flags: For TableNextRow()
//	using ImGuiTreeNodeFlags = ::ImGuiTreeNodeFlags;     // -> enum ImGuiTreeNodeFlags_   // Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
//	using ImGuiViewportFlags = ::ImGuiViewportFlags;     // -> enum ImGuiViewportFlags_   // Flags: for ImGuiViewport
//	using ImGuiWindowFlags = ::ImGuiWindowFlags;       // -> enum ImGuiWindowFlags_     // Flags: for Begin(), BeginChild()
//
//	// ImTexture: user data for renderer backend to identify a texture [Compile-time configurable type]
//	// - To use something else than an opaque void* pointer: override with e.g. '#define ImTextureID MyTextureType*' in your imconfig.h file.
//	// - This can be whatever to you want it to be! read the FAQ about ImTextureID for details.
//	using ImTextureID = ::ImTextureID;          // Default: store a pointer or an integer fitting in a pointer (most renderer backends are ok with that)
//
//	// ImDrawIdx: vertex index. [Compile-time configurable type]
//	// - To use 16-bit indices + allow large meshes: backend need to set 'io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset' and handle ImDrawCmd::VtxOffset (recommended).
//	// - To use 32-bit indices: override with '#define ImDrawIdx unsigned int' in your imconfig.h file.
//	using ImDrawIdx = ::ImDrawIdx;   // Default: 16-bit (for maximum compatibility with renderer backends)
//
//	// Character types
//	// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded character used for keyboard input and display)
//	using ImWchar32 = ::ImWchar32;     // A single decoded U32 character/code point. We encode them as multi bytes UTF-8 when used in strings.
//	using ImWchar16 = ::ImWchar16;   // A single decoded U16 character/code point. We encode them as multi bytes UTF-8 when used in strings.
//	using ImWchar = ::ImWchar; // ImWchar [configurable type: override in imconfig.h with '#define IMGUI_USE_WCHAR32' to support Unicode planes 1-16]
//
//}
//
//export namespace ImGui
//{
//
//}