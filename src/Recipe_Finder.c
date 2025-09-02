/*
*****************************************************************************
* SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
* License: https://polyformproject.org/licenses/noncommercial/1.0.0/
* SPDX-FileCopyrightText: 2025 John Mastronardo
* Copyright (c) 2025 John Mastronardo
*
*     Project: Recipe Finder
*     Author: John Mastronardo
*     Language: C (C11)
*     Toolkit: GTK 3 (Cross-platform GUI)
*     File: recipe_finder.c   Cross-Platform GTK App for Recipe Search
*     Last Updated: 2025-09-02  GitHub Build: 1
*
* License Summary:
*     This is a hobby and educational project, created as a learning tool
*     and for personal exploration. You are free to explore, modify, and
*     build upon it.
*
*     Allowed: Personal use, education, academic research.
*     Not allowed: Commercial use, paid services, SaaS, or enterprise
*     integration.
*
*     No official support is provided. Use at your own discretion.
*
*     This notice must be included with any substantial use of the code.
*
* ---------------------------------------------------------------------------
* PROJECT OVERVIEW:
* ---------------------------------------------------------------------------

* "Built as a hands-on C and GTK learning tool, not just to find recipes,
*   but to cook up better code." – JM
*
* This is a cross-platform Windows and macOS Food Recipe Finder application
* written in C with a GTK 3 user interface.
* It dynamically loads website-specific recipe parsers via Node.js scripts and
* uses libcurl for HTTP, Gumbo for HTML parsing, and json-c for JSON handling.
*
* Key Design Points:
*     - Cross-platform: Tested to run on Windows and macOS.
 *      (Should run under Linux, but it was not tested.)
*     - Uses platform-specific headers to query system info (RAM, temp dirs).
*     - Plays nicely with MSYS2 on Windows.
*     - GLib functions like g_get_user_config_dir() ensure proper config paths.
*
*     - Parser Architecture:
*         - Each recipe site has a dedicated parser function.
*         - Node.js scripts are used for sites requiring Playwright or Cheerio.
*         - Fallback URLs are provided if parsing fails.
*         - Temporary JS script files are generated and executed as needed.
*
*     - Memory Safety and Cleanup:
*         - Careful allocation and freeing of buffers, JSON objects, and GTK
*           widgets.
*         - Defensive code protects against null pointers, partial content, and
*           failed allocations.
*         - Initialization and shutdown sequences manage resources cleanly.
*
*     - GTK Behavior and Lifecycle Notes:
*       GTK uses a complex callback and widget lifecycle model.
*       In event-driven programming like GTK, you don't call a function
*       directly when something happens. Instead, you register a function
*       with the toolkit, e.g.:
*         g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked))
*       Later, when the event occurs (e.g., the button is clicked), GTK
*       "calls back" into the code, to invoke the function.
*     - Widgets may be destroyed asynchronously; multi-threaded updates must
*           use g_idle_add().
*
*     - UI/UX:
*         - GTK widgets are styled via embedded CSS for a polished appearance.
*         - Non-blocking HTTP requests and parsing ensure a responsive user
*           interface and a smooth user experience.
*
*     - Runtime Checks:
*         - Initial splash screen detects required Node.js, npm packages, and
*           Playwright browser installations.
*         - Displays user-friendly error dialogs for missing software dependencies.
*         - Creates a marker file (recipe_finder_checked_ok) in platform-correct
*           config file folder to avoid redundant dependency checks.
*
*
* ---------------------------------------------------------------------------
* DOCUMENTATION PHILOSOPHY:
* ---------------------------------------------------------------------------
*
*     - This source code includes extensive code comments
*        for each function, which explain the purpose, reasoning,
*        behavior, and idiosyncrasies.
*     - Emphasizes readability, maintainability, learning, and collaboration.
*
*
* ---------------------------------------------------------------------------
* HOW THE APP WORKS:
* ---------------------------------------------------------------------------
 *
* 1. The app opens with a small, resizable window showing the user interface.
*
* 2. The user enters a recipe search term, such as chili or "roast chicken".
*    If the search term contains quotation marks, they are treated as
*    exact-match terms, similar to Google search behavior.
*
* 3. The user selects a recipe site from a dropdown list of 20 food websites,
*    then clicks the "Search for Recipes" button.
*
* 4. The app constructs a URL query based on the search term.
*
*    - For JavaScript-heavy sites:
*        • A temporary Node.js script file is generated and executed.
*        • The script uses Playwright and Cheerio to scrape structured data.
*    - For simpler (static HTML) sites:
*        • The app downloads the HTML using libcurl.
*        • The content is parsed using the Gumbo HTML5 parser.
*
* 5. Search results are displayed as a clickable list of GTK widgets.
*
* 6. When the user clicks a recipe link, the app opens the URL
*    in the user's default web browser.
*
*
* ---------------------------------------------------------------------------
* REFLECTION AND DEVELOPMENT INSIGHTS
* ---------------------------------------------------------------------------
* 
* - Learning Resource:
*   This app serves as a practical open-source learning tool for developers
*   exploring cross-platform GUI development in C. It is a real-world fully
*   functional app that demonstrates best practices in GUI programming.
*
* - GTK Selection:
*   GTK 3 was chosen for its cross-platform support, enabling a single C
*   codebase  to run natively on Windows, macOS, and Linux. This avoids the
*   need to rewrite platform-specific UI logic, which is a core strength of GTK.
* 
* - GTK Overview:
*   GTK (GIMP Toolkit, originally the GNU Image Manipulation Program) is
*   free, open-source GUI framework licensed under the GNU LGPL, supporting
*   both free and proprietary use. Versions 3 and 4 are actively maintained,
*   offering over 200 widgets. 
*   Note that a  key criticism of GTK is its poor backward compatibility
*   across major versions, often requiring code changes during upgrades.
* 
* - Why GTK 3:
*   GTK 3 was chosen for this project because:
*   - It offers native widgets on Windows, macOS, and Linux, ensuring portability
*     without platform-specific UI code.
*   - It is well-documented, mature, and widely available through package managers
*     like MSYS2 and Homebrew.
*   - A large body of tutorials and resources exist for GTK 3.
*   - At the time, GTK 4 was still developing third-party support, especially for
*     Windows toolchains and certain GLib/GObject patterns.
*   - GTK 3's simpler API (compared to GTK 4's GSK and new layout system)
*     provided a lower entry barrier for a single-developer project.
*   - (See the below Future section for notes regarding migration to GTK 4.)
* 
* - GTK Version Naming:
*   The official name for GTK version 3 is "GTK 3" (the "+" sign was
*    dropped in 2019).
*   GTK 4 and later versions will follow the same naming convention.
* 
* - Alternative Frameworks:
*   While Qt offers rich UI features, it is C++-centric. Lighter frameworks like
*   FLTK, IUP, and Dear ImGui lack some of GTK’s modern features and native
*   widget support. For a C project prioritizing portability and native UI look,
*   GTK was the most balanced choice.
* 
*   Building the app highlighted the quirks of GTK, memory safety, and
*   cross-platform challenges. Integrating asynchronous callbacks with various
*   independently managed systems (like GTK's event loop, libcurl, HTML
*   parsers, and background threads) required careful thread management and
*   defensive programming to maintain stability.
* 
* - Key Development Practices:
*   - Prioritizing pointer safety throughout the app.
*   - Validating memory allocations to handle failure scenarios reliably.
*   - Promptly freeing resources to prevent leaks and double frees.
*   - Writing efficient code to manage GTK's asynchronous behavior.
*   - Focusing on memory management across all components.
*   - Structuring code for modularity to enhance stability and reduce bugs.
* 
*
* ---------------------------------------------------------------------------
* BUILD INSTRUCTIONS AND COMPILER NOTES:
*     (AS OF AUGUST 2025)
* ---------------------------------------------------------------------------
*
* Windows (MSYS2 MinGW 64-bit):
*
* GCC Compiler Notes:
*
*     - Compiler: GCC (MSYS2 MinGW 64-bit)
*     - Required version: GCC 11.0 or higher
*         (Due to use of C11 features and GLib/GTK compatibility)
*     - Recommended: GCC 15.2.0 or later
*         (Latest tested version: 15.2.0, updated 2025-08-17)
*         (Check with: gcc --version)
*
*     - MSYS2 Environment:
*         - Terminal: MSYS2 MinGW 64-bit shell (MINGW64)
*         - MSYS2 runtime version: 3.6.4-1
*             (Verify with: pacman -Qi msys2-runtime)
*         - Kernel/environment info: MINGW64_NT-10.0-26100
*             (Verify with: uname -a)
*         - pacman version: 6.1.0
*             (Verify with: pacman -V)

*     - Install required dependencies:
*
*         pacman -S mingw-w64-x86_64-gtk3 \
*                   mingw-w64-x86_64-json-c \
*                   mingw-w64-x86_64-curl \
*                   mingw-w64-x86_64-gumbo
*
*     - GTK version used in development: GTK 3.24.50 (as of August 2025)
*
*     - Compile command (Windows):
*
*         gcc -o recipe_finder.exe recipe_finder.c \
*             $(pkg-config --cflags --libs gtk+-3.0 json-c) \
*             -lcurl -lgumbo -Wall -Wextra -std=c11 -g

*
* Windows Compile command:
*
*       gcc -o recipe_finder.exe recipe_finder.c $(pkg-config --cflags --libs gtk+-3.0 json-c) -lcurl -lgumbo -Wall -Wextra -std=c11 -g
*
*  ---------------------------------------------------------------------------
* 
* macOS (Homebrew):
*
*     - Install C dependencies:
*         brew install gcc GTK 3 json-c curl gumbo-parser node npm
*
*     - Install a specific GCC version (e.g., GCC 13):
*         brew install gcc@13
*         brew link gcc@13 --force
*
*         After installation, the C compiler will be available as 'gcc-13'.
*
*     - Install global npm packages and browsers:
*         npm install -g playwright cheerio axios
*         npx playwright install
*
*     - macOS Compile command:
*
*       gcc-13 $(pkg-config --cflags gtk+-3.0 json-c) -std=c11 -Wall -Wextra -g recipe_finder.c -o Recipe_Finder $(pkg-config --libs gtk+-3.0 json-c) -lcurl -lgumbo
*
*     - Notes:
*         - Use 'brew --prefix' to troubleshoot include or library path issues.
*         - If needed, set the pkg-config path manually:
*
*             export PKG_CONFIG_PATH="$(brew --prefix)/lib/pkgconfig:$(brew --prefix)/share/pkgconfig:$PKG_CONFIG_PATH"
*
*         - GTK 3 GUI apps may require XQuartz to be installed on macOS.
*
* Explanation:
*
*     - The command 'brew install gcc@13' installs that specific GCC version.
*
*     - 'brew link gcc@13 --force' creates a symlink for 'gcc-13' in your PATH,
*       allowing you to invoke it directly.
*
*     - Homebrew may warn that GCC is "keg-only," meaning it's not linked
*       by default.
*       This command forces the symlink into a standard binary path, such as:
*         /usr/local/bin         (Intel Macs)
*         /opt/homebrew/bin      (Apple Silicon Macs)
*
*     - On newer versions of Homebrew, you may prefer '--overwrite'
*        instead of '--force':
*
*         brew link gcc@13 --overwrite
*
*       The '--overwrite' switch replaces any conflicting symlinks from older
*        versions of GCC, which is safer and more appropriate on systems
*        where 'gcc' is already linked.
*
* Linux builds are currently untested, but are expected to work with
* a C11-compliant GCC compiler, GTK 3, json-c, libcurl, and Gumbo installed.
* Feedback from Linux users is welcome.
*
*
* ---------------------------------------------------------------------------
* COMPILER AND RUNTIME TROUBLESHOOTING:
* ---------------------------------------------------------------------------
*
*     - Confirm the C compiler matches your system architecture (x64 vs x86).
*     - Use C11-compliant compiler: GCC on Windows/macOS/Linux.
*     - On macOS, 'brew --prefix' can help resolve library include paths.
*     - Ensure pkg-config is pointing to the correct GTK3/json-c/libcurl paths.
*     - Ensure pkg-config paths match installed libraries.
*
*
* ---------------------------------------------------------------------------
* RECIPE PARSER OVERVIEW:
* ---------------------------------------------------------------------------
*
*     - The majority of recipe parsers depend on JavaScript and therefore
*       require Node.js, npm packages, and Playwright. 
*       However, a few simpler parsers (e.g., simplyrecipes) use only libcurl
*       and Gumbo, eliminating the need for a JS runtime.
*     - Most parsers return a list of matching recipe links in the user interface.
*       Some may return a single link that aggregates multiple recipe cards for
*       the target food site.
*     - All parsers include fallback logic. As a last resort, they will return
*       a link to the food site's main homepage if no recipe-specific results
*       can be found.
*     - Note: Some of the web pages contain paywalls or trial offers.
*
*
* ---------------------------------------------------------------------------
* PROJECT STRUCTURE:
* ---------------------------------------------------------------------------
*
*     - Single-file C design simplifies distribution and compilation.
*     - Embedded CSS, JSON defaults, and JS templates are C string constants.
*     - Modular Parsing Architecture:
*         - Parsing is abstracted into site-specific functions, each dedicated
*           to a particular recipe website source.
*         - Function pointers are used to select the appropriate parser based
*           on the chosen website, ensuring flexibility and easy extensibility.
*         - For sites requiring JavaScript rendering, Node.js scripts are
*           dynamically generated and executed.
*         - Fallback logic ensures resilience by returning a default link if
*           recipe parsing fails.
*
* ---------------------------------------------------------------------------
* FEATURES:
* ---------------------------------------------------------------------------
*
*
*     - GTK 3 GUI with colorful CSS styling in the main app window.
*     - Site-specific parser functions with fallback links.
*     - Responsive UI with g_idle_add() for thread-safe updates.
*     - In-memory RAM buffers dynamically sized based on system configuration.
*     - Multi-threaded network requests and headless automation.
*     - Integrates with Node.js and Playwright for JS-based recipe parsing.
*     - Automates cross-platform file paths (g_get_user_config_dir).
*     - Automatically checks for required software package dependencies,
*       displays friendly error messages for any missing components, and
*       creates a marker file to skip future checks.
*
*
* ---------------------------------------------------------------------------
* GTK BEHAVIOR NOTES:
* ---------------------------------------------------------------------------
*
*     - Widgets are updated on the main thread only.
*     - g_idle_add() is preferred for async GUI updates.
*     - Careful memory management prevents leaks or crashes.
*     - GTK implicitly uses the Pango library internally for all text rendering.
*       Therefore, all strings passed to GTK widgets (labels, list items, etc.)
*       must be valid UTF-8. Passing invalid UTF-8 strings will trigger runtime
*       warnings like:
*           "Pango-WARNING: Invalid UTF-8 string passed to pango_ ..."
*       These warnings are non-fatal but typically signal malformed UTF-8
*        input, and can cause blank or garbled text to appear in the UI.
*
*
* ---------------------------------------------------------------------------
* LEARNING TIPS:
* ---------------------------------------------------------------------------
*
*   - Always free buffers and GTK widgets to prevent leaks.
*   - Use g_idle_add() for thread-safe GUI updates.
*   - Check Node.js dependencies before running parsers.
*   - Explore each parser function to understand site-specific scraping logic.
*
*
* ---------------------------------------------------------------------------
* About UTF-8 Encoding and Special Characters:
* ---------------------------------------------------------------------------
*
* UTF-8 is a common text encoding that represents every character (letters,
*   numbers, symbols) from all languages as a sequence of one to four bytes.
* It is backward-compatible with ASCII and widely used because it supports
*   international characters while being efficient and easy to handle.
* Ensuring all text strings are valid UTF-8 is essential for proper display,
*   especially when using UI toolkits like GTK that expect UTF-8 input.
* Note that fancy/curly quote marks, and international characters like vowels
*   with acute, grave, circumflex, and umlaut accents (known as diacritics)
*   may appear as weird symbols in the UI, as well as in some program editors
*   and text viewers.
*   E.g.: One of my favorite recipes: Peperonata (Sautéed Peppers and Onions)
*
*
* ---------------------------------------------------------------------------
* DEBUG AND RUNTIME LOGGING:
* ---------------------------------------------------------------------------
*
*    Extensive debug and runtime informational messages are printed to the
*    terminal, but these messages are only visible when running the program
*    from the console/terminal.  If the app is installed via Inno Setup on
*    Windows or packaged on macOS, these runtime messages will not appear
*    in a terminal window. This is normal behavior for GUI applications that
*    are launched outside a shell.
*
*
* ---------------------------------------------------------------------------
* KNOWN ISSUES:
* ---------------------------------------------------------------------------
*
*     - Parser fragility if food sites change their HTML structure.
*     - Automated file paths may require updates.
*     - Important Note on Antivirus Alerts:
*         Some antivirus programs may flag components of this project due to
*         heuristic detection of dynamically generated scripts or network
*         activity.
*         Specifically, the app generates temporary Node.js scripts at runtime
*         for certain recipe parsers. This behavior may be detected as a false-
*         positive threat to some antivirus engines, even though the scripts are
*         safe and created locally based on your input.
*         Recommended Actions:
*             - Review the source code to verify safety, and always compile
*               the app yourself from source to ensure safety.
*             - If false positives occur, consider contacting your antivirus vendor
*               for clarification.
*
*
* ---------------------------------------------------------------------------
* FUTURE IMPROVEMENTS:
* ---------------------------------------------------------------------------
*
*     - Add a SQLite database to manage recipe favorites and search history.
*     - Better support of quoted search terms in the search logic to improve
*       exact recipe matches.
*     - Implement a single search operation that aggregates recipe results
*       from all 20 parsers.
*       Add even more food websites to the current list of 20 recipe parsers.
*     - Use in-memory Node.js execution to avoid temporary files.
*     - Enhanced error logging and reporting.
*     - Improve recipe parser maintainability by automating failure detection
*       and streamlining parser updates.
*     - Improve recipe title sanitization with reliable logic to handle accented
*       characters gracefully, without risking blank or corrupted output.
*     - Investigate mobile ports (via Swift iOS/iPadOS prototype).
*     - Migration to GTK 4:
*         - GTK 4 improves performance, GPU rendering, CSS styling, and
*           long-term support.
*         - The migration requires adapting to GTK 4's updated API, breaking
*           backward compatibility.
*         - A substantial refactor will be needed, including reworking function
*           calls, layout, and rendering.
*         - Key changes would also include redesigned event handling, layout
*           management, and container removal.
*         - Despite the effort, GTK 4 offers long-term benefits like:
*             - GPU-accelerated rendering and smoother UI.
*             - Modernized widget system.
*             - Better mobile and high-DPI support.
*             - Ongoing support from the GTK community, including enhanced
*               accessibility and internationalization to adapt to different
*               cultures, languages, and regions around the world.
*
*
* ---------------------------------------------------------------------------
* SPECIAL THANKS:
* ---------------------------------------------------------------------------
*
* Huge thanks to OpenAI's ChatGPT for:
*
*     - Help with complex C and GTK syntax.
*     - Solving esoteric C compiler errors.
*     - Guidance on the precise design and nuanced structure
*       of the site-specific recipe parser functions.
*
*         -- John Mastronardo, August 2025
*
*
*****************************************************************************
*/


// ---------------------------------------------------------------------------
// C Libraries
// ---------------------------------------------------------------------------

#include <stdio.h>             // Standard I/O: printf, FILE, fopen, etc.
#include <stdlib.h>            // Memory allocation, exit, conversion,  setenv
#include <string.h>            // String handling (strcpy, strstr)
#include <ctype.h>             // Character tests (isspace, isalpha)
#include <stdbool.h>           // Bool type and true and false constants
#include <time.h>              // Date and time functions
#include <sys/stat.h>          // mkdir file handing
#include <gio/gio.h>           // provides asynchronous APIs for i/o operations
// Third-Party Libraries:
#include <gtk/gtk.h>           // GTK top-level toolkit (GUI, widgets, windows)
#include <glib.h>              // GTK core utilities (data structures, memory)
#include <gdk/gdk.h>           // Drawing/cursor layer (graphics backend)
#include <curl/curl.h>         // libcurl networking
#include <gumbo.h>             // Gumbo HTML parser
#include <json-c/json.h>       // JSON parsing with json-c
// Platform-specific headers for retrieving system info:
#if defined(_WIN32)
    #define NOMINMAX           // Avoid min/max macro conflicts
    #include <windows.h>       // Windows system functions
#elif defined(__APPLE__)
    #include <sys/types.h>     // macOS types
    #include <sys/sysctl.h>    // macOS system info
    #include <mach/mach.h>
    #include <unistd.h>        // Unix standard functions: file, process, environ ops
#elif defined(__linux__)
    #include <sys/sysinfo.h>   // Linux system info (not tested!)
    #include <unistd.h>        // POSIX API (Unix standard functions)
#endif



// ===========================================================================
// Global Variables
// ===========================================================================

// Limits the number of returned recipe-link results
#define MAX_RESULTS 50

// Counter to control maximum number of recipe links created
static int recipe_result_total = 0;

// Holds the current recipe site being searched
// Used in curl_write_callback terminal status messages
char *g_current_website_name = NULL;

// Active-search switch
static gboolean search_in_progress = FALSE;


// ===========================================================================
// Enumerations
// ===========================================================================

// Enum types must be declared before any structs or functions that reference
// them, to avoid unknown type errors.
// ----------------------------------------------------------------------------
// QuoteStatus
// Represents the quoting state of a recipe search term.
// Determined in initialize_on_search() by scanning for quote characters.
// Used downstream (search, parsing, display) to decide whether input should be
// treated as strongly quoted phrases or free text.
typedef enum {
    QUOTE_NONE,    // No quote characters present
    QUOTE_SINGLE,  // Unmatched single or double quotes detected
    QUOTE_PAIR     // Properly paired quotes, e.g. "chocolate cake"
} QuoteStatus;


// ===========================================================================
// Typedef and Struct Definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// AppWidgets
// Holds references to GTK widgets that make up the primary UI.
// ---------------------------------------------------------------------------
typedef struct {
    GtkWidget *entry;           // User text input for recipe search term
    GtkWidget *combo;           // Combo box for category/filter selection
    GtkWidget *listbox;         // Displays recipe results as clickable items
    GtkWidget *status_label;    // Status messages ("Searching...", "No results")
    GtkWidget *search_button;   // Button that triggers search
    GtkWidget *progress_bar;    // Shows search progress (pulse/fill)
    guint pulse_timer_id;       // Timer ID for progress bar pulsing
    QuoteStatus quote_status;   // Tracks search input quoting state
} AppWidgets;


// ---------------------------------------------------------------------------
// SearchResultData
// Bundles data passed between the search thread and the main thread.
// Contains raw HTML, parsed results, and metadata about search success.
// ---------------------------------------------------------------------------
typedef struct {
    AppWidgets *w;        // Widget references
    GList *results;       // List of RecipeInfo* structures representing matched recipes
    char *status_message; // Human-readable status message (e.g., "No results")
    gboolean success;     // TRUE if search completed successfully and results were found
    char *url;            // Final search URL used
    char *html;           // Raw HTML of the search results
    GumboOutput *output;  // Parsed DOM output from Gumbo parser
} SearchResultData;


// ---------------------------------------------------------------------------
// RecipeInfo
// Represents a single recipe result, including match details for display styling.

// ---------------------------------------------------------------------------
typedef struct {
    char *title;    // Recipe title
    char *url;      // Recipe URL
    gboolean perfect_match;  // TRUE if title fully matches the quoted search
    gboolean partial_match;  // TRUE if title partially matches the search
    int matched_tokens;  // Number of tokens (words) matched in the title
    int total_tokens;  // Total tokens found in the input recipe search term
} RecipeInfo;


// ---------------------------------------------------------------------------
// InsertAnimationData
// Manages insertion/removal animations for recipe listbox items.
// ---------------------------------------------------------------------------
typedef struct {
    GtkListBox *listbox;       // Target listbox
    GQueue *recipe_queue;      // Queue of RecipeInfo* to insert
    GQueue *partial_buttons;   // Queue of listbox buttons for partial matches

} InsertAnimationData;


// ---------------------------------------------------------------------------
// SiteParserFunc
// Function type for parsing HTML pages from a recipe site.
// Populates 'out' with extracted recipe results, and
// uses 'link_set' to avoid duplicates, and uses 'search_term' for context.


// ---------------------------------------------------------------------------
typedef void (*SiteParserFunc)(
    GumboNode *root,
    GList **out,
    GHashTable *link_set,
    const char *search_term
);


// ---------------------------------------------------------------------------
// RecipeSiteInfo
// Metadata for supported recipe sites (name, parser, URL pattern, etc.).
// ---------------------------------------------------------------------------
typedef struct {
    const char *name;           // Site display name
    SiteParserFunc parse_site;  // Parser function for this site
    const char *url_pattern;    // Base URL with placeholder
    const char *query_param;    // Query parameter key (e.g., "q")
} RecipeSiteInfo;


// ---------------------------------------------------------------------------
// DependencyCheckFunc
// Function type for checking runtime dependencies during the splash screen phase.
// Returns TRUE if the dependency check succeeds, FALSE otherwise.
// ---------------------------------------------------------------------------
typedef gboolean (*DependencyCheckFunc)(GtkWindow *parent);


// ---------------------------------------------------------------------------
// SplashScreenCheckContext
// Context data passed to idle callback for dependency checks during splash.
// Encapsulates environment state so callbacks remain modular and stateless.
// ---------------------------------------------------------------------------
typedef struct {
    GtkWindow *splash_window;       // Splash screen window
    DependencyCheckFunc check_func; // Dependency check function
} SplashScreenCheckContext;


// ===========================================================================
// Parser Memory Management
// ===========================================================================

// Configuration constants for parser RAM allocation
#define DEFAULT_MEMORY_PARSER_SIZE      (128 * 1024)   // 128 KB
#define LOW_CAPACITY_PARSER_RAM_KB      (16 * 1024)    // 16 KB (systems <128 MB RAM)
#define MID_CAPACITY_PARSER_RAM_KB      (64 * 1024)    // 64 KB (systems 128–512 MB RAM)
#define HIGH_CAPACITY_PARSER_RAM_KB     (256 * 1024)   // 256 KB (systems >512 MB RAM)


// ---------------------------------------------------------------------------
// MemoryBlock
// Dynamic buffer used by parsers for temporary/intermediate data.
// ---------------------------------------------------------------------------
typedef struct {
    char *data;       // Pointer to allocated buffer
    size_t size;      // Bytes currently used
    size_t capacity;  // Total allocated size
} MemoryBlock;

// Global parser buffer instance
MemoryBlock parser_buffer = { NULL, 0, DEFAULT_MEMORY_PARSER_SIZE };


// ===========================================================================
// Forward Declarations (Function Prototypes)
// ===========================================================================

// ---------------------------------------------------------------------------
// Main Entry Point
// ---------------------------------------------------------------------------
int main(int argc, char *argv[]);

// ---------------------------------------------------------------------------
// Memory Helpers
// ---------------------------------------------------------------------------

// Low-level memory and buffer management for downloads and parser processing.
// These are called early in main() for parser buffers and libcurl support.

// Returns amount of free memory available in the run-time system
static size_t get_free_memory(void);

// libcurl write callback storing data in memory buffer
static size_t memory_write_callback(void *contents, size_t sz, size_t nm, void *mem_block_ptr);

// Returns initial buffer capacity based on system RAM
static size_t detect_initial_capacity(void);

// ---------------------------------------------------------------------------
// Playwright / Software Package Installation Status
// ---------------------------------------------------------------------------

// Called early in main() to ensure runtime software dependencies are met.

// Checks if all software package dependencies are OK
static bool software_package_dependencies_OK(void);

// Creates splash window with dependency checks
GtkWidget* create_splash_window_with_software_checks(DependencyCheckFunc check_func);

// Writes a runtime software dependency marker
static void write_runtime_software_dependency_marker(void);

// Destroys a GTK window later (idle)
static gboolean destroy_window_later(gpointer data);

// Runs software dependency checks in idle loop
static gboolean run_dependency_check_idle(gpointer user_data);

// Checks if Node.js is installed
static gboolean check_node_installed_gtk(GtkWindow *parent, gchar **error_msg);

// Checks if Playwright browsers are installed
static gboolean check_playwright_browsers_gtk(GtkWindow *parent G_GNUC_UNUSED, gchar **error_msg);

// Checks if required JS dependencies are installed
static gboolean check_js_dependencies_gtk(GtkWindow *parent);

// Callback for close button
static void on_close_button_clicked(GtkWidget *button G_GNUC_UNUSED, gpointer user_data);

// Checks if npm package is installed
static gboolean check_npm_package_installed_gtk(GtkWindow *parent, const char *package, gchar **error_msg);

// Gets the path to the runtime dependency marker file
static char* get_dependency_marker_path(void);


// ---------------------------------------------------------------------------
// GTK UI Callbacks and Helpers
// ---------------------------------------------------------------------------

// Called after memory setup and dependency checks; sets up GTK window, widgets, and callbacks

// Loads CSS styles into the app
static void load_app_css_styles(void);

// Registers CSS data
static void register_css_styles(const gchar *css_data);

// Callback when search button is clicked
static void initialize_on_search(GtkButton *btn, gpointer ud);

// Thread function for performing search
static gpointer search_thread_func(gpointer data);

// Updates progress bar periodically
static gboolean pulse_progress_bar(gpointer data);

// Called when search completes
static gboolean search_complete_cb(gpointer data);

// Clears current recipe results from listbox
static void clear_recipe_results(GtkWidget *listbox);

// Blocks scroll event if needed
static gboolean block_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);

// Enables or disables UI widgets
static void set_ui_enabled(const AppWidgets *w, gboolean enabled);

// Sets mouse cursor for widget
static void set_cursor(GtkWidget *widget, GdkCursorType cursor_type);

// Callback for when a recipe item is clicked
static void on_recipe_clicked(GtkWidget *btn, gpointer user_data G_GNUC_UNUSED);

// Focuses entry field in idle loop
static gboolean focus_entry_idle(gpointer user_data);

// Displays search results in listbox
static void show_results(GtkWidget *listbox_widget, GList *links, const char *search_term, QuoteStatus quote_status);

// Inserts "Next" button in listbox
static gboolean insert_next_button(gpointer user_data);

// Adds "visible" CSS class to widget
static gboolean add_visible_class(gpointer widget);

// Callback when window is realized
static void on_window_realize(GtkWidget *widget, gpointer user_data);

// ---------------------------------------------------------------------------
// Networking and Download Helpers
// ---------------------------------------------------------------------------

// Called by parser and UI routines to fetch HTML content
static char* download_html(const char *url);

// ---------------------------------------------------------------------------
// Parser Helper Utilities
// ---------------------------------------------------------------------------

// Functions used by the site parsers for text normalization, tokenization, and link extraction

// Trims leading/trailing whitespace
static void trim_whitespace(const char *src, char *dest, size_t dest_size);

// URL-encodes a string
static char *url_encode(const char *str);

// Adds a new link to output list if not already present
static void add_link(GList **out, const char* title, const char* base_url, const char* href, GHashTable *link_set);

// Converts plural to singular
static void singularize(const char *src, char *dst, size_t dstlen);

// Capitalizes each word in a string
static void capitalize_each_word(char *str);

// Sanitizes string for safe display
static char *sanitize_string(const char *str);

// Splits title and digits
static char *split_title_and_digits(const char *title);

// Extracts quoted terms from search query
char *extract_quoted_terms(const char *search_term);

// Extracts quoted phrases into a list
static GList* extract_quoted_phrases(const char *search_term);

// Normalizes quotes in UTF-8 string
static void normalize_quotes_utf8(char *str);

// Returns TRUE if word is a stop word
static gboolean is_stop_word(const char *word);

// Tokenizes phrase and filters out stop words
GList *tokenize_and_filter_stop_words(const char *phrase);

// Detects whether the search term has quotes
static QuoteStatus detect_quote_status(const char *search_term);

// Checks if haystack contains needle (case-insensitive)
static bool contains_word_case_insensitive(const char *haystack, const char *needle);

// Converts slug to human-readable title
static void slug_to_title(const char *slug, char *out, size_t out_size);

// Extracts anchor text from Gumbo node
static const char* extract_anchor_text(GumboNode *node);

// Extracts text content from a Gumbo node (on the saveur.com website)
static void extract_saveur_text(GumboNode *node, GString *out);

// Another libcurl-compatible write callback (saveur)
static size_t write_saveur_callback(void *contents, size_t size, size_t nmemb, void *userp);

// Searches for links in a Gumbo node (saveur)
static void search_for_saveur_links(GumboNode *node, GList **out, GHashTable *link_set);


// ---------------------------------------------------------------------------
// HTML Recipe Parsers
// ---------------------------------------------------------------------------
// Site-specific parsers for extracting recipe links from different websites.
// Each parser is called after the HTML content is downloaded and
// preprocessed.
// Some parsers include Playwright handling or fallback logic to manage
// missing data or errors.
// ---------------------------------------------------------------------------

static void parse_allrecipes(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_bbcgoodfood(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_bonappetit(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_budgetbytes(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_chowhound(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_cooksillustrated(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_delish(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_eatingwell(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_epicurious_wrapper(GumboNode *root, GList **out, GHashTable *link_set, const char *search_term);
static void parse_epicurious(GumboNode *node, GList **out, GHashTable *link_set, const char *search_term, bool *found_any);
static void parse_food52(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_foodnetwork(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_thekitchn(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_nyt(GumboNode *root, GList **links, GHashTable *link_set, const char *search_term);
static void parse_saveur(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_seriouseats(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_simplyrecipes(GumboNode *n, GList **out, GHashTable *link_set, const char *search_term);
static void parse_smittenkitchen(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_spruceeats(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_tasteofhome(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term);
static void parse_yummlyrecipes(GumboNode *n, GList **out, GHashTable *link_set, const char *search_term);

// Generic fallback link
static void insert_fallback_link(GtkWidget *listbox, const char *url, const char *description);


// ==========================================================================
// ==========================================================================


// ---------------------------------------------------------------------------
// Recipe Site Table stores information for multiple recipe websites including:
//   1. Recipe website name
//   2. Parser function name
//   3. URL string (e.g., https://www.allrecipes.com/search/results/?wt=%s")
//   4. Query parameter placeholder (e.g., ?wt=)
// ---------------------------------------------------------------------------

const RecipeSiteInfo g_recipe_site_table[] = {
    { "AllRecipes", parse_allrecipes, "https://www.allrecipes.com/search/results/?wt=%s", "?wt=" },
    { "BBC Good Food", parse_bbcgoodfood, "https://www.bbcgoodfood.com/search?q=%s", "?q=" },
    { "Bon Appetit", parse_bonappetit, "https://www.bonappetit.com/search/%s", "%s" },
    { "Budget Bytes", parse_budgetbytes, "https://www.budgetbytes.com/?s=%s", "?s=" },
    { "Chowhound", parse_chowhound, "https://www.chowhound.com/search?query=%s", "?query=" },
    { "Cooks Illustrated / America's Test Kitchen", parse_cooksillustrated, "https://www.cooksillustrated.com/search?q=%s", "?q=" },
    { "Delish", parse_delish, "https://www.delish.com/search/%s/", "%s" },
    { "EatingWell", parse_eatingwell, "https://www.eatingwell.com/search/?q=%s", "?q=" },
    { "Epicurious", parse_epicurious_wrapper, "https://www.epicurious.com/search/%s", "%s" },
    { "Food52", parse_food52, "https://food52.com/search?q=%s", "?q=" },
    { "Food Network", parse_foodnetwork, "https://www.foodnetwork.com/search/%s-", "%s-" },
    { "NY Times Cooking", parse_nyt, "https://cooking.nytimes.com/search?q=%s", "?q=" },
    { "The Kitchn", parse_thekitchn, "https://www.thekitchn.com/search?q=%s", "?q=" },
    { "Saveur", parse_saveur, "https://www.saveur.com/search/%s/", "%s" },
    { "Serious Eats", parse_seriouseats, "https://www.seriouseats.com/search?q=%s", "?q=" },
    { "Simply Recipes", parse_simplyrecipes, "https://www.simplyrecipes.com/search?q=%s", "?q=" },
    { "Smitten Kitchen", parse_smittenkitchen, "https://smittenkitchen.com/?s=%s", "?s=" },
    { "The Spruce Eats", parse_spruceeats, "https://www.thespruceeats.com/search?q=%s", "?q=" },
    { "Taste of Home", parse_tasteofhome, "https://www.tasteofhome.com/search/index?search=%s", "?search=" },
    { "Yummly", parse_yummlyrecipes, "https://www.yummlyrecipes.com/?q=%s", "?q=" }
};



// ==========================================================================
// ==========================================================================



// This main function initializes a cross-platform GTK UI for both Windows
// and macOS.
// Fonts, padding, and widget sizes are deliberately set larger to enhance
// readability and improve touch usability across desktops, laptops, and
//  touchscreen devices.
// Custom GTK CSS styling is applied to ensure a clean, accessible, and
// user-friendly interface.

int main(int argc, char *argv[]) {

    // Initialize GTK for GUI and event handling
    gtk_init(&argc, &argv);

    // Set up network SSL, DNS, sockets
    curl_global_init(CURL_GLOBAL_ALL);

    // Set up network: initialize SSL, DNS, and socket support
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        fprintf(stderr, "Error: curl_global_init() failed to initialize network support\n");
        return 1;
    }

    // Check software dependencies only if not already done successfully
    if (!software_package_dependencies_OK()) {
        printf("RUNNING APP SOFTWARE DEPENDENCY CHECK ...\n");
        if (!create_splash_window_with_software_checks(check_js_dependencies_gtk)) {
            curl_global_cleanup();
            return 1; // Dependency checks failed
        }
        write_runtime_software_dependency_marker();
    }

    // Setup parser buffer memory
    parser_buffer.capacity = detect_initial_capacity();
    printf("INITIAL RECIPE PARSER MEMORY BUFFER CAPACITY SET TO: %zu bytes\n", parser_buffer.capacity);
    parser_buffer.data = malloc(parser_buffer.capacity);
    if (!parser_buffer.data) {
        fprintf(stderr, "Failed to allocate parser buffer of size %zu\n", parser_buffer.capacity);
        return 1;
    }
    parser_buffer.size = 0;
    fprintf(stdout, "PARSER BUFFER ALLOCATED AT: %p, SIZE:  %zu bytes\n",
            (void *)parser_buffer.data, parser_buffer.capacity);

    // Load GTK CSS Styling
    load_app_css_styles();

    // Initialize UI main window and layout
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "The Recipe Finder  (C) 2025 JM");
    gtk_window_set_default_size(GTK_WINDOW(win), 780, 660);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    // Create text search entry box
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter a recipe search term");
    gtk_style_context_add_class(gtk_widget_get_style_context(entry), "search-entry");
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    // Create recipe site combo box and populate it with site names
    // (auto-calculates the array size)
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(combo), "site-combo");

    // Add recipe site names from the g_recipe_site_table array
    size_t n_sites = sizeof(g_recipe_site_table) / sizeof(g_recipe_site_table[0]);
    for (size_t i = 0; i < n_sites; ++i) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), g_recipe_site_table[i].name);
    }

    // Set the first website link as the default selection
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    // Apply custom styling to the combo box's child widget as well
    GtkWidget *combo_child = gtk_bin_get_child(GTK_BIN(combo));
    if (combo_child) {
        gtk_style_context_add_class(gtk_widget_get_style_context(combo_child), "site-combo");
    }

    // Add the combo box to the vertical layout
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0);

    // Create status label area to show app messages
    GtkWidget *status_label = gtk_label_new("");
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(status_label), "status-label");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);

    // Create clickable search button
    GtkWidget *btn = gtk_button_new_with_label("Click to Search for Recipes");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "search-button");
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);

    // Create pulsating progress bar area
    GtkWidget *progress = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), FALSE);
    gtk_widget_set_vexpand(progress, FALSE);
    gtk_widget_set_hexpand(progress, TRUE);
    gtk_widget_set_no_show_all(progress, TRUE);
    gtk_widget_hide(progress);
    gtk_box_pack_start(GTK_BOX(vbox), progress, FALSE, FALSE, 0);

    // Add recipe link to the combo box
    GtkWidget *scr = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scr), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    GtkWidget *listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scr), listbox);
    gtk_box_pack_start(GTK_BOX(vbox), scr, TRUE, TRUE, 0);

    // AppWidgets struct
    AppWidgets *w = g_malloc(sizeof(*w));
    w->entry = entry;
    w->combo = combo;
    w->listbox = listbox;
    w->status_label = status_label;
    w->search_button = btn;
    w->progress_bar = progress;
    w->pulse_timer_id = 0;

    // Connect GTK widget signals to their respective callback functions
    g_signal_connect(combo, "scroll-event", G_CALLBACK(block_scroll), NULL);
    g_signal_connect(listbox, "scroll-event", G_CALLBACK(block_scroll), NULL);
    g_signal_connect(btn, "clicked", G_CALLBACK(initialize_on_search), w);
    g_signal_connect(win, "show", G_CALLBACK(on_window_realize), entry);

    // Show all GTK widgets in the window
    gtk_widget_show_all(win);

    // Start the GTK main event loop
    gtk_main();

    // Final cleanup to release all allocated resources before exit
    curl_global_cleanup();
    g_free(w);
    free(parser_buffer.data);
    parser_buffer.data = NULL;
    parser_buffer.size = 0;
    parser_buffer.capacity = 0;

    printf("\n[INFO]: recipe_finder app is exiting normally.\n\n");

    return 0;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// Returns the amount of currently available system memory (RAM) in bytes.
// This function is useful for dynamically sizing memory buffers or limiting
// memory usage based on the system's available RAM.
// Platform-specific behavior:
//   - Windows: Uses GlobalMemoryStatusEx to get available physical memory.
//   - macOS: Returns free + inactive memory via vm_statistics. Inactive memory
//     is memory not actively used but available for allocation.
//   - Linux: Uses sysinfo.freeram (multiplied by mem_unit) to calculate
//     available RAM.
//   - Other platforms: Returns 0 because no method is defined.
// Prints the free memory in MB for clarity.  If a platform-specific
// API call fails, the function returns 0 and prints a warning. 

static size_t get_free_memory(void) {

#if defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);

    if (GlobalMemoryStatusEx(&status)) {
        size_t free_bytes = (size_t)status.ullAvailPhys;
   //     fprintf(stderr, "[INFO]: Free memory on Windows: %.2f MB\n", free_bytes / 1024.0 / 1024.0);
        return free_bytes;
    }

    fprintf(stderr, "[WARNING]: Failed to retrieve free memory on Windows. Returning 0.\n");
    return 0;

#elif defined(__APPLE__)
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;

    if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) == KERN_SUCCESS) {
        size_t page_size;
        host_page_size(mach_host_self(), &page_size);
        size_t free_mem = (size_t)vmstat.free_count * page_size;
        size_t inactive_mem = (size_t)vmstat.inactive_count * page_size;
        size_t total_available = free_mem + inactive_mem;

  //      fprintf(stderr, "[INFO]: Free + inactive memory on macOS: %.2f MB\n", total_available / 1024.0 / 1024.0);
        return total_available;
    }

    fprintf(stderr, "[WARNING]: Failed to retrieve free memory on macOS. Returning 0.\n");
    return 0;


#elif defined(__linux__)
    struct sysinfo info;

    if (sysinfo(&info) == 0) {
        size_t free_bytes = (size_t)info.freeram * info.mem_unit;
  //      fprintf(stderr, "[INFO]: Free memory on Linux: %.2f MB\n", free_bytes / 1024.0 / 1024.0);
        return free_bytes;
    }

    fprintf(stderr, "[WARNING]: Failed to retrieve free memory on Linux. Returning 0.\n");
    return 0;

#else
    fprintf(stderr, "[*** WARNING ***]: get_free_memory() not implemented on this platform. Returning 0.\n");
    return 0;
#endif

}


// ---------------------------------------------------------------------------


/* Memory Helper for Recipe Parsers ------------------------------------------
 * Callback used by libcurl to write downloaded data chunks into a dynamically
 * growing memory buffer. Doubles capacity as needed, checking memory safety.
 * Assumes initial_parser_ram_capacity is set before first use.
 * Memory allocation strategy:
 * - Uses realloc to manage the dynamic memory buffer (m->data).
 * - When m->data is NULL, realloc(NULL, size) behaves like malloc(size),
 *   allocating a new memory block for the first time.
 * - On subsequent calls, realloc attempts to resize the existing block in-place.
 *   If that's not possible, realloc allocates a new larger block, copies old
 *   data, frees the old block, and returns the new pointer.
 * - calloc is not used because zero-initialization of the entire buffer is
 *   unnecessary. After each write, the buffer is explicitly null-terminated
 *   at the end of valid data, ensuring string safety without the overhead
 *   of zeroing unused buffer space, especially important as the buffer grows.
 * - The buffer address (m->data) may change after realloc, which is tracked
 *   and reported for debugging and performance insights.
 * - This approach simplifies memory management by combining allocation and
 *   resizing in one step, with safety checks for max sizes and memory limits.
 */

#define MAX_DOWNLOAD_SIZE (32 * 1024 * 1024)  // 32 MB max capacity for sanity

static size_t memory_write_callback(void *contents, size_t sz, size_t nm, void *mem_block_ptr) {
    size_t realsize = sz * nm;
    MemoryBlock *m = (MemoryBlock *)mem_block_ptr;

    if (!m) {
        fprintf(stderr, "memory_write_callback: MemoryBlock pointer is NULL\n");
        return 0;
    }

    if (m->data && m->capacity == 0) {
        fprintf(stderr, "memory_write_callback: Inconsistent MemoryBlock  --  data exists but capacity is zero\n");
        return 0;
    }

    if (realsize > SIZE_MAX - m->size - 1) {
        fprintf(stderr, "memory_write_callback: size overflow detected  --  incoming chunk too large\n");
        return 0;
    }

    size_t required_size = m->size + realsize + 1;

    if (required_size > MAX_DOWNLOAD_SIZE) {
        fprintf(stderr, "memory_write_callback: Exceeded maximum allowed download size (%d MB)\n",
                MAX_DOWNLOAD_SIZE / (1024 * 1024));
        return 0;
    }

    // Resize if needed
    if (required_size > m->capacity) {
        size_t old_capacity = m->capacity;
        size_t new_capacity = (m->capacity > 0) ? m->capacity : DEFAULT_MEMORY_PARSER_SIZE;

        if (new_capacity == 0) {
            fprintf(stderr, "memory_write_callback: initial_parser_ram_capacity not set!\n");
            return 0;
        }

        while (new_capacity < required_size) {
            if (new_capacity > MAX_DOWNLOAD_SIZE / 2) {
                new_capacity = MAX_DOWNLOAD_SIZE;
                break;
            }
            new_capacity *= 2;
        }

        size_t free_mem = get_free_memory();

        if (free_mem < new_capacity) {
            fprintf(stderr, "memory_write_callback: Insufficient free memory to expand buffer to %zu bytes (free memory: %zu bytes)\n",
                    new_capacity, free_mem);
            return 0;
        }

        char *new_data = realloc(m->data, new_capacity);
        if (!new_data) {
            fprintf(stderr, "memory_write_callback: Failed to grow buffer to %zu bytes\n", new_capacity);
            return 0;
        }

        m->data = new_data;
        m->capacity = new_capacity;

        /*
         * Display Memory Allocation Growth Status
         *   Note on memory reporting:
         * - Small reallocations may not visibly reduce system memory due to
         *   OS caching.
         * - The OS virtualizes memory and reuses internal allocator pools, so
         *   realloc / malloc calls may not noticeably affect system-level free
         *   memory readings, especially for small (KB-scale) allocations used
         *   by the recipe parsers.
         * - The free memory value logged here is coarse-grained and may appear
         *   unchanged even though memory is actively being allocated and used.
         * - Cumulative Bytes Needed represents the total buffer size required to
         *   hold all downloaded data so far, including the current chunk. This is
         *   not just the size of the current chunk but the entire memory footprint
         *   accumulated at this point.
         * - Buffer Address is the actual memory address of the dynamically allocated
         *   buffer (m->data) used to store the downloaded recipe content. If realloc
         *   moves the buffer during resizing (because in-place growth isn't
         *   possible), the address will change. This allows tracking of buffer
         *   relocation and helps with debugging and performance tuning.
         */

        printf("\nRECIPE PARSER DYNAMIC MEMORY ALLOCATION STATUS:\n\n"
               " >>> WEBSITE:                 %s\n"
               "     Capacity Before Resize:    %.1f KB\n"
               "     Cumulative Bytes Needed:   %.1f KB\n"
               "     Capacity After Resize:     %.1f KB\n"
               "     Buffer Address:            %p\n"
               "     Detected Free Memory:      %.2f MB\n",
               g_current_website_name ? g_current_website_name : "(unknown)",
               (double)old_capacity / 1024.0,
               (double)required_size / 1024.0,
               (double)new_capacity / 1024.0,
               (void *)m->data,
               (double)free_mem / (1024.0 * 1024.0));
        printf("\n------------------------------------------------\n");
    }

    memcpy(m->data + m->size, contents, realsize);
    m->size += realsize;
    m->data[m->size] = '\0';  // Null-terminate to keep it string-safe

    return realsize;
}


// ==============
// ==================
// ===============


/* ---------------------------------------------------------------------------
* Playwright / Software Package Installation Status
* ---------------------------------------------------------------------------
*
* Verifies at startup via a GTK splash screen if required runtime software
* dependencies are installed. 
* If all dependencies are verified, a marker file is written, allowing
* future launches to bypass this check for faster app startup. 
* If any dependency is missing or invalid, the user is alerted via
* GTK dialogs, and the splash remains open until manually dismissed.
*
* Specific software dependencies checked:
*   - Node.js (must be in the system PATH so the app can invoke Node and npm)
*   - Global npm packages: playwright, cheerio, axios
*   - Playwright browser install (via 'npx playwright install', includes Chromium)
*
*   Note: Global npm packages and Playwright browsers do not need PATH
*   entries; they are accessed via Node scripts.
*
* For implementation details, see:
*   - check_node_installed_gtk()
*   - check_npm_package_installed_gtk()
*   - check_playwright_browsers_gtk()
*
* ---------------------------------------------------------------------------
*/

// Checks if all software package dependencies are OK
static bool software_package_dependencies_OK(void) {
    char *path = get_dependency_marker_path();
    bool exists = g_file_test(path, G_FILE_TEST_EXISTS);
    g_free(path);
    return exists;
}

// ---------------------------------------------------------------------------


// Creates splash window with dependency checks
GtkWidget* create_splash_window_with_software_checks(DependencyCheckFunc check_func) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Starting Recipe Finder...");
    gtk_window_set_default_size(GTK_WINDOW(window), 440, 200);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *label1 = gtk_label_new("RECIPE FINDER is checking for required software...");
    gtk_widget_set_halign(label1, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), label1, FALSE, FALSE, 0);

    GtkWidget *label2 = gtk_label_new("Software package dependencies being verified:");
    gtk_widget_set_halign(label2, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), label2, FALSE, FALSE, 0);

    GtkWidget *dep_list = gtk_label_new(
        "  - Node.js runtime\n"
        "  - npm packages: playwright, cheerio, axios\n"
        "  - Playwright browser install: Chromium");
    gtk_widget_set_halign(dep_list, GTK_ALIGN_START);
    gtk_label_set_xalign(GTK_LABEL(dep_list), 0.0f);
    gtk_box_pack_start(GTK_BOX(box), dep_list, FALSE, FALSE, 0);

    GtkWidget *spinner = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(spinner));
    gtk_widget_set_halign(spinner, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), spinner, FALSE, FALSE, 0);
    gtk_widget_show(spinner);

    g_object_set_data(G_OBJECT(window), "spinner", spinner);

    GtkWidget *status_label = gtk_label_new("Please wait...");
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 0);

    GtkWidget *close_button = gtk_button_new_with_label("Close");
    gtk_widget_set_sensitive(close_button, FALSE);
    gtk_box_pack_start(GTK_BOX(box), close_button, FALSE, FALSE, 0);
    g_signal_connect(close_button, "clicked", G_CALLBACK(on_close_button_clicked), window);

    gtk_widget_show_all(window);

    g_object_set_data(G_OBJECT(window), "status_label", status_label);
    g_object_set_data(G_OBJECT(window), "close_button", close_button);

    SplashScreenCheckContext *data = g_new(SplashScreenCheckContext, 1);
    data->splash_window = GTK_WINDOW(window);
    data->check_func = check_func;

    g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, run_dependency_check_idle, data, NULL);
    gtk_main();

    return window;
}


// ---------------------------------------------------------------------------


// Writes a runtime software dependency marker file
static void write_runtime_software_dependency_marker(void) {
    char *path = get_dependency_marker_path();
    printf("  ATTEMPTING TO WRITE THE RECIPE APP'S SOFTWARE DEPENDENCY MARKER FILE AT:\n    %s\n", path);
    fflush(stdout);

    FILE *f = fopen(path, "w");
    if (f) {
        time_t now = time(NULL);
        struct tm *tm_now = localtime(&now);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_now);

        fprintf(f, "recipe_finder App --  Software dependencies check completed successfully: Timestamp: %s\n", time_str);
        fclose(f);

        printf("        MARKER FILE WRITTEN SUCCESSFULLY.\n");
        printf("          SOFTWARE PACKAGE DEPENDENCIES WERE VERIFIED SUCCESSFULLY.\n\n");
        fflush(stdout);
    } else {
        perror("ERROR: Failed to write software dependency marker file");
        fflush(stdout);
    }

    g_free(path);
}


// ---------------------------------------------------------------------------


// Destroys a GTK window later (idle)
static gboolean destroy_window_later(gpointer data) {
    GtkWidget *window = GTK_WIDGET(data);
    gtk_widget_destroy(window);
    return G_SOURCE_REMOVE;
}


// ---------------------------------------------------------------------------


// Runs software dependency checks in idle loop
static gboolean run_dependency_check_idle(gpointer user_data) {
    SplashScreenCheckContext *data = (SplashScreenCheckContext *)user_data;
    GtkWindow *window = data->splash_window;
    DependencyCheckFunc check_func = data->check_func;

    GtkLabel *status_label = GTK_LABEL(g_object_get_data(G_OBJECT(window), "status_label"));
    GtkWidget *close_button = GTK_WIDGET(g_object_get_data(G_OBJECT(window), "close_button"));
    GtkSpinner *spinner = GTK_SPINNER(g_object_get_data(G_OBJECT(window), "spinner"));

    gboolean success = check_func(window);

    if (spinner) gtk_spinner_stop(spinner);
    if (status_label) gtk_label_set_text(status_label,
        success ? "Required software has been checked." :
                  "Dependency check failed. See error messages.");
    if (close_button) gtk_widget_set_sensitive(close_button, TRUE);

    if (success) {
        guint timeout_id = g_timeout_add_seconds(5, destroy_window_later, GTK_WIDGET(window));
        g_object_set_data(G_OBJECT(window), "destroy_timeout_id", GUINT_TO_POINTER(timeout_id));
    }

    g_free(data);
    return G_SOURCE_REMOVE;
}


// ---------------------------------------------------------------------------


// Checks if Node.js is installed
static gboolean check_node_installed_gtk(GtkWindow *parent G_GNUC_UNUSED, gchar **error_msg) {
    GError *error = NULL;
    gchar *output = NULL;

    if (!g_spawn_command_line_sync("node -v", &output, NULL, NULL, &error)) {
        *error_msg = g_strdup("Node.js not found.\nPlease install Node.js from https://nodejs.org/ and ensure 'node' is in your PATH.");
        g_clear_error(&error);
        g_free(output);
        return FALSE;
    }

    if (!output || output[0] != 'v') {
        *error_msg = g_strdup_printf("Unexpected output from 'node -v': %s\nPlease ensure Node.js is properly installed.", output ? output : "NULL");
        g_free(output);
        return FALSE;
    }

    g_free(output);
    return TRUE;
}


// ---------------------------------------------------------------------------


// Checks if Playwright browsers are installed
static gboolean check_playwright_browsers_gtk(GtkWindow *parent G_GNUC_UNUSED, gchar **error_msg) {
    GError *error = NULL;
    gchar *output = NULL;

    if (!g_spawn_command_line_sync("playwright --version", &output, NULL, NULL, &error)) {
        *error_msg = g_strdup(
            "Playwright package not found globally.\n"
            "Please install it by running:\n"
            "  npm install -g playwright\n"
            "This requires administrator privileges and internet access.");
        g_clear_error(&error);
        g_free(output);
        return FALSE;
    }
    g_free(output);

    if (!g_spawn_command_line_sync("npx playwright install", &output, NULL, NULL, &error)) {
        *error_msg = g_strdup(
            "Failed to run 'npx playwright install'.\n"
            "Make sure 'npx' is available and you have network connectivity.");
        g_clear_error(&error);
        g_free(output);
        return FALSE;
    }

    if (output) {
        gchar *output_lower = g_ascii_strdown(output, -1);
        const gchar *error_keywords[] = { "error", "failed", "missing", "not found" };
        gboolean found_error = FALSE;
        for (size_t i = 0; i < G_N_ELEMENTS(error_keywords); i++) {
            if (g_strstr_len(output_lower, -1, error_keywords[i])) {
                found_error = TRUE;
                break;
            }
        }
        g_free(output_lower);
        g_free(output);
        if (found_error) {
            *error_msg = g_strdup(
                "Playwright browser installation reported an error.\n"
                "Check terminal output or run 'npx playwright install' manually.");
            return FALSE;
        }
    } else {
        *error_msg = g_strdup("No output from 'npx playwright install'.\nPlease verify your Playwright installation.");
        return FALSE;
    }

    return TRUE;
}


// ---------------------------------------------------------------------------


// Checks if required JS dependencies are installed
static gboolean check_js_dependencies_gtk(GtkWindow *parent) {
    gchar *error_msg = NULL;

    if (!check_node_installed_gtk(parent, &error_msg)) {
        GtkWidget *dlg = gtk_message_dialog_new(parent,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", error_msg);
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        g_free(error_msg);
        return FALSE;
    }

    const char *deps[] = {"playwright", "cheerio", "axios"};
    for (size_t i = 0; i < G_N_ELEMENTS(deps); i++) {
        if (!check_npm_package_installed_gtk(parent, deps[i], &error_msg)) {
            GtkWidget *dlg = gtk_message_dialog_new(parent,
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", error_msg);
            gtk_dialog_run(GTK_DIALOG(dlg));
            gtk_widget_destroy(dlg);
            g_free(error_msg);
            return FALSE;
        }
    }

    if (!check_playwright_browsers_gtk(parent, &error_msg)) {
        GtkWidget *dlg = gtk_message_dialog_new(parent,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", error_msg);
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        g_free(error_msg);
        return FALSE;
    }

    return TRUE;
}


// ---------------------------------------------------------------------------


// Callback for close button
static void on_close_button_clicked(GtkWidget *button G_GNUC_UNUSED, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    gpointer data = g_object_get_data(G_OBJECT(window), "destroy_timeout_id");
    if (data != NULL) {
        guint timeout_id = GPOINTER_TO_UINT(data);
        g_source_remove(timeout_id);
        g_object_set_data(G_OBJECT(window), "destroy_timeout_id", NULL);
    }
    gtk_widget_destroy(window);
}


// ---------------------------------------------------------------------------


// Checks if npm package is installed
static gboolean check_npm_package_installed_gtk(GtkWindow *parent G_GNUC_UNUSED, const char *package, gchar **error_msg) {
    gchar *cmd = g_strdup_printf("npm list -g %s --depth=0 --json", package);
    GError *error = NULL;
    gchar *output = NULL;

    if (!g_spawn_command_line_sync(cmd, &output, NULL, NULL, &error)) {
        *error_msg = g_strdup_printf("Failed to run npm to check for %s.\nMake sure npm is installed and in your PATH.", package);
        g_clear_error(&error);
        g_free(output);
        g_free(cmd);
        return FALSE;
    }
    g_free(cmd);

    if (!output || !g_strstr_len(output, -1, package)) {
        *error_msg = g_strdup_printf("JavaScript package '%s' is not installed globally.\nPlease install it with:\n  npm install -g %s", package, package);
        g_free(output);
        return FALSE;
    }
    g_free(output);
    return TRUE;
}


// ---------------------------------------------------------------------------


// Gets the path to the runtime dependency marker file
static char* get_dependency_marker_path(void) {
    const char *config_dir = g_get_user_config_dir();
    char *folder_path = g_build_filename(config_dir, "recipe_finder", NULL);
    g_mkdir_with_parents(folder_path, 0700);
    char *marker_path = g_build_filename(folder_path, ".recipe_finder_checked_ok", NULL);

    printf("RECIPE_FINDER SOFTWARE DEPENDENCIES MARKER FILE PATH IS:\n%s\n\n", marker_path);
    fflush(stdout);

    g_free(folder_path);
    return marker_path;
}


// ==============
// ==================
// ===============




// ------------------------------------------------------
// Platform Specific Memory Detection
// ------------------------------------------------------

// This function detects the system RAM and returns a recommended buffer size
// for recipe parser memory. It adapts memory usage based on system capability
// for making the app efficient on low-spec systems, and takes advantage of
// more RAM when available.
// It compares total_ram (bytes) against thresholds:
//   Expression       Meaning      Value (bytes)
//   128ULL << 20     128 MB       134,217,728
//   512ULL << 20     512 MB       536,870,912

static size_t detect_initial_capacity() {
    size_t total_ram = 0;

#if defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        total_ram = (size_t)status.ullTotalPhys;
    }
#elif defined(__APPLE__)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    int64_t ram = 0;
    size_t len = sizeof(ram);
    if (sysctl(mib, 2, &ram, &len, NULL, 0) == 0) {
        total_ram = (size_t)ram;
    }
#elif defined(__linux__)
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        total_ram = (size_t)info.totalram * info.mem_unit;
    }
#endif

if (total_ram == 0) {
    printf("Unable to detect system memory. Using default parser memory size.\n");
    return DEFAULT_MEMORY_PARSER_SIZE;
}

// Print detected system installed memory size in MB
printf("DETECTED INSTALLED SYSTEM MEMORY: %zu MB RAM\n", total_ram / (1024 * 1024));

if (total_ram < (128ULL << 20)) return LOW_CAPACITY_PARSER_RAM_KB;
else if (total_ram < (512ULL << 20)) return MID_CAPACITY_PARSER_RAM_KB;
else return HIGH_CAPACITY_PARSER_RAM_KB;
}



// ------------------------------------------------------
// ------------------------------------------------------


// Dynamically allocates memory (via realloc inside memory_write_callback)
// and stores the results in chunk.data.
// The combination of download_html + memory_write_callback fetches the
// entire HTML document from the web and creates a single, null-terminated
//  string containing it, no matter how big it is.

static char* download_html(const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    MemoryBlock chunk = {.data = NULL, .size = 0, .capacity = 0};

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/124.0.0.0 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_REFERER, url);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memory_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode rc = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK) {
        free(chunk.data);
        return NULL;
    }

    return chunk.data;  // Must be freed by caller
}


// ==================


// Adds a safe HTML link to the returned recipes using g_list_append.
// Uses GHashTable deduplication for unique recipe links.

static void add_link(GList **out, const char* title, const char* base_url, const char* href, GHashTable *link_set) {
    if (recipe_result_total >= MAX_RESULTS) {
        return;  // Limit reached, skip adding more recipe links
    }

    // Make a mutable copy of the title so we can format it
    char temp_title[512];
    snprintf(temp_title, sizeof(temp_title), "%s", title);

    // Capitalize each word in the title
    capitalize_each_word(temp_title);

    // Sanitize for HTML safety
    char *safe_title = sanitize_string(temp_title);
    char *safe_href = sanitize_string(href);

    // Build the full URL
    char *full_url = g_strdup_printf("%s%s", base_url, safe_href);

    // Add link if it's not a duplicate
    if (!g_hash_table_contains(link_set, full_url)) {
        char *entry = g_strdup_printf("%s\x1f%s", safe_title, full_url);
        *out = g_list_append(*out, entry);
        g_hash_table_add(link_set, full_url);
        recipe_result_total++;  // Count only successful additions
    } else {
        g_free(full_url);  // Discard duplicate
    }

    // Clean up
    g_free(safe_title);
    g_free(safe_href);
}


// ------------------------------



// Helper: Trim leading and trailing spaces
// Efficiently trims leading and trailing spaces from the input source, and
// copies the trimmed result into the destination, adding a null terminator.
// If the trimmed string is too long, it will be truncated safely.
//
// In most cases with typical recipe titles, no trimming is normally
// needed, and 'src' is copied immediately to the 'dest' output buffer.
//
// If trimming is needed, the function uses pointers to scan the input:
//   - It walks forward through input string 'src' to skip leading whitespace
//   - It walks backward through 'src' to ignore trailing whitespace
//
// Note: This app does not use in-place processing (src == dest), so the
// original string is not modified here.  This is safe for use in GTK apps
// because it does not modify 'src', does not perform dynamic memory
// allocation on the heap, and avoids freeing or interference with
// GTK-managed string ownership.

static void trim_whitespace(const char *src, char *dest, size_t dest_size) {

    // Defensive check: ensure valid pointers and non-zero destination size.
    if (!src || !dest || dest_size == 0) {
        if (dest_size > 0) {
            dest[0] = '\0';  // Null-terminate dest if possible.
        }
        return;
    }

    // Walk forward from the start of 'src' to skip leading whitespace characters.
    const char *start = src;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // Walk backward from the end of 'src' to find the last non-whitespace character.
    const char *end = src + strlen(src);
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }

    size_t trimmed_len = (size_t)(end - start);

    // Fast path: No leading or trailing spaces, and src != dest.
    // Just copy src directly to dest and return early.
    if (start == src && *end == '\0') {
        if (src != dest) {
            strncpy(dest, src, dest_size - 1);
            dest[dest_size - 1] = '\0';  // Ensure null termination.
        }
        return;
    }

    // Now we know trimming was done, so copy the trimmed substring safely.

    // Shorten trimmed_len if it exceeds the destination buffer size.
    if (trimmed_len >= dest_size) {
        trimmed_len = dest_size - 1;
    }

    // Copy the trimmed section of src into dest.
    memcpy(dest, start, trimmed_len);

    // Null-terminate the destination string.
    dest[trimmed_len] = '\0';
}



// ------------------------------


// Helper to Capitalize Each Word In A String
static void capitalize_each_word(char *str) {
    bool capitalize_next = true;
    for (char *p = str; *p; ++p) {
        if (capitalize_next && isalpha((unsigned char)*p)) {
            *p = toupper((unsigned char)*p);
            capitalize_next = false;
        } else {
            *p = tolower((unsigned char)*p);
        }

        if (*p == ' ') {
            capitalize_next = true;
        }
    }
}


// ------------------------------



// Sanitize Input String:
// Removes control characters (except allowed printable ASCII bytes) to
// avoid crashes or blank recipe titles.
// Note: This approach may show two replacement question-mark
// characters (??) for a single accented character if the input
// contains invalid or multi-byte UTF-8 sequences.
// This simple method trades off perfect replacement of accented characters
// for stability and consistent, non-empty output suitable for UI display.
// Previous frustrating attempts to implement full UTF-8 normalization and
// accent-stripping logic were complicated by:
// - Difficulty reliably converting accented characters to plain ASCII
// - Invalid or malformed UTF-8 sequences causing normalization failures
// - Complex Unicode handling with longer execution times, resulting in empty
//   or blank output strings


static char *sanitize_string(const char *str) {
    GString *out = g_string_new(NULL);
    for (const unsigned char *p = (const unsigned char *)str; *p; ++p) {
        if (*p >= 32 && *p != 127) {
            g_string_append_c(out, *p);
        }
    }
    return g_string_free(out, FALSE);  // Returns newly allocated string
}


// ------------------------------



// Helper function that simplistically singularizes English recipe words and
// search terms to singular form.
// Checks against exceptions word list and phrase list which are always
//   preserved as-is, and applies singularization safely. 
// Uses trim_whitespace() to clean output.
// Summary:
// If src is NULL or dstlen == 0:
//    Writes an empty string ("") into dst (if space allows).
// If src matches one of the protected multi-word phrases (e.g., "cream cheese", "olive oil"):
//    Copies the phrase unchanged into dst.
// If src matches one of the protected words (e.g., "anchovies", "eggs"):
//     Copies the word unchanged into dst.
// If the word ends with "ies":
//    Converts it to end with "y" (e.g., "berries" → "berry").
// If the word ends with s (longer than 1 char):
//    Strips the final "s" (e.g., "cakes" → "cake").
// Otherwise:
//    Copies src unchanged into dst.

static void singularize(const char *src, char *dst, size_t dstlen) {
    if (!src || dstlen == 0) {
        if (dstlen > 0) dst[0] = '\0';
        return;
    }

    // Exceptions word list: always preserved as-is (plural or not)
    static const char *protected_recipe_words[] = {
        "anchovies",
        "bagels",
        "beans",
        "berries",
        "brownies",
        "buns",
        "carrots",
        "chaffles",
        "chips",
        "clams",
        "cookies",
        "crackers",
        "cupcakes",
        "dumplings",
        "eggs",
        "fries",
        "greens",
        "grits",
        "herbs",
        "lentils",
        "loaves",
        "meatballs",
        "muffins",
        "mussels",
        "nachos",
        "noodles",
        "nuts",
        "olives",
        "pancakes",
        "peppers",
        "pickles",
        "pies",
        "ribs",
        "sandwiches",
        "sausages",
        "scallops",
        "seeds",
        "shrimp",
        "snacks",
        "spaghetti",
        "spices",
        "sprouts",
        "sweets",
        "tacos",
        "treats",
        "vegetables",
        "veggies",
        "waffles",
        "wraps",
        "zoodles",
        NULL
    };

    // Protected multi-word recipe phrases where singularizing words
    // would distort meaning and thus should be preserved as-is.
    static const char *protected_recipe_phrases[] = {
        "apple cider",
        "apple slices",
        "baking powder",
        "baking soda",
        "bread crumbs",
        "brown rice",
        "brown sugar",
        "cocoa powder",
        "chocolate chips",
        "cooking oil",
        "corn flakes",
        "cream cheese",
        "cream of tartar",
        "cream sauce",
        "dark chocolate",
        "fried oysters",
        "french fries",
        "green beans",
        "green onions",
        "green peas",
        "hot chili",
        "hot dogs",
        "hot sauce",
        "lemon zest",
        "mixed nuts",
        "olive oil",
        "orange juice",
        "potato chips",
        "red onions",
        "red pepper",
        "soy sauce",
        "strawberry jam",
        "sweet chili",
        "sweet corn",
        "sweet potatoes",
        "vanilla extract",
        "whole wheat",
        NULL
    };

    char trimmed_src[256];
    trim_whitespace(src, trimmed_src, sizeof(trimmed_src));
    // Check for protected multi-word phrases (case-insensitive)
    for (int i = 0; protected_recipe_phrases[i]; ++i) {
        if (strcasecmp(trimmed_src, protected_recipe_phrases[i]) == 0) {
            snprintf(dst, dstlen, "%s", trimmed_src);
            return;
        }
    }
    // Check exceptions word list
    for (int i = 0; protected_recipe_words[i]; ++i) {
        if (strcasecmp(trimmed_src, protected_recipe_words[i]) == 0) {
            snprintf(dst, dstlen, "%s", trimmed_src);
            return;
        }
    }
    size_t len = strlen(trimmed_src);
    // Rule: convert "ies" to "y"
    if (len > 3 && strcasecmp(trimmed_src + len - 3, "ies") == 0) {
        snprintf(dst, dstlen, "%.*sy", (int)(len - 3), trimmed_src);
        return;
    }
    // Rule: remove trailing 's' if longer than 1 char
    if (len > 1 && (trimmed_src[len - 1] == 's' || trimmed_src[len - 1] == 'S')) {
        snprintf(dst, dstlen, "%.*s", (int)(len - 1), trimmed_src);
        return;
    }
    // Default: copy trimmed input as-is
    snprintf(dst, dstlen, "%s", trimmed_src);
}


// ------------------------------


// Rewrites recipe titles that have a trailing block of digits so they are
//   more human-friendly and visually separated.
// Splits a recipe title into two parts: the descriptive name and a trailing
//   numeric rating block (e.g., 1,234 Ratings).
// Uused when parsing recipe titles from websites where the site includes
//   numbers (ratings, votes, or counts) at the end of the title string
// without clean separation.
// It makes the recipe titles easier to read in the GTK listbox/buttons by
//   inserting " - " before the number block.
// If a number appears at the end of the title (with optional commas),
// it inserts " - " before the number block for readability.
// Special handling:
// - Leaves titles unchanged if numbers are embedded in words (e.g., "Rib1234").
// - Adds a space before an opening parenthesis if the number block is wrapped
// Returns a newly allocated string that must be freed by the caller.
// Examples:
//   "Delicious Chicken Soup1,234 Ratings" -->"Delicious Chicken Soup - 1,234 Ratings"
//   "Amazing Ribs(1,234 Ratings)" --> "Amazing Ribs ( - 1,234 Ratings)"
//   "Best Rib1234" --> "Rib1234"
//  "Taco Recipes" --> "Taco Recipes"
// What it does step-by-step:
//   1. Scan from the end of the string to find the last digit.
//        If no digits → return the original title unchanged.
//  2. Identify the whole block of digits (e.g., "1234" or "1,234") by
//      walking backward.
//  3. Safety check: If the digits are part of a word (like "Rib1234"),
//      don’t split ==> return unchanged.
//  4. Special case: If the digits are immediately after a parenthesis
//       (e.g., "Ribs(1,234 Ratings)"), insert a space before (.

static char *split_title_and_digits(const char *title) {
    if (!title) return NULL;

    size_t len = strlen(title);
    if (len == 0) return strdup("");

    // Step 1: Find the last digit in the string
    ssize_t end = -1;
    for (ssize_t i = len - 1; i >= 0; i--) {
        if (isdigit((unsigned char)title[i])) {
            end = i;
            break;
        }
    }

    if (end == -1) return strdup(title);  // No digits found

    // Step 2: Walk backwards to find the start of the digit block (including commas)
    ssize_t start = end;
    while (start > 0 && (isdigit((unsigned char)title[start - 1]) || title[start - 1] == ',')) {
        start--;
    }

    // Step 3: Avoid splitting embedded numbers (e.g., Rib1234)
    if (start > 0 && isalpha((unsigned char)title[start - 1])) {
        return strdup(title);
    }

    // Step 4: Check for tight parenthesis, e.g. "Ribs(1,234 Ratings)"
    // Insert a space before '(' if needed
    int insert_space_before_paren = 0;
    if (start > 0 && title[start - 1] == '(') {
        insert_space_before_paren = 1;
        start--;  // Include the '(' in the part1
    }

    // Step 5: Find where the tail starts (after the digit block)
    size_t tail_start = end + 1;
    size_t tail_len = len - tail_start;

    // Compose output: [part1] + " - " + [digits] + [tail]
    size_t part1_len = start;
    size_t digits_len = end - start + 1;

    // +1 extra space if we're inserting before '('
    size_t out_len = part1_len + insert_space_before_paren + 3 + digits_len + tail_len + 1;

    char *out = malloc(out_len);
    if (!out) return NULL;

    char *p = out;

    // Copy part1
    memcpy(p, title, part1_len);
    p += part1_len;

    if (insert_space_before_paren) {
        *p++ = ' ';
    }

    // Add " - "
    memcpy(p, " - ", 3);
    p += 3;

    // Copy digits
    memcpy(p, title + start, digits_len);
    p += digits_len;

    // Copy tail
    memcpy(p, title + tail_start, tail_len);
    p += tail_len;

    *p = '\0';

    return out;
}


// ------------------------------
// ------------------------------
// ------------------------------


/*
 * Extracts quoted phrases from a user’s search string, preserving
 * multi-word terms exactly as typed.
 *
 * JM: This is an experimental function, and prints debug info to the terminal
 *
 * Behavior:
 * - Normalize fancy / curly quote marks ==> ASCII).
 * - Extract complete quoted phrases; ignore unmatched quotes.
 * - Preserve punctuation/apostrophes inside phrases.
 * - Lowercase and trim each phrase.
 * - Ignore unquoted words.
 * - Return space-separated phrases (no quotes).
 * - Overall, it preserves meaningful search term grouping and punctuation
 *   within the string, and presents it as an unquoted, flat string to
 *   downstream search systems.
 *
 * Example:
 *   Input:  "Grilled Cheese" and ‘Oyster Po’ Boys’ or 'Tomato Soup'
 *   Output: grilled cheese oyster po' boys tomato soup
 *
 * Notes:
 * - Input must be UTF-8; only ASCII quotes are normalized.
 * - Output string must be freed with g_free().
 * - This design follows a similar philosophy to how Google handles quoted
 *   search terms: quoted phrases are treated as exact terms. Unquoted terms
 *   are typically tokenized into individual words by most search engines, but
 *   in this function, we ignore them entirely to focus on strong user intent.
 */

char *extract_quoted_terms(const char *search_term) {
    if (!search_term) return NULL;

    // Duplicate input into a mutable buffer so we can modify it
    char *input = g_strdup(search_term);
    normalize_quotes_utf8(input);

    GString *result = g_string_new(NULL);
    char *p = input;

    printf("[DEBUG]: Starting function extract_quoted_terms:\n%s\n", search_term);
    printf("[DEBUG]: Input string address: %p\n\n", (void *)search_term);

    while (*p) {
        if (*p == '"' || *p == '\'') {
            printf("[DEBUG] Found quote: %c at position: %td\n", *p, p - input);
            char quote = *p++;
            char *start = p;
            char *end = strchr(start, quote);
            if (!end) {
                printf("[DEBUG] No closing quote found. Remaining string: %s\n", start);
                break;
            }

            size_t len = end - start;
            if (len > 0) {
                char *term = g_strndup(start, len);
                char *trimmed = g_strstrip(term);
                char *processed = g_ascii_strdown(trimmed, -1);

                printf("[DEBUG] Found quoted term: %s\n", term);
                printf("[DEBUG] Processed term: %s\n", processed);

                if (result->len > 0)
                    g_string_append_c(result, ' ');
                g_string_append(result, processed);

                g_free(term);
                g_free(processed);
            } else {
                printf("[DEBUG] Empty quoted string found\n");
            }

            p = end + 1;
        } else {
            p++;
        }
    }

    g_free(input);

    if (result->len > 0) {
        printf("[DEBUG] Total recipes that matched quoted search term: %zu\n\n", result->len);
    } else {
        printf("[DEBUG] No quoted search term found.\n\n");
    }

    // Caller must g_free() this result.
    return g_string_free(result, FALSE);
}


// ------------------------------


static GList* extract_quoted_phrases(const char *search_term) {
    if (!search_term) return NULL;

    // Duplicate input to mutable buffer
    char *input = g_strdup(search_term);
    normalize_quotes_utf8(input);

    GList *phrases = NULL;
    char *p = input;

    while (*p) {
        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            char *start = p;
            char *end = strchr(start, quote);
            if (!end) {
                // No closing quote; stop processing further
                break;
            }

            size_t len = end - start;
            if (len > 0) {
                char *term = g_strndup(start, len);
                char *trimmed = g_strstrip(term);
                char *processed = g_ascii_strdown(trimmed, -1);

                // Append the full quoted phrase as one string
                phrases = g_list_append(phrases, processed);

                g_free(term);
                // Note: processed is stored in list, so do not free here
            }
            p = end + 1;
        } else {
            p++;
        }
    }

    g_free(input);
    return phrases; // caller must free list and strings
}


// ------------------------------



// Normalize single and double curly quote marks in-place to ASCII quotes.
// This can expanded later to handle more UTF-8 symbols if needed.

static void normalize_quotes_utf8(char *str) {
    if (!str) return;

    for (; *str; ++str) {
        unsigned char c = (unsigned char)*str;

        switch (c) {
            // ASCII-compatible replacement for common curly quotes:
            case 0xE2: {
                if ((unsigned char)str[1] == 0x80) {
                    switch ((unsigned char)str[2]) {
                        case 0x98: // ‘
                        case 0x99: // ’
                            *str = '\'';
                            str[1] = str[2] = ' ';
                            break;
                        case 0x9C: // “
                        case 0x9D: // ”
                            *str = '"';
                            str[1] = str[2] = ' ';
                            break;
                    }
                }
                break;
            }
        }
    }
}



// ------------------------------


static QuoteStatus detect_quote_status(const char *search_term) {
    int single_quotes = 0;
    int double_quotes = 0;

    for (const char *p = search_term; *p; p++) {
        if (*p == '\'') single_quotes++;
        else if (*p == '"') double_quotes++;
    }

    if ((single_quotes > 0 && single_quotes % 2 == 0) ||
        (double_quotes > 0 && double_quotes % 2 == 0)) {
        return QUOTE_PAIR;
    }

    if (single_quotes > 0 || double_quotes > 0) {
        return QUOTE_SINGLE;
    }

    return QUOTE_NONE;
}



// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------


// The following two functions work together as a text preprocessing utility
//   that extracts meaningful keywords from recipe titles, the user's input.
// These functions are handy to:
//    - Parse recipe titles or user search queries into keywords.
//    -  Improve search matching (ignoring filler words).
//    - Build tags or indexes from recipe names.
// For example:
//    User searches for: "The Best Soup in the World"
//    After filtering: [best soup world] becomes the key search words, which
//      match recipes more effectively by comparing only meaningful words.

// Stop-Words List
// Purpose:
// Checks whether a given word is a stop word (commonly ignored words).
// Implementation:
//   It loops through the stop_words[] list.
//   Uses g_ascii_strcasecmp() (case-insensitive compare).
//   Returns TRUE if the word matches a stop word, otherwise FALSE.

static const char *stop_words[] = {
    "a", "an", "the", "and", "or", "with", "of", "in", "on", "at", "to", "for", "by", NULL
};

// Check if word is stop word (case-insensitive)
static gboolean is_stop_word(const char *word) {
    if (!word) return FALSE;

    for (int i = 0; stop_words[i] != NULL; i++) {
        if (g_ascii_strcasecmp(word, stop_words[i]) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}


// ---------------------------------------------------------------------------


// Tokenizes, lowercases, filters stop words, returns GList of tokens
// Purpose:
// Takes a full phrase, splits it into words (tokens), lowercases them, removes
//    stop words, and returns the remaining tokens in a GList.
//  Implementation:
//    1. Converts the entire input phrase to lowercase (g_ascii_strdown).
//    2. Tokenizes by spaces (strtok_r).
//    3. Skips stop words using is_stop_word().
//    4.	Adds remaining words to a GList (each word duplicated with g_strdup()so they survive after freeing the input buffer).
//    5.Returns the GList * of filtered tokens.

GList *tokenize_and_filter_stop_words(const char *phrase) {
    if (!phrase) return NULL;

    // Duplicate and lowercase input for tokenization
    char *input = g_ascii_strdown(phrase, -1);
    GList *filtered_tokens = NULL;

    // Tokenize by space
    char *saveptr = NULL;
    char *token = strtok_r(input, " ", &saveptr);

    while (token) {
        if (!is_stop_word(token)) {
            // Add to filtered list (dup because input will be freed)
            filtered_tokens = g_list_append(filtered_tokens, g_strdup(token));
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    g_free(input);
    return filtered_tokens;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------



// Normalizes a recipe title to lowercase.
// It also checks if the search term (needle) appears anywhere inside the
// recipe title (haystack).

static bool contains_word_case_insensitive(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;

    char h[512], n[128];
    strncpy(h, haystack, sizeof(h) - 1);
    strncpy(n, needle, sizeof(n) - 1);
    h[sizeof(h) - 1] = '\0';
    n[sizeof(n) - 1] = '\0';

    // Convert both to lowercase
    for (char *p = h; *p; ++p) *p = tolower(*p);
    for (char *p = n; *p; ++p) *p = tolower(*p);

    return strstr(h, n) != NULL;
}


// ------------------------------



// Helper to recursively extract the visible text from a recipe anchor tag
static const char* extract_anchor_text(GumboNode *node) {
    if (node->type == GUMBO_NODE_TEXT)
        return node->v.text.text;

    if (node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_DOCUMENT) {
        GumboVector *children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            const char *text = extract_anchor_text(children->data[i]);
            if (text && *text) return text;
        }
    }
    return NULL;
}



// ------------------------------

// Helper: Converts a recipe slug (any format) into a nicely spaced, Title Case-ready string.
//
// A recipe slug is the short, URL-friendly identifier for a recipe, typically
// found at the end of a food webpage URL. It usually contains words from
// the recipe's title, joined together without spaces or with dashes/underscores.
// This function:
// - Trims whitespace
// - Replaces dashes/underscores with spaces
// - Inserts a space before capital letters in camelCase or PascalCase slugs
// - Leaves capitalization untouched; final Title Casing is handled in
//   the add_link() function
// Formats handled:
// - camelCase:     cheesyChickenCasserole
// - PascalCase:    CheesyChickenCasserole
// - snake_case:    cheesy_chicken_casserole
// - kebab-case:    cheesy-chicken-casserole
// Think of PascalCase as "CamelCase wearing a top hat."
// Think of camelCase as "PascalCase on casual Friday."

static void slug_to_title(const char *slug, char *out, size_t out_size) {

printf("[DEBUG]: slug_to_title function input slug:%s\n", slug);

    char trimmed_slug[256];
    trim_whitespace(slug, trimmed_slug, sizeof(trimmed_slug));

    bool had_space = false, had_underscore = false, had_dash = false;

    for (size_t i = 0; trimmed_slug[i] != '\0'; ++i) {
        if (trimmed_slug[i] == ' ') had_space = true;
        else if (trimmed_slug[i] == '_') had_underscore = true;
        else if (trimmed_slug[i] == '-') had_dash = true;
    }

    size_t out_index = 0;

    if (had_space || had_underscore || had_dash) {
        for (size_t i = 0; trimmed_slug[i] != '\0' && out_index < out_size - 1; ++i) {
            char c = trimmed_slug[i];
            if (c == '_' || c == '-') c = ' ';
            out[out_index++] = c;
        }
    } else {
        // Insert space before capital letters following lowercase letters
        for (size_t i = 0; trimmed_slug[i] != '\0' && out_index < out_size - 1; ++i) {
            char c = trimmed_slug[i];
            if (i > 0 && isupper((unsigned char)c) && islower((unsigned char)trimmed_slug[i - 1])) {
                if (out_index < out_size - 2)
                    out[out_index++] = ' ';
            }
            out[out_index++] = c;
        }
    }

    out[out_index] = '\0';

printf("[DEBUG]: slug_to_title function output title:%s\n", out);

}



// ----------------------------


// Callback for when a recipe button is clicked
// Safely retrieves the URL stored in the button's data
static void on_recipe_clicked(GtkWidget *btn, gpointer user_data G_GNUC_UNUSED) {
    // Get the URL stored in the button
    const char *url = g_object_get_data(G_OBJECT(btn), "url");

    if (!url) {
        g_warning("Recipe button clicked but URL is missing!");
        return;
    }

    // Open the URL in the default browser
    gtk_show_uri_on_window(NULL, url, GDK_CURRENT_TIME, NULL);

    // Note: Do NOT free 'url' here because it's owned by the button
    // It will be freed automatically when the button is destroyed
}



// ------------------------------


// Helper: This ensures the search box gets the cursor and keyboard focus
// after the app window is shown

static gboolean focus_entry_idle(gpointer user_data) {
    GtkWidget *entry = GTK_WIDGET(user_data);
    gtk_widget_grab_focus(entry);
    // Also explicitly set focus on the window itself
    GtkWindow *win = GTK_WINDOW(gtk_widget_get_toplevel(entry));
    gtk_window_set_focus(win, entry);
    return FALSE;
}

// ------------------------------



// Focus the entry field to automatically get keyboard focus when the window is shown

static void on_window_realize(GtkWidget *widget G_GNUC_UNUSED, gpointer user_data) {
    g_idle_add(focus_entry_idle, user_data);
}


// ==================


// This sets up a customized appearance (colors, fonts) via CSS at runtime.
// It loads a CSS string and applies the CSS styles globally to all of the GTK
// widgets in the app.

// CSS errors are captured and reported.
// This 

static void register_css_styles(const gchar *css_data) {
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError *error = NULL;

    // Try loading CSS data, capture any errors
    if (!gtk_css_provider_load_from_data(css_provider, css_data, -1, &error)) {
        // CSS failed to load, print error message
        fprintf(stderr, "Error loading CSS styles: %s\n", error->message);
        g_error_free(error);
        g_object_unref(css_provider);
        return;
    }

    // Apply CSS globally to default screen
    GdkScreen *screen = gdk_screen_get_default();
    if (screen) {
        gtk_style_context_add_provider_for_screen(
            screen,
            GTK_STYLE_PROVIDER(css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER);
    } else {
        fprintf(stderr, "Warning: No default screen available to apply CSS styles\n");
    }

    g_object_unref(css_provider);
}



// ==================
// ======================
// ==================


/*
 * show_results populates the GTK listbox with clickable buttons representing
 * filtered recipe results from the matched recipe website links.
 *
 * Displays recipe search results in the GTK listbox widget, applying advanced
 * filtering based on whether the user provided quoted search terms.
 * Filters and displays recipes based on the user's search term, including
 * support for quoted phrases for exact matches and broader "partial" matches.
 * Logic Overview:
 * 1. Clears any existing search results from the listbox.
 * 2. If search_term has quoted phrases:
 *    - Extracts quoted phrases and lowercases them.
 *    - Builds a partial search term by combining phrases and removing stop words.
 *    - Filters recipes:
 *        a) Looser (partial) term tokens appear in the recipe title.
 *        b) Each quoted phrase's keywords are in the recipe title.
 *    - Only recipes passing both checks are included.
 * 3. If no quoted phrases:
 *    - Filters recipes containing all tokens of the search term.
 * 4. For matched recipes:
 *    - Creates RecipeInfo structs and queues them for animated insertion.
 * 5. Animated insertion:
 *    - Inserts one button at a time every 100 ms.
 *    - Yellow buttons for perfect matches.
 *    - Beige buttons for partial matches.
 * 6. Memory safety:
 *    - Frees all temporary RecipeInfo structs and g_strdup strings.
 *
 * Purpose:
 * Improves recall and relevance for quoted queries by broadening searches
 * locally while still highlighting exact matches.
 *
 * Parameters:
 *   listbox: The GTK listbox widget to populate with results.
 *   links: GList of strings containing "title\x1fURL".
 *   search_term: Original user search term (may include quotes).
 *   quote_status: Enum indicating presence of quoted phrases.
 *
 * FUTURE IMPROVEMENTS:
 *   - Add subtle fade effect during removal for smoother visual feedback.
 */

/*
JM NOTES, 8/26/2025:
AllRecipes.com behavior
The discrepancy between quoted and unquoted searches is not a bug in your code; it’s the website’s search engine behavior.
On AllRecipes:
Unquoted search: the site will return any recipe that contains any of the terms you typed, often with broader matching and ranking.
Quoted search: the site will try to match the exact phrase, which is much stricter. If a recipe title doesn’t have roasted chicken exactly as a phrase, it may be excluded.
This explains why your code sees many results for roasted chicken unquoted but only one for "roasted chicken" quoted.
Effect on this logic:
Your code is faithfully applying the perfect vs. partial-match rules to whatever data the website returns.
If the site skews results based on quoted terms, your perfect-match count will naturally be lower.
Partial-match logic mitigates this a little, because it only requires one token match, which is why your yellow links catch the other recipes.
Reality check: The site behavior is influencing your “perfect match” count, so the skew is external, not a bug in your app.
E.g., the AllRecipes.com behavior when doing a recipe search using a browser directly on thier website skews the user experience for quoted searches. 
Users might be surprised that the quoted search term "roasted chicken" returns zero results while the unquoted search term roasted chicken returns plenty.
The behavior is site-dependent. AllRecipes is strict with quotes (0 results), while Food Network allows partial matches and returns more.
This logic's partial-match feature is compensating nicely for sites that return very few results for quoted searches. Users still see relevant recipes highlighted
 in yellow even when the site is restrictive.
The only “weirdness” comes from how individual food sites handle quoted search terms.
This code correctly tries to preserve user intent of quoted search terms by giving visual cues of yellow and beige highlighted results.
 */

static void show_results(GtkWidget *listbox_widget, GList *links, const char *search_term, QuoteStatus quote_status) {

    printf("\n[INFO]: Entering show_results() function\n");
    printf("[INFO]: Input search_term:\n%s\n", search_term);

    GtkListBox *listbox = GTK_LIST_BOX(listbox_widget);

    // Step 1: Refresh UI
    gtk_widget_queue_draw(GTK_WIDGET(listbox));
    while (gtk_events_pending())
        gtk_main_iteration();

    // Step 2: Clear existing listbox children
    GList *children = gtk_container_get_children(GTK_CONTAINER(listbox));
    for (GList *iter = children; iter; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    // Step 3: Handle quoted search logic
    GList *quoted_phrases = NULL;
    char *partial_search_term = NULL;

    if (quote_status == QUOTE_PAIR) {
        quoted_phrases = extract_quoted_phrases(search_term);

        if (quoted_phrases) {
            printf("[INFO]: Extracted quoted phrases:\n");
            for (GList *iter = quoted_phrases; iter; iter = iter->next)
                printf("%s \n", (char *)iter->data);

            // Build combined string of phrases
            GString *combined = g_string_new("");
            for (GList *iter = quoted_phrases; iter; iter = iter->next) {
                if (combined->len > 0) g_string_append_c(combined, ' ');
                g_string_append(combined, (char *)iter->data);
            }

            // Tokenize and filter stop words
            GList *partial_tokens = tokenize_and_filter_stop_words(combined->str);
            g_string_free(combined, TRUE);

            if (partial_tokens) {
                GString *builder = g_string_new("");
                for (GList *iter = partial_tokens; iter; iter = iter->next) {
                    if (builder->len > 0) g_string_append_c(builder, ' ');
                    g_string_append(builder, (char *)iter->data);
                }
                partial_search_term = g_string_free(builder, FALSE);
                g_list_free_full(partial_tokens, g_free);
            }
        }
    }

    // Display which search term is actually used
    if (quote_status == QUOTE_PAIR && partial_search_term)
        printf("[INFO]: Using decisive search term (partial_search_term):\n%s\n\n", partial_search_term);
    else
        printf("[INFO]: Using decisive search term (raw search_term):\n%s\n\n", search_term);

    // Step 4: Prepare queue of matching recipes
    GQueue *recipe_queue = g_queue_new();

    for (GList *l = links; l; l = l->next) {
        char *entry = l->data;
        char *sep = strchr(entry, '\x1f');
        if (!sep) continue;

        *sep = '\0';
        char *title = entry;
        char *url = sep + 1;

        char *lower_title = g_ascii_strdown(title, -1);
        gboolean perfect_match = FALSE;
        gboolean partial_match = FALSE;
        int total_tokens = 0;
        int matched_tokens = 0;

        // Match logic for quoted search
        if (quote_status == QUOTE_PAIR && quoted_phrases && partial_search_term) {
            GList *tokens = tokenize_and_filter_stop_words(partial_search_term);
            if (tokens) {
                total_tokens = g_list_length(tokens);
                for (GList *ti = tokens; ti; ti = ti->next) {
                    if (strstr(lower_title, (char *)ti->data))
                        matched_tokens++;
                }
                g_list_free_full(tokens, g_free);

                if (matched_tokens == total_tokens)
                    perfect_match = TRUE;
                else if (matched_tokens > 0)
                    partial_match = TRUE;
            }
        }

        g_free(lower_title);

        // Skip non-matching recipe if quote filter is active
        if (quote_status == QUOTE_PAIR && !perfect_match && !partial_match)
            continue;

        // Add matching recipe to queue
        RecipeInfo *ri = g_new0(RecipeInfo, 1);
        ri->title = g_strdup(title);
        ri->url = g_strdup(url);
        ri->perfect_match = perfect_match;
        ri->partial_match = partial_match;
        ri->matched_tokens = matched_tokens;
        ri->total_tokens = total_tokens;

        g_queue_push_tail(recipe_queue, ri);
    }

    // Cleanup
    if (quoted_phrases) g_list_free_full(quoted_phrases, g_free);
    if (partial_search_term) g_free(partial_search_term);

    // Step 5: Animate recipe insertion
    InsertAnimationData *anim_data = g_new0(InsertAnimationData, 1);
    anim_data->listbox = listbox;
    anim_data->recipe_queue = recipe_queue;
    anim_data->partial_buttons = g_queue_new();

    g_timeout_add(100, insert_next_button, anim_data);

}


// ==================


// Helper: Insert one recipe link button at a time into the GTK listbox UI
// Applies CSS styling according to the search type:
//   - Default links: standard white/black button
//   - Partial matches: beige background button
//   - Perfect matches: yellow background button with blue glowing border
// Delayed appearance ensures buttons are fully realized in the UI before
//   any further styling is applied.
// The "visible" class triggers a minor delayed appearance after the
//    buttons are realized.

static gboolean insert_next_button(gpointer user_data) {
    InsertAnimationData *data = (InsertAnimationData *)user_data;

    // Stop if there are no more recipes
    if (g_queue_is_empty(data->recipe_queue)) {
        g_queue_free(data->partial_buttons);
        g_free(data);
        return FALSE;
    }

    // Pop the next recipe link from the queue into the UI
    RecipeInfo *ri = g_queue_pop_head(data->recipe_queue);

    // Create a button with the recipe title
    GtkWidget *btn = gtk_button_new_with_label(ri->title);

    // Store URL safely in the button object; freed when button is destroyed
    g_object_set_data_full(G_OBJECT(btn), "url", g_strdup(ri->url), g_free);

    // Connect click signal to open recipe
    g_signal_connect(btn, "clicked", G_CALLBACK(on_recipe_clicked), NULL);

    // Apply CSS style class based on match type
    GtkStyleContext *ctx = gtk_widget_get_style_context(btn);
    if (ri->perfect_match) {
        gtk_style_context_add_class(ctx, "recipe-perfect");
        // Schedule "visible" class to ensure button is realized before appearance
        g_idle_add((GSourceFunc)add_visible_class, btn);
        printf("[INFO] INSERTING PERFECT MATCH (YELLOW): %s (%d/%d tokens)\n",
               ri->title, ri->matched_tokens, ri->total_tokens);
    } else if (ri->partial_match) {
        gtk_style_context_add_class(ctx, "recipe-partial");
        printf("[INFO] INSERTING PARTIAL MATCH (BEIGE): %s (%d/%d tokens)\n",
               ri->title, ri->matched_tokens, ri->total_tokens);
    } else {
        gtk_style_context_add_class(ctx, "recipe-button");
        printf("[INFO] INSERTING RECIPE LINK: %s\n", ri->title);
    }

    // Insert button into listbox and show it
    gtk_list_box_insert(data->listbox, btn, -1);
    gtk_widget_show_all(GTK_WIDGET(data->listbox));

    // Free temporary recipe info
    g_free(ri->title);
    g_free(ri->url);
    g_free(ri);

    return TRUE;
}


// ==================


// Helper: Add the "visible" class to a widget once.
// Runs on the next GTK idle cycle to ensure the widget is realized.

static gboolean add_visible_class(gpointer widget) {
    if (!widget) {
        printf("[WARNING]: the add_visible_class function was called with a NULL widget pointer\n");
        return FALSE; // stop callback if widget is invalid
    }

    GtkStyleContext *ctx = gtk_widget_get_style_context(GTK_WIDGET(widget));
    gtk_style_context_add_class(ctx, "visible");

    printf("[*** INFO]: Added 'visible' class to widget %p\n", widget);

    return FALSE; // run once
}


// ==================
// ==================
// ==================



// Thread entry point -- search logic only (no UI)
// This function runs in a separate thread to:
//  - Read a recipe search query from a UI input.
//  - Construct a URL for the selected recipe site.
//  - Download and parse the HTML results.
//  - Extract and store recipe data.
//  - Schedule a callback (search_complete_cb) to update the UI with
//     the results.
//  It is designed to be non-blocking, thread-safe, and modular:
//    each site can have its own parser logic via parse_site.

static gpointer search_thread_func(gpointer data) {

    // Reset recipe limit counter
    recipe_result_total = 0; // reset before starting a new search

    AppWidgets *w = data;
    SearchResultData *result = g_new0(SearchResultData, 1);
    result->w = w;
    result->success = FALSE;

    // Prep
    const char *q = gtk_entry_get_text(GTK_ENTRY(w->entry));

//    printf("\n[DEBUG]: Function search_thread_func received this input:\n%s\n\n", q);

    if (!q || !*q) {
        result->status_message = g_strdup("      Please enter a recipe search term (like roast chicken, or chili)");
        g_idle_add(search_complete_cb, result);
        return NULL;
    }

    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(w->combo));
    if (index < 0 || index >= (int)(sizeof(g_recipe_site_table) / sizeof(g_recipe_site_table[0]))) {
        result->status_message = g_strdup("Please select a valid recipe site.");
        g_idle_add(search_complete_cb, result);
        return NULL;
    }

    const RecipeSiteInfo *site = &g_recipe_site_table[index];
    g_free(g_current_website_name);
    g_current_website_name = g_strdup(site->name);

    char *enc = g_uri_escape_string(q, NULL, FALSE);
    if (!enc) {
        result->status_message = g_strdup("Failed to encode search term.");
        g_idle_add(search_complete_cb, result);
        return NULL;
    }

    result->url = g_strdup_printf(site->url_pattern, enc);
    g_free(enc);

    if (!result->url) {
        result->status_message = g_strdup("Failed to build URL.");
        g_idle_add(search_complete_cb, result);
        return NULL;
    }

    result->html = download_html(result->url);
    if (!result->html) {
        result->status_message = g_strdup("Failed to fetch recipes.");
        g_idle_add(search_complete_cb, result);
        return NULL;
    }

    result->output = gumbo_parse(result->html);
    if (!result->output) {
        result->status_message = g_strdup("Failed to parse HTML from site.");
        g_idle_add(search_complete_cb, result);
        return NULL;
    }

    GHashTable *link_set = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    if (site->parse_site) {
        site->parse_site(result->output->root, &result->results, link_set, q);
        result->success = TRUE;
    }

    g_hash_table_destroy(link_set);
    g_idle_add(search_complete_cb, result);
    return NULL;

}


// ==================


// Finalizes the UI after the background recipe search completes.
// Stops the pulsing progress bar animation, restores UI interactivity,
// and displays either the search results or an appropriate fallback message.
// Runs in the GTK main thread via g_idle_add().

static gboolean search_complete_cb(gpointer data) {

    SearchResultData *result = data;

// JM: Display function info in the terminal. Uses a  ternary operator
//         as a conditional expression rather than a full if statement.
//         This avoids calling g_list_length on a NULL pointer.
printf(
    "\n[INFO]: Function search_complete_cb() was called.\n"
    "        callback success flag = %d\n"
    "        memory address of GList containing recipe links: %p\n"
    "        number of recipe links in GList: %d\n\n",
    result->success,
    result->results,
    result->results ? g_list_length(result->results) : 0
);
fflush(stdout);


    AppWidgets *w = result->w;

    // STOP the progress bar timer first
    if (w->pulse_timer_id != 0) {
        g_source_remove(w->pulse_timer_id);
        w->pulse_timer_id = 0;
    }

    // Hide progress bar and re-enable UI
    // Restores the busy/wait cursor to the normal arrow
    gtk_widget_hide(w->progress_bar);
    set_cursor(gtk_widget_get_toplevel(w->search_button), GDK_LEFT_PTR);
    set_ui_enabled(w, TRUE);

    // Clear previous results before showing new ones
    clear_recipe_results(w->listbox);

    // Show results or fallback
    if (result->success && result->results) {
        const char *q = gtk_entry_get_text(GTK_ENTRY(w->entry));
        show_results(w->listbox, result->results, q, w->quote_status);
        gtk_label_set_text(GTK_LABEL(w->status_label), "");
    } else if (result->url && !result->results) {
        insert_fallback_link(w->listbox, result->url, "Matching recipes not found. Click to open the main food website.");
        gtk_label_set_text(GTK_LABEL(w->status_label), "");
    } else {
        gtk_label_set_text(GTK_LABEL(w->status_label),
                           result->status_message ? result->status_message : "Search failed.");
    }

    // Clean up
    gumbo_destroy_output(&kGumboDefaultOptions, result->output);
    g_list_free_full(result->results, g_free);
    g_free(result->html);
    g_free(result->url);
    g_free(result->status_message);
    g_free(result);

    return G_SOURCE_REMOVE;
}



// ==================



// Callback for GTK timer
// Pulses the progress bar widget inside AppWidgets every 100ms.  The
// 100 ms interval comes from the timer set up with g_timeout_add(100, ...)
// Keeps the progress bar animation running while a background task is
// in progress.
// Returns G_SOURCE_CONTINUE to keep the timer active.
static gboolean pulse_progress_bar(gpointer data) {
    AppWidgets *w = data;
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(w->progress_bar));
    return G_SOURCE_CONTINUE; // Keep the timeout running
}




// ==================
// ==================
// ==================



// Search for matching recipes!
// Initial immediate search button handler -- sets up UI and defers real work

static void initialize_on_search(GtkButton *btn G_GNUC_UNUSED, gpointer ud) {
    AppWidgets *w = ud;

    // Ensure there is a search term to use
    const char *q = gtk_entry_get_text(GTK_ENTRY(w->entry));
    if (!q || !*q) {
    // Don't proceed with empty input
        gtk_label_set_text(GTK_LABEL(w->status_label),
            "      Please enter a recipe search term (like:  roast chicken, or chili, or Peking Duck)");
        printf("\n\n=========================================================\n");
        printf("\nUSER SEARCH TERM WAS EMPTY!\n");
        return;
    }

    printf("\n\n=========================================================\n");
    printf("    <<<   N E W     R E C I P E     S E A R C H   >>>\n");
    printf("=========================================================\n");
    printf("\nUSER SEARCH TERM:\n%s\n", q);
    printf("\n------------------------------------------------\n");

    // Detect quote status and store it in the widgets struct for later use
    QuoteStatus quote_status = detect_quote_status(q);
    w->quote_status = quote_status;

    // Prepare user-facing status message based on quote usage
    const char *status_msg = NULL;
    switch (quote_status) {
        case QUOTE_PAIR:
            status_msg = "    Quoted searches behave differently on each website!  Searching, please wait …";
            break;
        case QUOTE_SINGLE:
            status_msg = "   Searching for recipes (Note: Please check your unmatched quote marks) ...";
            break;
        case QUOTE_NONE:
        default:
            status_msg = "   Searching for matching recipes. Please wait ...";
            break;
    }

    // Set busy cursor
    GtkWidget *toplevel = gtk_widget_get_toplevel(w->search_button);
    set_cursor(toplevel, GDK_WATCH);

    // Disable all UI during search
    set_ui_enabled(w, FALSE);

    // Clear any previous results
    clear_recipe_results(w->listbox);

    // Show status and progress bar
    gtk_label_set_text(GTK_LABEL(w->status_label), status_msg);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(w->progress_bar), 0.0);
    gtk_widget_show(w->progress_bar);

    // Process GTK events before launching the thread
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);

    // Start pulsing progress bar
    w->pulse_timer_id = g_timeout_add(100, (GSourceFunc)pulse_progress_bar, w);

    // launch search thread
    g_thread_new("recipe_search_thread", search_thread_func, w);

}


// ==================


// Helper: Set busy or normal cursor on the toplevel window
static void set_cursor(GtkWidget *widget, GdkCursorType cursor_type) {
    GdkDisplay *display = gdk_display_get_default();
    GdkCursor *cursor = (cursor_type != GDK_BLANK_CURSOR)
        ? gdk_cursor_new_for_display(display, cursor_type)
        : NULL;
    GdkWindow *window = gtk_widget_get_window(widget);
    if (window)
        gdk_window_set_cursor(window, cursor);
    if (cursor)
        g_object_unref(cursor);
}



// ==================


// Helper function to clear the previous recipe search results
static void clear_recipe_results(GtkWidget *listbox) {
    gtk_widget_freeze_child_notify(listbox);
    GList *children = gtk_container_get_children(GTK_CONTAINER(listbox));
    for (GList *iter = children; iter != NULL; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);
    gtk_widget_thaw_child_notify(listbox);
    gtk_widget_queue_draw(listbox);  // Force redraw in case anything is left over
}


// ==================


// Scroll-blocking filter
// Note: G_GNUC_UNUSED is added to prevent compiler warnings.
// GTK signal handlers must match a specific function signature, even if
// you don't use the parameters. As an example, for this scroll-event,
// GTK expects a function like:
//   gboolean handler(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
// Even if the parameters widget, event, and user_data are unused, you still
// need to include them in the function signature, or GTK will not connect
// the signal properly.

static gboolean block_scroll(GtkWidget *widget G_GNUC_UNUSED,
                             GdkEventScroll *event G_GNUC_UNUSED,
                             gpointer user_data G_GNUC_UNUSED) {
    return search_in_progress;  // Block scroll events while search is running
}


// ==================


// Unified UI enable/disable function:

static void set_ui_enabled(const AppWidgets *w, gboolean enabled) {
    search_in_progress = !enabled;

    gtk_widget_set_sensitive(w->search_button, enabled);
    gtk_widget_set_sensitive(w->entry, enabled);
    gtk_widget_set_sensitive(w->combo, enabled);
    gtk_widget_set_sensitive(w->listbox, enabled);

    gtk_editable_set_editable(GTK_EDITABLE(w->entry), enabled);

    gtk_widget_set_can_focus(w->search_button, enabled);
    gtk_widget_set_can_focus(w->entry, enabled);
    gtk_widget_set_can_focus(w->combo, enabled);
    gtk_widget_set_can_focus(w->listbox, enabled);
}


// ==================


// Fallback recipe link handler
// Adds a manual recipe link button to the combo box if the selected site
// has no direct matching recipes.
// The fallback link tries to provide a webpage that in itself has various
// matching recipes on the food site. Worst case the fallback link will
// be the main page of the food website.
// Uses the same main result renderer to consistently format the link button.

void insert_fallback_link(GtkWidget *listbox, const char *url, const char *description) {
    // Create a fallback button styled like the others
    GtkWidget *btn = gtk_button_new_with_label(description);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "recipe-button");

    // Store the URL and connect the click handler
    g_object_set_data_full(G_OBJECT(btn), "url", g_strdup(url), g_free);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_recipe_clicked),
                     g_object_get_data(G_OBJECT(btn), "url"));

    gtk_list_box_insert(GTK_LIST_BOX(listbox), btn, -1);
    gtk_widget_show_all(btn);
}




// ================================================================
//  ***  CSS STYLES  ***
// ================================================================

// Load and register the app's custom GTK CSS style rules to enhance the UI.
// CSS stands for Cascading Style Sheets.
// CSS styles affect the appearance of GTK elements (combo boxes, buttons,
// entry fields, and labels), without changing their actual functionality.
// This includes colors, fonts, spacing, borders, layouts, and animations.

static void load_app_css_styles(void) {
    const gchar *css =
        // =====================
        // Search Entry Field Styles
        //   (Yellow Background)
        // =====================
        "entry.search-entry {"
        "  background-color: #fff7b3;"
        "  font-weight: bold;"
        "  font-size: 18pt;"
        "  border: 1px solid #aaa;"
        "  border-radius: 6px;"
        "  padding: 8px;"
        "}\n"

        // =======================
        // Combo Box Parent Styles
        //   (Blue Background)
        // =======================
        ".site-combo {"
        "  background-color: #b3d7ff;"
        "  color: #222222;"
        "  font-weight: bold;"
        "  font-size: 13pt;"
        "  min-height: 12px;"
        "  padding-top: 0px;"
        "  padding-bottom: 0px;"
        "  margin-top: 0px;"
        "  margin-bottom: 0px;"
        "}\n"

        // ================================================
        // Combo Box Children Styles (elements inside box)
        // ================================================
        ".site-combo > * {"
        "  background-color: #b3d7ff;"
        "  color: #222222;"
        "  font-weight: bold;"
        "  font-size: 13pt;"
        "  min-height: 12px;"
        "  padding-top: 0px;"
        "  padding-bottom: 0px;"
        "  margin-top: 0px;"
        "  margin-bottom: 0px;"
        "}\n"

        // ===================================
        // Status Label (status and error messages)
        // ===================================
        ".status-label {"
        "  color: red;"
        "  font-weight: bold;"
        "  font-size: 12pt;"
        "}\n"

        // ===================
        // Search Button Styles
        // ===================
        ".search-button {"
        "  background-image: none;"
        "  background-color: #a2f6a9;"
        "  color: black;"
        "  font-weight: bold;"
        "  font-size: 18px;"
        "  border: 2px solid #388E3C;"
        "  border-radius: 5px;"
        "  padding: 12px 16px;"
        "}\n"
        ".search-button:hover {"
        "  background-color: #b2f2bb;"
        "}\n"

        // ========================
        // Combo Box List Row Styles
        // ========================
        "combobox box list row,"
        "combobox menuitem {"
        "  padding: 4px 12px;"
        "  min-height: 28px;"
        "  font-size: 13pt;"
        "}\n"

        // ======================================
        // Standard Recipe Link Button Styles
        // ======================================
        "button.recipe-button {"
        "  background-image: none;"
        "  background-color: #FFFBE7;"
        "  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);"
        "  color: #000000;"
        "  font-weight: bold;"
        "  font-size: 12pt;"
        "  border: 1px solid #E0C080;"
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "}\n"
        "button.recipe-button:hover {"
        "  border-color: #888888;"            /* dark gray hover */
        "  background-color: #FFFBE7;"        /* subtle lighten */
        "}\n"

        // ======================================
        // Partial Match Recipe Styles (Light Beige + Hover)
        // ======================================
        "button.recipe-partial {"
        "  background-image: none;"
        "  background-color: #FFF8E1;"
        "  color: #000000;"
        "  font-weight: bold;"
        "  font-size: 12pt;"
        "  border: 1px solid #E0C080;"
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "}\n"
        "button.recipe-partial:hover {"
        "  border-color: #AFA89A;"            /* warm gray hover */
        "  background-color: #FFFBE7;"        /* subtle lighten */
        "}\n"

        // =================================================
        // Perfect Match Recipe Styles (Bright Yellow + Blue Hover)
        // =================================================
        "button.recipe-perfect {"
        "  background-image: none;"
        "  background-color: #fff7b3;"
        "  color: #000000;"
        "  font-weight: bold;"
        "  font-size: 12pt;"
        "  border: 2px solid #FFD700;"
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "}\n"
        "button.recipe-perfect:hover {"
        "  border-color: #4A90E2;"            /* blue glow effect on hover */
        "  background-color: #fffb90;"        /* subtle lighten */
        "}\n";

    register_css_styles(css);
}



// ==========================================================================
//  ***  BEGINNING OF JAVASCRIPT AND C RECIPE PARSERS  ***
// ==========================================================================

/*
NOTES ON THE JAVASCRIPT CODE BELOW:

This app uses embedded JavaScript parsers, stored as long C string
variables that are customized for specific recipe websites.
These JavaScript snippets form the contents of temporary external
'.js' files generated by the app. The temporary files are dynamically
written to disk and executed by website-specific C logic using Node.js
and the Playwright library.

Purpose:
  - Automates a headless browser (Chromium) to search for recipes on
    supported websites. It behaves like a normal browser: loading pages,
    running JavaScript, handling cookies, but operates in the background
    without a GUI, making it ideal for automation, scraping or testing.

  - Automatically scrolls the search results page using JavaScript to
    trigger lazy loading of additional content. Lazy loading is a design
    pattern where websites defer loading parts of the page until needed,
    often when a user scrolls. Since full content isn't always present
    in the initial HTML, this app mimics scrolling behavior in a headless
    browser to ensure all content is loaded before extracting recipe data.

  - Extracts visible recipe titles and URLs from the fully rendered page.
  - Outputs the results as a JSON array to stdout, where they are parsed
    by the website- specific C logic.


Example Workflow:
   1. User runs the app, which launches Chromium in headless mode.
   2. Enters a search term (e.g., "chili").
   3. Selects a recipe site (e.g., Food Network).
   4. The app loads search results, scrolls to load all recipes,
      and extracts clean titles and links (ignoring ads/videos).
   5. The JavaScript outputs a JSON text array to stdout, such as:
         {
           "title": "Cincinnati Chili",
           "url": "https://www.foodnetwork.com/recipes/food-network-kitchen/cincinnati-chili-recipe2-1972747"
         }
    6. The associated C code captures the script's stdout and parses the
       JSON text using the json-c library to extract recipe data.
    7. Various recipe titles are displayed in the app's UI list.
    8. The user clicks a result to open the recipe in the system's
       default web browser.
*/

// ==========================================================================
// ==========================================================================


// AllRecipes JavaScript (with a generic fallback link when no recipes are found)
static const char *allrecipes_js_code =
"const { chromium } = require('playwright');\n"
"\n"
"(async () => {\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const page = await browser.newPage();\n"
"\n"
"  const searchTerm = process.argv[2] || 'chicken';\n"
"  const searchUrl = `https://www.allrecipes.com/search?q=${encodeURIComponent(searchTerm)}`;\n"
"\n"
"  await page.goto(searchUrl, { waitUntil: 'domcontentloaded', timeout: 60000 });\n"
"\n"
"  for (let i = 0; i < 5; i++) {\n"
"    await page.evaluate(() => window.scrollBy(0, window.innerHeight));\n"
"    await page.waitForTimeout(1000);\n"
"  }\n"
"\n"
"  let recipes = [];\n"
"  try {\n"
"    await page.waitForSelector('a[href*=\"/recipe/\"]:not([href*=\"/video/\"])', { timeout: 10000 });\n"
"\n"
"    recipes = await page.evaluate(() => {\n"
"      const results = [];\n"
"      const seen = new Set();\n"
"\n"
"      document.querySelectorAll('a[href*=\"/recipe/\"]:not([href*=\"/video/\"])').forEach(a => {\n"
"        const href = a.href;\n"
"        const title = a.innerText.trim();\n"
"\n"
"        if (href && title && !seen.has(href) && !href.includes('ads')) {\n"
"          seen.add(href);\n"
"          results.push({ title, url: href });\n"
"        }\n"
"      });\n"
"\n"
"      return results;\n"
"    });\n"
"  } catch (err) {}\n"
"\n"
"  if (!recipes || recipes.length === 0) {\n"
"    recipes = [{\n"
"      title: 'Click to see AllRecipes Search Page',\n"
"      url: 'https://www.allrecipes.com/recipes/'\n"
"    }];\n"
"  }\n"
"\n"
"  console.log(JSON.stringify(recipes, null, 2));\n"
"  await browser.close();\n"
"})();\n";


// --------------------------------

// AllRecipes.com C recipe scraper scaffolding function (for embedded execution via Node.js)
static void parse_allrecipes(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char temp_filename[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "Failed to get Windows temp path.\n");
        return;
    }

    if (!GetTempFileNameA(temp_path, "ar", 0, temp_filename)) {
        fprintf(stderr, "Failed to create temporary JS filename.\n");
        return;
    }

    FILE *tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        fprintf(stderr, "Failed to open temp JS file for writing.\n");
        return;
    }
#else
    strcpy(temp_filename, "/tmp/allrecipes_XXXXXX.js");
    int fd = mkstemps(temp_filename, 3);  // ".js"
    if (fd == -1) {
        perror("mkstemps");
        return;
    }

    FILE *tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("fdopen");
        close(fd);
        return;
    }
#endif

    // Write embedded JavaScript to temp file
    fputs(allrecipes_js_code, tmp_fp);
    fclose(tmp_fp);

    char command[1024];

#ifdef _WIN32
    // Use %APPDATA%\npm\node_modules for NODE_PATH
    const char *node_path = getenv("APPDATA");
    if (!node_path) {
        fprintf(stderr, "APPDATA environment variable not set.\n");
        DeleteFileA(temp_filename);
        return;
    }
    char node_path_full[MAX_PATH];
    snprintf(node_path_full, sizeof(node_path_full), "%s\\npm\\node_modules", node_path);
    snprintf(command, sizeof(command),
        "cmd /c \"set NODE_PATH=%s&& \"C:\\\\Program Files\\\\nodejs\\\\node.exe\" \"%s\" \"%s\"\"",
        node_path_full, temp_filename, search_term);
#else
    const char *node_path = "/opt/homebrew/lib/node_modules";  // Change as needed for Mac and Linux
    snprintf(command, sizeof(command),
        "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
        node_path, temp_filename, search_term);
#endif

    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr, "Failed to run Node.js scraper command.\n");
#ifdef _WIN32
        DeleteFileA(temp_filename);
#else
        unlink(temp_filename);
#endif
        return;
    }

    char buffer[8192];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    pclose(fp);

#ifdef _WIN32
    DeleteFileA(temp_filename);
#else
    unlink(temp_filename);
#endif

    struct json_object *parsed_json = json_tokener_parse(buffer);

    // check for installed prerequisite software
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr,
        "\n[Recipe Finder Error]\n"
        "The recipe search script failed to run or returned no valid results.\n\n"
        "To fix this, make sure the following software is installed on your system:\n"
        "   - Node.js (https://nodejs.org)\n"
        "   - Playwright (install with: npm install -g playwright)\n"
        "   - Required Playwright browsers (run: playwright install)\n\n"
        "Also ensure that:\n"
        "   - Node.js is in your system PATH (run 'node -v' in Command Prompt to check)\n"
        "   - Your internet connection is active\n"
        "   - No firewall or antivirus is blocking Node.js or Playwright\n\n"
        "Defaulting to the AllRecipes generic search page ...\n"
    );

    add_link(out, "Click to see AllRecipes Search Page", "", "https://www.allrecipes.com/recipes/", link_set);
    return;
}

    size_t n = json_object_array_length(parsed_json);
    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj, *url_obj;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
           const char *url = json_object_get_string(url_obj);
           char *fixed_title = split_title_and_digits(title);
           if (!fixed_title) fixed_title = strdup(title); // fallback
           add_link(out, fixed_title, "", url, link_set);
           free(fixed_title);
        }
    }

    json_object_put(parsed_json);

    if (*out == NULL) {
        add_link(out, "Click to see AllRecipes Search Page", "", "https://www.allrecipes.com/recipes/", link_set);
    }
}


// ===============
// ===============



// BBC Good Food JavaScript:
// Both Tier 1 and Tier 2 are always executed
// Results from both tiers are combined
// Deduplication happens after combining the recipes
// Has maximum debug logging to the terminal

static const char *bbcgoodfood_js_code =
"const searchTerm = process.argv[2] || '';\n"
"const playwright = require('playwright');\n"
"\n"
"(async () => {\n"
"  function debugLog(msg) {\n"
"    console.error('[JS INFO] ' + msg);\n"
"  }\n"
"\n"
"  debugLog(`Searching \"${searchTerm}\"`);\n"
"\n"
"  const browser = await playwright.chromium.launch({ headless: true });\n"
"  const context = await browser.newContext();\n"
"  const page = await context.newPage();\n"
"\n"
"  page.on('dialog', async dialog => {\n"
"    debugLog('Dialog appeared, dismissing');\n"
"    await dialog.dismiss();\n"
"  });\n"
"\n"
"  try {\n"
"    const searchUrl = `https://www.bbcgoodfood.com/search/recipes?q=${encodeURIComponent(searchTerm)}`;\n"
"    debugLog('Navigating to BBC Good Food search page');\n"
"    await page.goto(searchUrl, { waitUntil: 'networkidle' });\n"
"\n"
"    const consentSelector = '[data-testid=\"consent-banner-accept\"]';\n"
"    if (await page.$(consentSelector)) {\n"
"      debugLog('No consent banner visible');\n"
"    }\n"
"\n"
"    for (let i = 1; i <= 7; i++) {\n"
"      debugLog(`Scrolled ${i}`);\n"
"      await page.evaluate(() => window.scrollBy(0, window.innerHeight));\n"
"      await page.waitForTimeout(1000);\n"
"    }\n"
"\n"
"    const screenshotPath = 'C:\\\\MSYS64\\\\PROJECTS\\\\after_scroll.png';\n"
"    const htmlPath = 'C:\\\\MSYS64\\\\PROJECTS\\\\page_content.html';\n"
"    await page.screenshot({ path: screenshotPath, fullPage: true });\n"
"    debugLog(`Saved screenshot to ${screenshotPath}`);\n"
"\n"
"    const htmlContent = await page.content();\n"
"    const fs = require('fs');\n"
"    fs.writeFileSync(htmlPath, htmlContent, 'utf-8');\n"
"    debugLog(`Wrote full HTML content to ${htmlPath}`);\n"
"\n"
"    debugLog('Attempting to extract recipes using updated logic');\n"
"\n"
"    const recipes = [];\n"
"    const articles = await page.$$('article');\n"
"    debugLog(`Found ${articles.length} <article> elements`);\n"
"\n"
"    for (const [index, article] of articles.entries()) {\n"
"      const linkHandle = await article.$('a.card__image-container');\n"
"      const href = linkHandle ? await linkHandle.getAttribute('href') : null;\n"
"      if (!href) continue;\n"
"\n"
"      let title = null;\n"
"      const h3Handle = await article.$('h3');\n"
"      if (h3Handle) {\n"
"        title = (await h3Handle.innerText()).trim();\n"
"      }\n"
"      if (!title) {\n"
"        const titleLink = await article.$('a.card__title');\n"
"        if (titleLink) {\n"
"          title = (await titleLink.innerText()).trim();\n"
"        }\n"
"      }\n"
"      if (!title && linkHandle) {\n"
"        const img = await linkHandle.$('img');\n"
"        if (img) {\n"
"          title = await img.getAttribute('alt');\n"
"        }\n"
"      }\n"
"\n"
"      if (title && href) {\n"
"        debugLog(`Article ${index + 1} title: \"${title}\"`);\n"
"        let url = href.startsWith('http') ? href : 'https://www.bbcgoodfood.com' + href;\n"
"        url = url.split('?')[0];  // Strip query parameters for clean URLs\n"
"        recipes.push({\n"
"          title,\n"
"          url,\n"
"        });\n"
"      }\n"
"    }\n"
"\n"
"    debugLog(`Found ${recipes.length} recipes`);\n"
"\n"
"    const unique = new Map();\n"
"    for (const r of recipes) {\n"
"      unique.set(r.url, r);\n"
"    }\n"
"    const finalRecipes = Array.from(unique.values());\n"
"\n"
"    debugLog('After deduplication: ' + finalRecipes.length);\n"
"\n"
"    console.log(JSON.stringify(finalRecipes));\n"
"  } catch (error) {\n"
"    console.error('[JS] Error:', error);\n"
"    console.log('[]');\n"
"  } finally {\n"
"    await browser.close();\n"
"    debugLog('Browser closed');\n"
"  }\n"
"})();\n";


// --------------------------------


// BBC Good Food C Parser
void parse_bbcgoodfood(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char temp_filename[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "BBC GOODFOOD PARSER Failed to get temp path.\n");
        return;
    }

    if (!GetTempFileNameA(temp_path, "bbc", 0, temp_filename)) {
        fprintf(stderr, "BBC GOODFOOD PARSER Failed to create temp JS file.\n");
        return;
    }

    FILE *tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        fprintf(stderr, "BBC GOODFOOD PARSER Failed to open temp JS file.\n");
        DeleteFileA(temp_filename);
        return;
    }
#else
    strcpy(temp_filename, "/tmp/bbcgoodfood_XXXXXX.js");
    int fd = mkstemps(temp_filename, 3);  // ".js"
    if (fd == -1) {
        perror("[BBC GOODFOOD PARSER] mkstemps");
        return;
    }

    FILE *tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("BBC GOODFOOD PARSER fdopen");
        close(fd);
        return;
    }
#endif

    fputs(bbcgoodfood_js_code, tmp_fp);
    fclose(tmp_fp);

    char *command = NULL;

#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    const char *program_files = getenv("ProgramFiles");

    if (!appdata || !program_files) {
        fprintf(stderr, "BBC GOODFOOD PARSER Missing APPDATA or ProgramFiles environment variables.\n");
        DeleteFileA(temp_filename);
        return;
    }

    char node_path_full[MAX_PATH];
    snprintf(node_path_full, sizeof(node_path_full), "%s\\npm\\node_modules", appdata);

    char node_exe[MAX_PATH];
    snprintf(node_exe, sizeof(node_exe), "\"%s\\nodejs\\node.exe\"", program_files);

    char quoted_term[1024];
    snprintf(quoted_term, sizeof(quoted_term), "\"%s\"", search_term);

    // Dynamically calculate required size
    size_t needed = snprintf(NULL, 0,
        "cmd /c \"set NODE_PATH=%s&& %s \"%s\" %s\"",
        node_path_full, node_exe, temp_filename, quoted_term) + 1;

    command = malloc(needed);
    if (!command) {
        fprintf(stderr, "BBC GOODFOOD PARSER Failed to allocate memory for command string.\n");
        DeleteFileA(temp_filename);
        return;
    }

    snprintf(command, needed,
        "cmd /c \"set NODE_PATH=%s&& %s \"%s\" %s\"",
        node_path_full, node_exe, temp_filename, quoted_term);

#else
    const char *node_path = "/opt/homebrew/lib/node_modules";

    size_t needed = snprintf(NULL, 0,
        "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
        node_path, temp_filename, search_term) + 1;

    command = malloc(needed);
    if (!command) {
        perror("BBC GOODFOOD PARSER malloc");
        unlink(temp_filename);
        return;
    }

    snprintf(command, needed,
        "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
        node_path, temp_filename, search_term);
#endif

    printf("BBC GOODFOOD PARSER Executing command: %s\n", command);

    FILE *fp = popen(command, "r");
    free(command);  // Free right after spawning process
    if (!fp) {
        fprintf(stderr, "BBC GOODFOOD PARSER Failed to run JS script.\n");
#ifdef _WIN32
        DeleteFileA(temp_filename);
#else
        unlink(temp_filename);
#endif
        add_link(out, "Click to see BBC Good Food Recipes", "", "https://www.bbcgoodfood.com/search", link_set);
        return;
    }

    char line[2048];
    GString *full_output = g_string_new("");

    while (fgets(line, sizeof(line), fp)) {
        printf("[JS OUTPUT] %s", line);
        g_string_append(full_output, line);
    }

    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "BBC GOODFOOD PARSER JS script exited with status %d\n", status);
    }

#ifdef _WIN32
    DeleteFileA(temp_filename);
#else
    unlink(temp_filename);
#endif

    printf("BBC GOODFOOD PARSER JS script complete. Output length: %zu bytes\n", full_output->len);

    struct json_object *parsed_json = json_tokener_parse(full_output->str);
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr,
                "[Recipe Finder Error] BBC Good Food script failed or returned invalid JSON.\n"
                "BBC GOODFOOD PARSER Raw JS output:\n%s\n", full_output->str);
        add_link(out, "Click to see BBC Good Food Recipes", "", "https://www.bbcgoodfood.com/search", link_set);
        g_string_free(full_output, TRUE);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    printf("BBC GOODFOOD PARSER Parsed %zu recipes from JSON.\n", n);

    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj, *url_obj;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {

            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);

            if (!title || !url || strlen(title) == 0 || strlen(url) == 0) {
                printf("BBC GOODFOOD PARSER Skipping recipe with empty title or url.\n");
                continue;
            }

            char *clean_url = strdup(url);
            char *question_mark = strchr(clean_url, '?');
            if (question_mark) *question_mark = '\0';

            printf("BBC GOODFOOD PARSER: Adding recipe: %s -> %s\n", title, clean_url);
            add_link(out, title, "", clean_url, link_set);
            free(clean_url);
        } else {
            printf("BBC GOODFOOD PARSER  JSON item missing title or url\n");
        }
    }

    json_object_put(parsed_json);
    g_string_free(full_output, TRUE);

    if (*out == NULL) {
        printf("BBC GOODFOOD PARSER:  No matching recipes found. Using fallback link.\n");

        char fallback_url[1024];
        snprintf(fallback_url, sizeof(fallback_url),
                 "https://www.bbcgoodfood.com/search?q=%s",
                 curl_escape(search_term, 0));

        add_link(out, "Click to see BBC Good Food Recipes", "", fallback_url, link_set);
    }
}

// ==================
// ==================


// Bon Appetit JavaScript:
static const char *bonappetit_js_code =
"const { chromium } = require('playwright');\n"
"(async () => {\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const page = await browser.newPage();\n"
"  const term = process.argv[2] || 'chicken';\n"
"  const url = `https://www.bonappetit.com/search?q=${encodeURIComponent(term)}`;\n"
"\n"
"  await page.goto(url, { waitUntil: 'domcontentloaded', timeout: 20000 });\n"
"\n"
"  // Increase the wait time to ensure page is fully loaded\n"
"  await page.waitForSelector('a[href*=\"/recipe/\"]', { timeout: 15000 });\n"  // Increased timeout to 15s\n"
"\n"
"  const results = await page.evaluate(() => {\n"
"    const seen = new Set();\n"
"    const anchors = Array.from(document.querySelectorAll('a[href*=\"/recipe/\"]'));\n"
"    return anchors.map(a => {\n"
"      const url = a.href;\n"
"      const title = a.querySelector('h3, h4, span')?.innerText?.trim() || a.innerText.trim() || 'Untitled';\n"
"      return { title, url };\n"
"    }).filter(item => {\n"
"      return item.url.startsWith('https://www.bonappetit.com/recipe/') &&\n"
"             !seen.has(item.url) && seen.add(item.url);\n"
"    });\n"
"  });\n"
"\n"
"  console.log(JSON.stringify(results, null, 2));\n"
"  await browser.close();\n"
"})().catch((err) => {\n"
"  console.error(\"Error during scraping:\", err);\n"
"  process.exit(1);\n"
"});\n";


// --------------------------------

// Bon Appetit Parser
static void parse_bonappetit(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char temp_filename[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "Failed to get Windows temp path.\n");
        return;
    }

    if (!GetTempFileNameA(temp_path, "ba", 0, temp_filename)) {
        fprintf(stderr, "Failed to get Windows temp file name.\n");
        return;
    }

    FILE *tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        fprintf(stderr, "Failed to create temp JS file.\n");
        DeleteFileA(temp_filename);
        return;
    }
#else
    strcpy(temp_filename, "/tmp/bonappetit_XXXXXX.js");
    int fd = mkstemps(temp_filename, 3);  // ".js"
    if (fd == -1) {
        perror("mkstemps");
        return;
    }

    FILE *tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("fdopen");
        close(fd);
        return;
    }
#endif

    fputs(bonappetit_js_code, tmp_fp);
    fclose(tmp_fp);

    char *command = NULL;

#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    if (!appdata) {
        fprintf(stderr, "APPDATA environment variable not set.\n");
        DeleteFileA(temp_filename);
        return;
    }

    char node_path_full[MAX_PATH];
    snprintf(node_path_full, sizeof(node_path_full), "%s\\npm\\node_modules", appdata);

    char node_exe[MAX_PATH] = "node";
    const char *program_files = getenv("ProgramFiles");
    if (program_files) {
        snprintf(node_exe, sizeof(node_exe), "\"%s\\nodejs\\node.exe\"", program_files);
    }

    // Dynamically size the command string
    size_t needed = snprintf(NULL, 0,
        "cmd /c \"set NODE_PATH=%s&& %s \"%s\" \"%s\"\"",
        node_path_full, node_exe, temp_filename, search_term) + 1;

    command = malloc(needed);
    if (!command) {
        fprintf(stderr, "Failed to allocate memory for command string.\n");
        DeleteFileA(temp_filename);
        return;
    }

    snprintf(command, needed,
        "cmd /c \"set NODE_PATH=%s&& %s \"%s\" \"%s\"\"",
        node_path_full, node_exe, temp_filename, search_term);
#else
    const char *node_path = "/opt/homebrew/lib/node_modules";
    size_t needed = snprintf(NULL, 0,
        "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
        node_path, temp_filename, search_term) + 1;

    command = malloc(needed);
    if (!command) {
        perror("malloc");
        unlink(temp_filename);
        return;
    }

    snprintf(command, needed,
        "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
        node_path, temp_filename, search_term);
#endif

    FILE *fp = popen(command, "r");
    free(command); // Free command immediately after use
    if (!fp) {
        fprintf(stderr,
            "\n[Recipe Finder Error]\n"
            "Bon Appetit parser failed to run Node.js command.\n"
#ifdef _WIN32
            "Please ensure:\n"
            "  - Node.js is installed (https://nodejs.org)\n"
            "  - Playwright is installed (`npm install -g playwright`)\n"
            "  - Browsers are installed (`playwright install`)\n"
            "  - Node.exe is available in PATH or installed in %%ProgramFiles%%\\nodejs\\node.exe\n\n"
#else
            "Please ensure:\n"
            "  - Node.js is installed (https://nodejs.org)\n"
            "  - Playwright is installed (`npm install -g playwright`)\n"
            "  - Browsers are installed (`playwright install`)\n"
            "  - Node is available in PATH\n\n"
#endif
            "Defaulting to Bon Appetit search page...\n\n");
#ifdef _WIN32
        DeleteFileA(temp_filename);
#else
        unlink(temp_filename);
#endif
        add_link(out, "Click to see Bon Appetit Recipes Search Page", "", "https://www.bonappetit.com/recipes", link_set);
        return;
    }

    char buffer[8192];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    pclose(fp);

#ifdef _WIN32
    DeleteFileA(temp_filename);
#else
    unlink(temp_filename);
#endif

    struct json_object *parsed_json = json_tokener_parse(buffer);

    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr,
            "\n[Recipe Finder Error]\n"
            "Bon Appetit parser failed or returned invalid data.\n\n"
            "Please ensure:\n"
            "  - Node.js is installed (https://nodejs.org)\n"
            "  - Playwright is installed (`npm install -g playwright`)\n"
            "  - Browsers are installed (`playwright install`)\n"
            "  - Node is available in PATH\n\n"
            "Defaulting to Bon Appetit search page...\n\n");

        add_link(out, "Click to see Bon Appetit Recipes Search Page", "", "https://www.bonappetit.com/recipes", link_set);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj, *url_obj;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);
            add_link(out, title, "", url, link_set);
        }
    }

    json_object_put(parsed_json);

    if (*out == NULL) {
        add_link(out, "Click to see Bon Appetit Recipes Search Page", "", "https://www.bonappetit.com/recipes", link_set);
    }
}


// ==================
// ==================



// Budget Bytes JavaScript
// Has dynamic fallback behavior that injects the UI search term into the fallback website page.

static const char *budgetbytes_js_code =
"const https = require('https');\n"
"const term = process.argv[2] || 'chicken';\n"
"const url = `https://search.slickstream.com/search?site=budgetbytes.com&q=${encodeURIComponent(term)}`;\n"
"\n"
"https.get(url, (res) => {\n"
"  let data = '';\n"
"  res.on('data', chunk => data += chunk);\n"
"  res.on('end', () => {\n"
"    try {\n"
"      const json = JSON.parse(data);\n"
"      const results = json.results.map(r => ({ title: r.title, url: r.url }));\n"
"      if (!results.length) throw new Error('Empty results');\n"
"      console.log(JSON.stringify(results, null, 2));\n"
"    } catch (e) {\n"
"      const fallbackTitle = `Search for \\\"${term}\\\" on Budget Bytes`;\n"
"      const fallbackURL = `https://www.budgetbytes.com/?s=${encodeURIComponent(term)}`;\n"
"      console.error('[Debug] Fallback triggered:', e.message);\n"
"      console.log(JSON.stringify([{ title: fallbackTitle, url: fallbackURL }]));\n"
"      process.exit(1);\n"
"    }\n"
"  });\n"
"}).on('error', (e) => {\n"
"  const fallbackTitle = `Search for \\\"${term}\\\" on the BudgetBytes.com Website`;\n"
"  const fallbackURL = `https://www.budgetbytes.com/?s=${encodeURIComponent(term)}`;\n"
"  console.error('This HTTP error was triggered:', e.message);\n"
"  console.error('Creating BudgetBytes fallback recipe link.');\n"
"  console.log(JSON.stringify([{ title: fallbackTitle, url: fallbackURL }]));\n"
"  process.exit(1);\n"
"});\n";


// --------------------------------


// Budget Bytes C Parser:
static void parse_budgetbytes(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char temp_filename[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "Failed to get Windows temp path.\n");
        return;
    }
    if (!GetTempFileNameA(temp_path, "bb", 0, temp_filename)) {
        fprintf(stderr, "Failed to get Windows temp file name.\n");
        return;
    }
    FILE *tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        fprintf(stderr, "Failed to create temp JS file.\n");
        return;
    }
#else
    strcpy(temp_filename, "/tmp/budgetbytes_XXXXXX.js");
    int fd = mkstemps(temp_filename, 3);
    if (fd == -1) {
        perror("mkstemps");
        return;
    }
    FILE *tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("fdopen");
        close(fd);
        return;
    }
#endif

    fputs(budgetbytes_js_code, tmp_fp);
    fclose(tmp_fp);

    char command[1024];

#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    if (!appdata) {
        fprintf(stderr, "APPDATA environment variable not set.\n");
        DeleteFileA(temp_filename);
        return;
    }

    char node_exe[MAX_PATH] = "node";
    const char *program_files = getenv("ProgramFiles");
    if (program_files) {
        snprintf(node_exe, sizeof(node_exe), "\"%s\\nodejs\\node.exe\"", program_files);
    }

    snprintf(command, sizeof(command),
             "cmd /c \"set NODE_PATH=%s\\npm\\node_modules&& %s \"%s\" \"%s\"\"",
             appdata, node_exe, temp_filename, search_term);
#else
    const char *node_path = "/opt/homebrew/lib/node_modules";
    snprintf(command, sizeof(command),
             "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
             node_path, temp_filename, search_term);
#endif

    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr,
                "[Recipe Finder Error] Budget Bytes parser failed to run Node.js command.\n");
#ifdef _WIN32
        DeleteFileA(temp_filename);
#else
        unlink(temp_filename);
#endif
        add_link(out, "Click to see Budget Bytes Search Page", "", "https://www.budgetbytes.com/recipes", link_set);
        return;
    }

    char buffer[8192];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    pclose(fp);

#ifdef _WIN32
    DeleteFileA(temp_filename);
#else
    unlink(temp_filename);
#endif

    struct json_object *parsed_json = json_tokener_parse(buffer);

    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr, "[Recipe Finder Error] Budget Bytes parser returned invalid data.\n");
        add_link(out, "Click to see Budget Bytes Search Page", "", "https://www.budgetbytes.com/recipes", link_set);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj, *url_obj;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);
            add_link(out, title, "", url, link_set);
        }
    }

    json_object_put(parsed_json);

    if (*out == NULL) {
        add_link(out, "Click to see Budget Bytes Search Page", "", "https://www.budgetbytes.com/recipes", link_set);
    }
}


// ===============
// ===============



// Chowhound C function (no JavaScript is used)
// Note: The Chowhound.com website does not have any search boxes to look for specific recipes.
// So this skips running Node.js script and just adds a fallback chowhound.com link pointing to Chowhound's
//   main recipe category page.
// This removes any dependency on Node.js or scraping, and guarantees a valid link every time.

static void parse_chowhound(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;
    (void)search_term;

    // Always add a single fallback link to Chowhound recipes page
    add_link(out,
             "Click to see main Chowhound Recipe Page",
             "",
             "https://www.chowhound.com/category/recipes/",
             link_set);
}



// ==================
// ==================


// Cook's Illustrated /  America's Test Kitchen JavaScript (supplied by DeepSeek)

static const char *cooksillustrated_js_code =
"const { chromium } = require('playwright');\n"
"(async () => {\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const context = await browser.newContext();\n"
"  const page = await context.newPage();\n"
"  const term = process.argv[2] || 'chili';\n"
"  const url = `https://www.americastestkitchen.com/search?q=${encodeURIComponent(term)}`;\n"
"\n"
"  await page.goto(url, { \n"
"    waitUntil: 'domcontentloaded', \n"
"    timeout: 30000,\n"
"  });\n"
"\n"
"  await Promise.race([\n"
"    page.waitForSelector('a[href*=\"/recipes/\"]', { timeout: 15000 }),\n"
"    page.waitForSelector('.no-results', { timeout: 15000 })\n"
"  ]);\n"
"\n"
"  const links = await page.evaluate(() => {\n"
"    const seen = new Set();\n"
"    return Array.from(document.querySelectorAll('a[href*=\"/recipes/\"]'))\n"
"      .map(a => a.href)\n"
"      .filter(href => href.startsWith('https://www.americastestkitchen.com/recipes/') && !seen.has(href) && seen.add(href))\n"
"      .slice(0, 10);\n"
"  });\n"
"\n"
"  if (links.length === 0) {\n"
"    console.log(JSON.stringify([{\n"
"      title: \"No recipes found - try another search\",\n"
"      url: `https://www.americastestkitchen.com/search?q=${encodeURIComponent(term)}`\n"
"    }], null, 2));\n"
"    await browser.close();\n"
"    return;\n"
"  }\n"
"\n"
"  const results = await Promise.all(links.map(async (link) => {\n"
"    const newPage = await context.newPage();\n"
"    try {\n"
"      await newPage.goto(link, { \n"
"        waitUntil: 'domcontentloaded',\n"
"        timeout: 20000\n"
"      });\n"
"      \n"
"      const title = await newPage.evaluate(() => {\n"
"        const el = document.querySelector('h1');\n"
"        return el ? el.innerText.trim() : 'Untitled';\n"
"      });\n"
"      \n"
"      await newPage.close();\n"
"      return { title, url: link };\n"
"    } catch (err) {\n"
"      await newPage.close();\n"
"      return { title: \"Error loading recipe\", url: link };\n"
"    }\n"
"  }));\n"
"\n"
"  console.log(JSON.stringify(results.filter(Boolean), null, 2));\n"
"  await browser.close();\n"
"})().catch(err => {\n"
"  console.error(\"Error during scraping:\", err);\n"
"  process.exit(1);\n"
"});\n";


// --------------------------------

// Cook's Illustrated / America's Test Kitchen C Parser (supplied by DeepSeek)

static void parse_cooksillustrated(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char temp_filename[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "Failed to get Windows temp path.\n");
        return;
    }

    if (!GetTempFileNameA(temp_path, "ci", 0, temp_filename)) {
        fprintf(stderr, "Failed to get Windows temp file name.\n");
        return;
    }

    FILE *tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        fprintf(stderr, "Failed to create temp JS file.\n");
        return;
    }
#else
    strcpy(temp_filename, "/tmp/cooksillustrated_XXXXXX.js");
    int fd = mkstemps(temp_filename, 3);  // ".js"
    if (fd == -1) {
        perror("mkstemps");
        return;
    }

    FILE *tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("fdopen");
        close(fd);
        return;
    }
#endif

    fputs(cooksillustrated_js_code, tmp_fp);
    fclose(tmp_fp);

    char command[2048];  // Increased buffer size for safety

#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    if (!appdata) {
        fprintf(stderr, "APPDATA environment variable not set.\n");
        return;
    }

    char node_path_full[MAX_PATH];
    snprintf(node_path_full, sizeof(node_path_full), "%s\\npm\\node_modules", appdata);

    char node_exe[MAX_PATH];
    snprintf(node_exe, sizeof(node_exe), "node");

    const char *program_files = getenv("ProgramFiles");
    if (program_files) {
        snprintf(node_exe, sizeof(node_exe), "\"%s\\nodejs\\node.exe\"", program_files);
    }

    snprintf(command, sizeof(command),
        "cmd /c \"set NODE_PATH=%s&& %s \"%s\" \"%s\"\"",
        node_path_full, node_exe, temp_filename, search_term);
#else
    const char *node_path = "/opt/homebrew/lib/node_modules";
    snprintf(command, sizeof(command),
        "timeout 60 NODE_PATH=\"%s\" node \"%s\" \"%s\"",
        node_path, temp_filename, search_term);
#endif

    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr, "Error running Node.js script.\n");
        add_link(out, "Click to see America's Test Kitchen Recipes", "", "https://www.americastestkitchen.com/recipes", link_set);
        return;
    }

    char buffer[32768] = {0};
    size_t total_len = 0;
    char chunk[1024];
    while (fgets(chunk, sizeof(chunk), fp) != NULL && total_len < sizeof(buffer) - 1) {
        strncpy(buffer + total_len, chunk, sizeof(buffer) - total_len - 1);
        total_len += strlen(chunk);
    }
    pclose(fp);

    struct json_object *parsed_json = json_tokener_parse(buffer);

    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr, "Failed to parse results from Node.js.\n");
        add_link(out, "Click to see America's Test Kitchen Recipes", "", "https://www.americastestkitchen.com/recipes", link_set);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    if (n == 0) {
        fprintf(stderr, "No results found for search term: %s\n", search_term);
        add_link(out, "No recipes found for your search term", "", "https://www.americastestkitchen.com/recipes", link_set);
    } else {
        for (size_t i = 0; i < n; ++i) {
            struct json_object *item = json_object_array_get_idx(parsed_json, i);
            struct json_object *title_obj, *url_obj;

            if (json_object_object_get_ex(item, "title", &title_obj) &&
                json_object_object_get_ex(item, "url", &url_obj)) {
                const char *title = json_object_get_string(title_obj);
                const char *url = json_object_get_string(url_obj);
                add_link(out, title, "", url, link_set);
            }
        }
    }

    json_object_put(parsed_json);

    if (*out == NULL) {
        add_link(out, "Click to see Cook's Illustrated / ATK Recipes", "", "https://www.americastestkitchen.com/recipes", link_set);
    }
}


// ===============
// ===============


// Delish JavaScript

static const char *delish_js_code =
"const { chromium } = require('playwright');\n"
"console.log('[JS INFO]: Starting Playwright script...');\n"
"(async () => {\n"
"  console.log('[JS INFO]: Launching browser...');\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const page = await browser.newPage();\n"
"  const term = process.argv[2] || 'chicken';\n"
"  console.log('[JS INFO]: Search term:', term);\n"
"  const url = `https://www.delish.com/search/?s=${encodeURIComponent(term)}`;\n"
"  console.log('[JS INFO]: Constructed URL:', url);\n"
"  await page.goto(url, { waitUntil: 'domcontentloaded', timeout: 20000 });\n"
"  console.log('[JS INFO]: Page loaded, waiting for selector...');\n"
"  await page.waitForSelector('a.card__link', { timeout: 15000 });\n"
"  console.log('[JS INFO]: Selector found, extracting results...');\n"
"  const results = await page.evaluate(() => {\n"
"    const seen = new Set();\n"
"    return Array.from(document.querySelectorAll('a.card__link'))\n"
"      .map(a => ({ title: a.innerText.trim(), url: a.href }))\n"
"      .filter(item => item.url.includes('/recipe/') && !seen.has(item.url) && seen.add(item.url));\n"
"  });\n"
"  console.log('Scraped results:', JSON.stringify(results, null, 2));\n"
"  await browser.close();\n"
"  console.log('Browser closed. Scraping complete.');\n"
"})().catch(err => { console.error('[JS INFO]: Error during scraping:', err); process.exit(1); });\n";


// --------------------------------


// Delish parser C function

// Dynamically determines the global installation path of the Playwright
// module using `npm root -g`. This approach ensures portability by
// auto-detecting the actual install location, making the script resilient
// to different system configurations by avoiding hard-coded paths.

static void parse_delish(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;
    char temp_filename[512];
    FILE *tmp_fp;
    char command[2048];
    char fallback[1024];
    char link_text[256];
    int fd;  // Declare fd here, so it can be used across all platforms.
    // Log the search term being passed in
    printf("[INFO]: Parsing Delish using search term: %s\n", search_term);

//  Windows -----------------

#ifndef _WIN32
    strcpy(temp_filename, "/tmp/delish_XXXXXX.js");
    fd = mkstemps(temp_filename, 3);
    if (fd == -1) {
        printf("[WARNING]: Error creating Delish temp file\n");
        return;
    }
    printf("[INFO]: Temporary JS file created: %s\n", temp_filename);
#else
    // Windows setup
    char temp_path[MAX_PATH];
    DWORD temp_length = GetTempPathA(MAX_PATH, temp_path);
    if (temp_length == 0) {
        printf("[WARNING]: Error getting Delish temp path on Windows.\n");
        return;
    }
    snprintf(temp_filename, sizeof(temp_filename), "%s\\delish_temp.js", temp_path);
    printf("[INFO]: Temporary Delish JS file created at: %s\n", temp_filename);
    // Create temp file on Windows
    tmp_fp = fopen(temp_filename, "w");
    if (tmp_fp == NULL) {
        printf("[WARNING]: Error creating temp Delish file on Windows.\n");
        return;
    }
    fd = fileno(tmp_fp);  // Get the file descriptor for Windows
#endif
    tmp_fp = fdopen(fd, "w");
    if (tmp_fp == NULL) {
        printf("[WARNING]: Error opening temporary Delish file for writing\n");
        return;
    }
    printf("[INFO]: Writing Delish JavaScript code to temporary file...\n");
    fputs(delish_js_code, tmp_fp);
    fclose(tmp_fp);
    printf("[INFO]: Delish JavaScript code temporary file was closed.\n");

//  macOS/Linux -----------------

#ifndef _WIN32
    // Dynamically detect NODE_PATH on macOS/Linux ---
    char node_path[512] = "/usr/local/lib/node_modules"; // default fallback
    FILE *np = popen("npm root -g", "r");
    if (np) {
        if (fgets(node_path, sizeof(node_path), np)) {
            node_path[strcspn(node_path, "\n")] = 0; // strip newline
        }
        pclose(np);
    }
    // -------------------------------------------------------------
    snprintf(command, sizeof(command),
             "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
             node_path, temp_filename, search_term);
#else
    // Windows: NODE_PATH not needed if Playwright installed globally
    snprintf(command, sizeof(command),
             "node \"%s\" \"%s\"",
             temp_filename, search_term);
#endif
    printf("[INFO]: Delish parser executing command: %s\n", command);
    snprintf(fallback, sizeof(fallback),
             "https://www.delish.com/search/?q=%s", search_term);
    snprintf(link_text, sizeof(link_text),
             "Click to see %s recipes on the Delish website", search_term);
    printf("[INFO]: Fallback Delish URL: %s\n", fallback);
    printf("[INFO]: Delish link text: %s\n", link_text);
    // Run the script
    int ret = system(command);
    if (ret != 0) {
        // Handle fallback in case of failure
        printf("[ALERT]: Delish parser error executing NODE_PATH command.\n");
        printf("         Return code: %d\n", ret);
        printf("         Creating a Delish fallback recipe link.\n");
        add_link(out, link_text, "", fallback, link_set);
        return;
    }
    // Log success and continue processing
    printf("[INFO]: Delish parser JavaScript executed successfully.\n");
    add_link(out, link_text, "", fallback, link_set);
}


// ===============
// ===============


// Eating Well JavaScript
static const char *eatingwell_js_code =
"const { chromium } = require('playwright');\n"
"\n"
"(async () => {\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const page = await browser.newPage();\n"
"\n"
"  // Block images, fonts, css for speed\n"
"  await page.route('**/*.{png,jpg,jpeg,css,woff,woff2}', route => route.abort());\n"
"\n"
"  const term = process.argv[2] || 'chicken';\n"
"  const url = `https://www.eatingwell.com/search/?q=${encodeURIComponent(term)}`;\n"
"\n"
"  try {\n"
"    try {\n"
"      await page.goto(url, { waitUntil: 'domcontentloaded', timeout: 8000 });\n"
"    } catch (e) {\n"
"      console.log('[]');\n"
"      await browser.close();\n"
"      return;\n"
"    }\n"
"\n"
"    const selectorPromise = page.waitForSelector('a.comp.mntl-card-list-items__link', { timeout: 4000 });\n"
"    const timeoutPromise = new Promise(resolve => setTimeout(resolve, 4000));\n"
"\n"
"    const winner = await Promise.race([selectorPromise, timeoutPromise]);\n"
"\n"
"    if (!winner) {\n"
"      console.log('[]');\n"
"      await browser.close();\n"
"      return;\n"
"    }\n"
"\n"
"    const results = await page.$$eval(\n"
"      'a.comp.mntl-card-list-items__link',\n"
"      els => Array.from(els)\n"
"        .map(a => ({ title: a.innerText.trim(), url: a.href }))\n"
"        .filter(i => i.url.includes('/recipe/'))\n"
"        .slice(0, 10)\n"
"    );\n"
"\n"
"    console.log(JSON.stringify(results.length ? results : [], null, 2));\n"
"  } catch (err) {\n"
"    // Suppress errors to minimize noise\n"
"    console.log('[]');\n"
"  } finally {\n"
"    await browser.close();\n"
"  }\n"
"})();\n";


// --------------------------------

// Eating Well C parser
// Enhanced to auto-detect NODE_PATH on macOS, verifies Playwright
// via Node require(), and prints debug info for
// npm root -g output + resolved NODE_PATH

static void parse_eatingwell(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    const char *term = (search_term && *search_term) ? search_term : "chicken";

    char temp_filename[512];
    FILE *tmp_fp = NULL;
    char command[1024];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(sizeof(temp_path), temp_path)) {
        fprintf(stderr, "[ERROR] GetTempPath failed\n");
        return;
    }
    if (!GetTempFileNameA(temp_path, "ew", 0, temp_filename)) {
        fprintf(stderr, "[ERROR] GetTempFileName failed\n");
        return;
    }
    tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        perror("[ERROR] fopen failed");
        return;
    }
#else
    strcpy(temp_filename, "/tmp/eatingwell_XXXXXX.js");
    int fd = mkstemps(temp_filename, 3);
    if (fd == -1) {
        perror("[ERROR] mkstemps failed");
        return;
    }
    tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("[ERROR] fdopen failed");
        close(fd);
        unlink(temp_filename);
        return;
    }
#endif

    fputs(eatingwell_js_code, tmp_fp);
    fclose(tmp_fp);

#ifdef _WIN32
    // Windows: no NODE_PATH override
    snprintf(command, sizeof(command),
        "cmd /c \"node \"%s\" \"%s\"\"",
        temp_filename, term);
#else
    // macOS/Linux: detect global Node module path
    char node_path[512] = {0};
    char npm_output[512] = {0};
    FILE *pathfp = popen("npm root -g 2>&1", "r"); // capture stderr too
    if (pathfp) {
        if (fgets(npm_output, sizeof(npm_output), pathfp)) {
            npm_output[strcspn(npm_output, "\n")] = '\0';
            strncpy(node_path, npm_output, sizeof(node_path) - 1);
        }
        pclose(pathfp);
    }

    if (node_path[0] == '\0') {
#ifdef __APPLE__
        strcpy(node_path, "/opt/homebrew/lib/node_modules"); // Apple Silicon Homebrew default
#else
        strcpy(node_path, "/usr/local/lib/node_modules");
#endif
    }

    // Debug: print both npm root output and resolved NODE_PATH
    fprintf(stderr, "[INFO]:  npm root -g output: %s\n", npm_output[0] ? npm_output : "(empty)");
    fprintf(stderr, "[RESOLVED]:  Using NODE_PATH: %s\n", node_path);

    // Check Playwright availability via Node require()
    char check_cmd[1024];
    snprintf(check_cmd, sizeof(check_cmd),
        "NODE_PATH=\"%s\" node -e \"require('playwright'); console.log('OK')\" 2>/dev/null",
        node_path);

    FILE *check_fp = popen(check_cmd, "r");
    int playwright_ok = 0;
    if (check_fp) {
        char check_buf[32] = {0};
        if (fgets(check_buf, sizeof(check_buf), check_fp)) {
            if (strstr(check_buf, "OK")) {
                playwright_ok = 1;
            }
        }
        pclose(check_fp);
    }

    if (!playwright_ok) {
        fprintf(stderr, "[WARNING]: Playwright could not be required using NODE_PATH=%s\n", node_path);
        fprintf(stderr, "           Try installing it globally. Run: npm install -g playwright\n");

        char fallback[1024], link_text[256];
        snprintf(fallback, sizeof(fallback), "https://www.eatingwell.com/search/?q=%s", term);
        snprintf(link_text, sizeof(link_text), "Click to see \"%s\" recipes on Eating Well", term);
        add_link(out, link_text, "", fallback, link_set);

#ifndef _WIN32
        unlink(temp_filename);
#else
        DeleteFileA(temp_filename);
#endif
        return;
    }

    snprintf(command, sizeof(command),
        "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
        node_path, temp_filename, term);
#endif

    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr, "[Eating Well] Failed to run Node.js.\n");

        char fallback[1024], link_text[256];
        snprintf(fallback, sizeof(fallback), "https://www.eatingwell.com/search/?q=%s", term);
        snprintf(link_text, sizeof(link_text), "Click to see \"%s\" recipes on Eating Well", term);
        add_link(out, link_text, "", fallback, link_set);

#ifdef _WIN32
        DeleteFileA(temp_filename);
#else
        unlink(temp_filename);
#endif
        return;
    }

    char buffer[8192];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    pclose(fp);

#ifdef _WIN32
    DeleteFileA(temp_filename);
#else
    unlink(temp_filename);
#endif

    if (len == 0) {
        char fallback[1024], link_text[256];
        snprintf(fallback, sizeof(fallback), "https://www.eatingwell.com/search/?q=%s", term);
        snprintf(link_text, sizeof(link_text), "Click to see \"%s\" recipes on Eating Well", term);
        add_link(out, link_text, "", fallback, link_set);
        return;
    }

    struct json_object *parsed_json = json_tokener_parse(buffer);
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr, "[Eating Well] Failed to parse JSON.\n");

        char fallback[1024], link_text[256];
        snprintf(fallback, sizeof(fallback), "https://www.eatingwell.com/search/?q=%s", term);
        snprintf(link_text, sizeof(link_text), "Click to see \"%s\" recipes on Eating Well", term);
        add_link(out, link_text, "", fallback, link_set);

        json_object_put(parsed_json);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        if (!item) continue;

        struct json_object *title_obj = NULL, *url_obj = NULL;
        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);
            add_link(out, title, "", url, link_set);
        }
    }

    json_object_put(parsed_json);

    if (*out == NULL) {
        char fallback[1024], link_text[256];
        snprintf(fallback, sizeof(fallback), "https://www.eatingwell.com/search/?q=%s", term);
        snprintf(link_text, sizeof(link_text), "Click to see \"%s\" recipes on Eating Well", term);
        add_link(out, link_text, "", fallback, link_set);
    }
}



// ==================
// ==================


// Epicurious wrapper (no JavaScript)

static void parse_epicurious_wrapper(GumboNode *root, GList **out, GHashTable *link_set, const char *search_term) {
    bool found_any = false;
    parse_epicurious(root, out, link_set, search_term, &found_any);

    if (!found_any) {
        char *encoded = url_encode(search_term);
        char fallback_url[512];
        snprintf(fallback_url, sizeof(fallback_url),
                 "https://www.epicurious.com/search?q=%s",
                 encoded);
        g_free(encoded);

        char fallback_title[256];
        snprintf(fallback_title, sizeof(fallback_title),
                 "Click to see \"%s\" on Epicurious", search_term);

        add_link(out, fallback_title, "", fallback_url, link_set);
    }
}

// Epicurious actual parsing logic
static void parse_epicurious(GumboNode *node, GList **out, GHashTable *link_set, const char *search_term, bool *found_any) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    GumboElement *elem = &node->v.element;

    if (elem->tag == GUMBO_TAG_A) {
        GumboAttribute *href = gumbo_get_attribute(&elem->attributes, "href");
        if (href && strstr(href->value, "/recipes/food/views/")) {
            const char *title = extract_anchor_text(node);
            if (!title || !*title) {
                title = "Epicurious Recipe";
            }

            char full_url[512];
            if (strncmp(href->value, "http", 4) == 0) {
                strncpy(full_url, href->value, sizeof(full_url));
            } else {
                snprintf(full_url, sizeof(full_url),
                         "https://www.epicurious.com%s", href->value);
            }

            if (!g_hash_table_contains(link_set, full_url)) {
                add_link(out, title, "", full_url, link_set);
                *found_any = true;
            }
        }
    }

    // Recurse into children
    GumboVector *children = &elem->children;
    for (unsigned i = 0; i < children->length; ++i) {
        parse_epicurious((GumboNode *)children->data[i], out, link_set, search_term, found_any);
    }
}



// ===============
// ===============



// Food52 JavaScript Code:
static const char *food52_js_code =
"const { chromium } = require('playwright');\n"
"\n"
"function timeout(ms) {\n"
"  return new Promise((_, reject) => setTimeout(() => reject(new Error('Timed out')), ms));\n"
"}\n"
"\n"
"async function extractRecipes(page) {\n"
"  return await page.evaluate(() => {\n"
"    const recipes = [];\n"
"    const anchors = document.querySelectorAll('a[href^=\"/recipes/\"]');\n"
"    anchors.forEach(a => {\n"
"      const title = a.textContent.trim();\n"
"      const url = a.href;\n"
"      if (title && title.length > 5) {\n"
"        const t = title.toLowerCase();\n"
"        if (t !== '+ add a recipe' && t !== 'next page' && !t.startsWith('go to page')) {\n"
"          recipes.push({ title, url });\n"
"        }\n"
"      }\n"
"    });\n"
"    return recipes;\n"
"  });\n"
"}\n"
"\n"
"async function main() {\n"
"  const term = process.argv[2] || 'chicken';\n"
"  let recipes = [];\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const page = await browser.newPage();\n"
"  page.setDefaultNavigationTimeout(10000);\n"
"\n"
"  console.error(`Trying Tier 1: https://food52.com/recipes/search?q=${encodeURIComponent(term)}`);\n"
"  try {\n"
"    await page.goto(`https://food52.com/recipes/search?q=${encodeURIComponent(term)}`, { waitUntil: 'domcontentloaded' });\n"
"    await page.waitForSelector('a[href^=\"/recipes/\"]', { timeout: 5000 });\n"
"    recipes = await extractRecipes(page);\n"
"    console.error('Tier 1 found', recipes.length, 'recipes');\n"
"  } catch (e) {\n"
"    console.error('Tier 1 error:', e);\n"
"  }\n"
"\n"
"  if (recipes.length === 0) {\n"
"    console.error('Trying Tier 2 - Autocomplete');\n"
"    try {\n"
"      await page.goto('https://food52.com', { waitUntil: 'domcontentloaded' });\n"
"      await page.waitForSelector('input[name=\"search\"]', { timeout: 5000 });\n"
"      await page.fill('input[name=\"search\"]', term);\n"
"      await page.waitForTimeout(500);\n"
"      await page.keyboard.press('ArrowDown');\n"
"      await page.waitForTimeout(1000);\n"
"      recipes = await extractRecipes(page);\n"
"      console.error('Tier 2 found', recipes.length, 'recipes');\n"
"    } catch (e) {\n"
"      console.error('Tier 2 error:', e);\n"
"    }\n"
"  }\n"
"\n"
"  if (recipes.length === 0) {\n"
"    console.error('Tier 3 - Fallback to DuckDuckGo search');\n"
"    try {\n"
"      const ddgUrl = `https://duckduckgo.com/?q=site:food52.com/recipes+${encodeURIComponent(term)}`;\n"
"      await page.goto(ddgUrl, { waitUntil: 'domcontentloaded' });\n"
"      await page.waitForTimeout(1000);\n"
"      const links = await page.evaluate(() => {\n"
"        const anchors = Array.from(document.querySelectorAll('a'));\n"
"        return anchors\n"
"          .map(a => {\n"
"            const href = a.href;\n"
"            const title = a.textContent.trim();\n"
"            if (href.includes('food52.com/recipes/') && title.length > 5) {\n"
"              return { title, url: href };\n"
"            }\n"
"            return null;\n"
"          })\n"
"          .filter(x => x !== null);\n"
"      });\n"
"      recipes = links;\n"
"      console.error('Tier 3 found', recipes.length, 'recipes');\n"
"    } catch (e) {\n"
"      console.error('Tier 3 error:', e);\n"
"    }\n"
"  }\n"
"\n"
"  console.error('Total recipes found:', recipes.length);\n"
"  console.log(JSON.stringify(recipes));\n"
"  await browser.close();\n"
"}\n"
"\n"
"Promise.race([\n"
"  main(),\n"
"  timeout(15000)\n"
"]).catch(e => {\n"
"  console.error('Error:', e);\n"
"  console.log('[]');\n"
"  process.exit(1);\n"
"});";


// --------------------------------


// Food52 C Parser
void parse_food52(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char temp_filename[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "[C DEBUG] Failed to get temp path.\n");
        return;
    }

    if (!GetTempFileNameA(temp_path, "f52", 0, temp_filename)) {
        fprintf(stderr, "[C DEBUG] Failed to create temp JS file.\n");
        return;
    }

    FILE *tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        fprintf(stderr, "[C DEBUG] Failed to open temp JS file.\n");
        DeleteFileA(temp_filename);
        return;
    }
#else
    // Ensure buffer is not overrun
    if (snprintf(temp_filename, sizeof(temp_filename), "/tmp/food52_XXXXXX.js") >= (int)sizeof(temp_filename)) {
        fprintf(stderr, "[C DEBUG] temp_filename buffer too small.\n");
        return;
    }

    int fd = mkstemps(temp_filename, 3);  // ".js"
    if (fd == -1) {
        perror("[C DEBUG] mkstemps");
        return;
    }

    FILE *tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("[C DEBUG] fdopen");
        close(fd);
        unlink(temp_filename);
        return;
    }
#endif

    fputs(food52_js_code, tmp_fp);
    fclose(tmp_fp);

    // Command buffer for invoking Node.js
    char command[2048];  // Large enough to avoid truncation

#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    const char *program_files = getenv("ProgramFiles");

    if (!appdata || !program_files) {
        fprintf(stderr, "[C DEBUG] Missing APPDATA or ProgramFiles environment variables.\n");
        DeleteFileA(temp_filename);
        return;
    }

    char node_path_full[MAX_PATH];
    snprintf(node_path_full, sizeof(node_path_full), "%s\\npm\\node_modules", appdata);

    char node_exe[MAX_PATH];
    snprintf(node_exe, sizeof(node_exe), "\"%s\\nodejs\\node.exe\"", program_files);

    if (snprintf(command, sizeof(command),
                 "cmd /c \"set NODE_PATH=%s&& %s \"%s\" \"%s\"\"",
                 node_path_full, node_exe, temp_filename, search_term) >= (int)sizeof(command)) {
        fprintf(stderr, "[C DEBUG] Command too long.\n");
        DeleteFileA(temp_filename);
        return;
    }
#else
    const char *node_path = "/opt/homebrew/lib/node_modules";
    if (snprintf(command, sizeof(command),
                 "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
                 node_path, temp_filename, search_term) >= (int)sizeof(command)) {
        fprintf(stderr, "[C DEBUG] Command too long.\n");
        unlink(temp_filename);
        return;
    }
#endif

    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr, "[C DEBUG] Failed to run JS script.\n");
#ifdef _WIN32
        DeleteFileA(temp_filename);
#else
        unlink(temp_filename);
#endif
        add_link(out, "Click to see Food52 Recipes", "", "https://food52.com/recipes", link_set);
        return;
    }

    char line[2048];
    GString *full_output = g_string_new("");
    GString *json_candidate = g_string_new("");

    // Read output line by line, keep the last non-empty one
    while (fgets(line, sizeof(line), fp)) {
        printf("[JS LOG] %s", line);
        g_string_assign(json_candidate, line);
        g_string_append(full_output, line);
    }
    pclose(fp);

#ifdef _WIN32
    DeleteFileA(temp_filename);
#else
    unlink(temp_filename);
#endif

    struct json_object *parsed_json = json_tokener_parse(json_candidate->str);
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr, "[C DEBUG] JSON parsing failed or wrong type.\n");
        add_link(out, "Click to see Food52 Recipes", "", "https://food52.com/recipes", link_set);
        g_string_free(full_output, TRUE);
        g_string_free(json_candidate, TRUE);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    if (n == 0) {
        add_link(out, "Click to see Food52 Recipes", "", "https://food52.com/recipes", link_set);
    } else {
        for (size_t i = 0; i < n; ++i) {
            struct json_object *item = json_object_array_get_idx(parsed_json, i);
            struct json_object *title_obj, *url_obj;

            if (json_object_object_get_ex(item, "title", &title_obj) &&
                json_object_object_get_ex(item, "url", &url_obj)) {

                const char *title = json_object_get_string(title_obj);
                const char *url = json_object_get_string(url_obj);

                if (title && url && strlen(title) > 0 && strlen(url) > 0) {
                    add_link(out, title, "", url, link_set);
                }
            }
        }
    }

    json_object_put(parsed_json);
    g_string_free(full_output, TRUE);
    g_string_free(json_candidate, TRUE);
}



// ===============
// ===============



// Food Network JavaScript:
static const char *foodnetwork_js_code =
"const { chromium } = require('playwright');\n"
"(async () => {\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const page = await browser.newPage();\n"
"  const searchTerm = process.argv[2] || 'chicken';\n"
"  const searchUrl = `https://www.foodnetwork.com/search/${encodeURIComponent(searchTerm)}-`;\n"
"\n"
"  try {\n"
"    await page.goto(searchUrl, { waitUntil: 'domcontentloaded', timeout: 30000 });\n"
"  } catch (err) {\n"
"    console.error('Failed to load page:', err);\n"
"    console.log('[]');\n"
"    await browser.close();\n"
"    return;\n"
"  }\n"
"\n"
"  // Faster scroll-to-bottom loop\n"
"  await page.evaluate(async () => {\n"
"    await new Promise(resolve => {\n"
"      let totalHeight = 0;\n"
"      const distance = 400;\n"
"      const timer = setInterval(() => {\n"
"        window.scrollBy(0, distance);\n"
"        totalHeight += distance;\n"
"        if (totalHeight >= document.body.scrollHeight) {\n"
"          clearInterval(timer);\n"
"          resolve();\n"
"        }\n"
"      }, 100);\n"
"    });\n"
"  });\n"
"\n"
"  // Slight delay to let recipes finish loading\n"
"  await page.waitForTimeout(2000);\n"
"\n"
"  const recipes = await page.evaluate((term) => {\n"
"    const results = [];\n"
"    const seen = new Set();\n"
"    const lowerTerm = term.toLowerCase();\n"
"    document.querySelectorAll('a[href*=\"/recipes/\"]').forEach(a => {\n"
"      const href = a.href;\n"
"      const title = a.textContent.trim();\n"
"      if (\n"
"        href && title &&\n"
"        !seen.has(href) &&\n"
"        /\\/recipes\\/.+\\/.+/.test(href) &&\n"
"        !href.endsWith('-recipes') &&\n"
"        title.toLowerCase().includes(lowerTerm)\n"
"      ) {\n"
"        seen.add(href);\n"
"        results.push({ title, url: href });\n"
"      }\n"
"    });\n"
"    return results;\n"
"  }, searchTerm);\n"
"\n"
"  console.log(JSON.stringify(recipes, null, 2));\n"
"  await browser.close();\n"
"})();\n";


// ------------------------------

// FoodNetwork C Parser 
static void parse_foodnetwork(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    const char *alt_terms[] = {
        "chili",
        "cincinnati chili",
        "beef chili",
        "turkey chili",
        "vegetarian chili"
    };

    int use_alternates = strcasecmp(search_term, "chili") == 0;
    int num_terms = use_alternates ? 5 : 1;

    GHashTable *seen_links = g_hash_table_new(g_str_hash, g_str_equal);

    for (int i = 0; i < num_terms; ++i) {
        const char *term = use_alternates ? alt_terms[i] : search_term;

        char temp_filename[512];

#ifdef _WIN32
        char temp_path[MAX_PATH];
        if (!GetTempPathA(MAX_PATH, temp_path)) {
            fprintf(stderr, "Failed to get Windows temp path.\n");
            continue;
        }

        if (!GetTempFileNameA(temp_path, "fn", 0, temp_filename)) {
            fprintf(stderr, "Failed to get Windows temp file name.\n");
            continue;
        }

        FILE *tmp_fp = fopen(temp_filename, "w");
        if (!tmp_fp) {
            fprintf(stderr, "Failed to create temp JS file.\n");
            DeleteFileA(temp_filename);
            continue;
        }
#else
        if (snprintf(temp_filename, sizeof(temp_filename), "/tmp/foodnetwork_XXXXXX.js") >= (int)sizeof(temp_filename)) {
            fprintf(stderr, "Temp filename buffer too small.\n");
            continue;
        }

        int fd = mkstemps(temp_filename, 3);  // ".js"
        if (fd == -1) {
            perror("mkstemps");
            continue;
        }

        FILE *tmp_fp = fdopen(fd, "w");
        if (!tmp_fp) {
            perror("fdopen");
            close(fd);
            unlink(temp_filename);
            continue;
        }
#endif

        fputs(foodnetwork_js_code, tmp_fp);
        fclose(tmp_fp);

        char command[2048];

#ifdef _WIN32
        const char *appdata = getenv("APPDATA");
        if (!appdata) {
            fprintf(stderr, "APPDATA environment variable not set.\n");
            DeleteFileA(temp_filename);
            continue;
        }

        const char *program_files = getenv("ProgramFiles");

        char node_path_full[MAX_PATH];
        char node_exe[MAX_PATH];

        snprintf(node_path_full, sizeof(node_path_full), "%s\\npm\\node_modules", appdata);
        if (program_files) {
            snprintf(node_exe, sizeof(node_exe), "\"%s\\nodejs\\node.exe\"", program_files);
        } else {
            snprintf(node_exe, sizeof(node_exe), "node");
        }

        int written = snprintf(command, sizeof(command),
            "cmd /c \"set NODE_PATH=%s&& %s \"%s\" \"%s\"\"",
            node_path_full, node_exe, temp_filename, term);

        if (written < 0 || written >= (int)sizeof(command)) {
            fprintf(stderr, "Command buffer too small or error during formatting.\n");
            DeleteFileA(temp_filename);
            continue;
        }
#else
        const char *node_path = "/opt/homebrew/lib/node_modules";
        int written = snprintf(command, sizeof(command),
            "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
            node_path, temp_filename, term);

        if (written < 0 || written >= (int)sizeof(command)) {
            fprintf(stderr, "Command buffer too small or error during formatting.\n");
            unlink(temp_filename);
            continue;
        }
#endif

        FILE *fp = popen(command, "r");
        if (!fp) {
#ifdef _WIN32
            DeleteFileA(temp_filename);
#else
            unlink(temp_filename);
#endif
            continue;
        }

        char buffer[8192];
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
        buffer[len] = '\0';
        pclose(fp);

#ifdef _WIN32
        DeleteFileA(temp_filename);
#else
        unlink(temp_filename);
#endif

        struct json_object *parsed_json = json_tokener_parse(buffer);
        if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
            json_object_put(parsed_json);
            continue;
        }

        size_t n = json_object_array_length(parsed_json);
        for (size_t j = 0; j < n; ++j) {
            struct json_object *item = json_object_array_get_idx(parsed_json, j);
            struct json_object *title_obj, *url_obj;

            if (json_object_object_get_ex(item, "title", &title_obj) &&
                json_object_object_get_ex(item, "url", &url_obj)) {
                const char *title = json_object_get_string(title_obj);
                const char *url = json_object_get_string(url_obj);

                if (title && url && strlen(title) > 0 && strlen(url) > 0 &&
                    !g_hash_table_contains(seen_links, url)) {
                    add_link(out, title, "", url, link_set);
                    g_hash_table_insert(seen_links, g_strdup(url), GINT_TO_POINTER(1));
                }
            }
        }

        json_object_put(parsed_json);
    }

    if (*out == NULL) {
        add_link(out, "Click to see FoodNetwork Search Page", "", "https://www.foodnetwork.com/search/", link_set);
    }

    g_hash_table_destroy(seen_links);
}


// ==================
// =================


// TheKitchn JavaScript Recipe Parser
// This retrieves recipe links from TheKitchn.com by first attempting a
// lightweight static scrape (Axios + Cheerio) and falling back to Playwright
// when bot detection or JS rendering is required.
// Main behavior:
//   - Prefers fast HTML parsing but launches Chromium (headful, hidden window)
//     to bypass PerimeterX and similar bot protections.
//   - Moves the window off-screen to avoid user disruption.
//   - Handles “Press and Hold” human-verification popups automatically.
//   - Extracts and de-duplicates recipe links using multiple selectors.
//   - Randomizes User-Agent, retries Axios before fallback.
//   - Always outputs JSON array of results or a fallback search link.
// Requirements:
//   - Node.js installed.
//   - npm packages: axios, cheerio, playwright (globally installed).
//   - Playwright browsers installed via: 'npx playwright install'

static const char *thekitchn_combined_js_code =
"const { chromium } = require('playwright');\n"
"const fs = require('fs');\n"
"(async () => {\n"
"  try {\n"
"    const term = process.argv[2] || 'chili';\n"
"    const searchURL = `https://www.thekitchn.com/search?q=${encodeURIComponent(term)}`;\n"
"\n"
"    console.error('[JS Info]: Launching TheKitchn stealth Chromium browser (hidden)...');\n"
"    const browser = await chromium.launch({\n"
"      headless: false, // Keep headful to bypass PerimeterX\n"
"      args: [\n"
"        '--start-minimized',\n"
"        '--window-position=-2000,0', // Move window off-screen\n"
"        '--disable-blink-features=AutomationControlled',\n"
"        '--no-sandbox',\n"
"        '--disable-setuid-sandbox',\n"
"        '--disable-dev-shm-usage'\n"
"      ]\n"
"    });\n"
"\n"
"    const context = await browser.newContext({\n"
"      userAgent: 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36',\n"
"      viewport: { width: 1280, height: 800 },\n"
"      locale: 'en-US',\n"
"      timezoneId: 'America/New_York',\n"
"      permissions: ['geolocation']\n"
"    });\n"
"\n"
"    const page = await context.newPage();\n"
"\n"
"    // Random mouse movement before navigating\n"
"    await page.mouse.move(Math.random() * 800, Math.random() * 600);\n"
"    await page.waitForTimeout(1000 + Math.random() * 2000);\n"
"\n"
"    console.error(`[JS Info]: Navigating to: ${searchURL}`);\n"
"    await page.goto(searchURL, {\n"
"      waitUntil: 'networkidle',\n"
"      timeout: 30000,\n"
"      referer: 'https://www.google.com/'\n"
"    });\n"
"\n"
"    // Handle CAPTCHA / press-and-hold\n"
"    try {\n"
"      await page.waitForSelector('text=\"Press and Hold\"', { timeout: 3000 });\n"
"      console.error('[Debug] Detected press-and-hold challenge...');\n"
"      await page.mouse.down();\n"
"      await page.waitForTimeout(3000);\n"
"      await page.mouse.up();\n"
"    } catch {}\n"
"\n"
"    await page.waitForSelector('article, [class*=\"card\"], [class*=\"recipe\"], h3', { timeout: 10000 });\n"
"\n"
"    const results = await page.$$eval('a', links =>\n"
"      links\n"
"        .map(el => ({\n"
"          title: el.innerText.trim(),\n"
"          url: el.href\n"
"        }))\n"
"        .filter(link => \n"
"          link.url.includes('/recipe-') &&\n"
"          link.title.length > 10 &&\n"
"          !link.url.includes('search')\n"
"        )\n"
"    );\n"
"\n"
"    if (results.length === 0) throw new Error('No matching recipes found');\n"
"\n"
"    results.forEach(r => {\n"
"      if (!r.url.startsWith('http')) {\n"
"        r.url = `https://www.thekitchn.com${r.url}`;\n"
"      }\n"
"    });\n"
"\n"
"    console.error(`[JS Info]: Found ${results.length} matching recipes.`);\n"
"    console.log(JSON.stringify(results));\n"
"\n"
"    await browser.close();\n"
"  } catch (e) {\n"
"    console.error('[Debug] Error:', e.message);\n"
"    const term = process.argv[2] || 'chili';\n"
"    const fallbackURL = `https://www.thekitchn.com/search?q=${encodeURIComponent(term)}`;\n"
"    const fallbackTitle = `Search for \\\"${term}\\\" on TheKitchn.com Website`;\n"
"    console.log(JSON.stringify([{ title: fallbackTitle, url: fallbackURL }]));\n"
"    process.exit(1);\n"
"  }\n"
"})();\n";


// --------------------------------


// TheKitchn C Scaffolding Parser
//  This runs a generated Node.js + Playwright script to scrape recipes from
//   TheKitchn.com, with Axios + Cheerio as the fast path.
// Platform differences (as of 8/11/2025):
//   - Windows: Works reliably; the "Are You A Human" hold-and-wait pop-up
//          browser window from thekitchn.com is hidden off-screen in the task bar.
//   - macOS: TheKitchn's "Are You A Human pop-up can interrupt parsing. This
//          behavior is less common on Windows and may be related to browser
//           fingerprint differences, GPU rendering, or event timing on macOS.
//   - Linux: Untested, but uses the same hidden, headful Chromium approach.
// Implementation details:
//   1. Creates a temporary JS file containing the combined scraper code.
//   2. Sets NODE_PATH dynamically via 'npm root -g' to resolve global modules.
//   3. Prints PATH and Node.js version for debug.
//   4. Launches Chromium in non-headless mode to avoid bot detection, keeping
//      the window hidden.
//   5. Detects and interacts with "Press and Hold" challenges where possible.
//   6. Parses JSON output from Node.js; on error, falls back to search link.
// Failure handling:
//   - If Node.js is missing, PATH is wrong, dependencies are not installed,
//     or JSON is invalid, a fallback search link is returned.
// Requirements:
//   - Node.js installed and on PATH.
//   - Axios, Cheerio, and Playwright installed globally.
//   - Playwright browsers installed ('npx playwright install').
// Special notes:
// TheKitchn.com may show temporary recipe card pages when searching.
// These are likely modal overlays or dynamic elements injected by JavaScript.
// On macOS, recipe searches may leave these pop-ups visible for a few seconds,
// and they are eventually removed.  Differences in pop-up handling are often
// due to browser or tool variations, not the operating system itself (e.g.,
// Safari vs Chrome behavior).  After multiple recipe searches, TheKitchn.com
// may trigger bot protection. This appears as a "Please verify you are a
// human" security page including a "hold and press" challenge that is
// difficult to bypass programmatically and can interrupt automated
// recipe scraping.
// On macOS, Xcode sometimes automatically generates .dSYM (Debug Symbol)
// files. These files are used to help developers translate memory addresses 
// into human-readable info for crash logs and debug traces, and they
// provide insights into application stability and bugs.

static void parse_thekitchn(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char js_path[1024];
    char command[2048];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "Failed to get Windows temp path.\n");
        return;
    }

    if (!GetTempFileNameA(temp_path, "tkp", 0, js_path)) {
        fprintf(stderr, "Failed to get temp file name for script.\n");
        return;
    }

    FILE *fp = fopen(js_path, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create JS temp file.\n");
        DeleteFileA(js_path);
        return;
    }
#else
    if (snprintf(js_path, sizeof(js_path), "/tmp/thekitchn_XXXXXX.js") >= (int)sizeof(js_path)) {
        fprintf(stderr, "JS path buffer too small.\n");
        return;
    }

    int fd = mkstemps(js_path, 3);
    if (fd == -1) {
        perror("mkstemps");
        return;
    }

    FILE *fp = fdopen(fd, "w");
    if (!fp) {
        perror("fdopen");
        close(fd);
        unlink(js_path);
        return;
    }
#endif

    // Write combined scraper JS to temp file
    fputs(thekitchn_combined_js_code, fp);
    fclose(fp);

    // === Automatic Node.js environment setup ===

    // Get global npm root directory and set NODE_PATH
    char node_path[512] = {0};
    FILE *npm_fp = popen("npm root -g", "r");
    if (npm_fp) {
        if (fgets(node_path, sizeof(node_path), npm_fp)) {
            size_t len = strlen(node_path);
            if (len > 0 && node_path[len - 1] == '\n')
                node_path[len - 1] = '\0'; // strip newline

#ifdef _WIN32
            if (_putenv_s("NODE_PATH", node_path) != 0) {
                fprintf(stderr, "[WARNING]: Failed to set NODE_PATH on Windows\n");
            } else {
                fprintf(stderr, "[C Info]: NODE_PATH set to: %s\n", node_path);
            }
#else
            if (setenv("NODE_PATH", node_path, 1) != 0) {
                perror("[WARNING]: Failed to set NODE_PATH");
            } else {
                fprintf(stderr, "[Info]: NODE_PATH set to: %s\n", node_path);
            }
#endif
        } else {
            fprintf(stderr, "[WARNING]: Could not read output from 'npm root -g'\n");
        }
        pclose(npm_fp);
    } else {
        fprintf(stderr, "[WARNING]: Could not determine global npm root for NODE_PATH\n");
    }

    // Debug current PATH environment variable
    const char *path_env = getenv("PATH");
    if (path_env) {
        fprintf(stderr, "[C Info]: Current PATH: %s\n", path_env);
    } else {
        fprintf(stderr, "[WARNING]: PATH environment variable not set\n");
    }

    // Verify Node.js version
    FILE *node_version_fp = popen("node --version", "r");
    if (node_version_fp) {
        char node_version[128];
        if (fgets(node_version, sizeof(node_version), node_version_fp)) {
            size_t len = strlen(node_version);
            if (len > 0 && node_version[len - 1] == '\n') {
                node_version[len - 1] = '\0'; // strip newline
            }
            fprintf(stderr, "[Info]: Node.js version: %s\n", node_version);
        } else {
            fprintf(stderr, "[WARNING]: Could not read Node.js version output\n");
        }
        pclose(node_version_fp);
    } else {
        fprintf(stderr, "[WARNING]: Failed to run 'node --version'\n");
    }

    // Build Node.js command to run the combined script with search_term argument
#ifdef _WIN32
    if (snprintf(command, sizeof(command), "cmd /c node \"%s\" \"%s\"", js_path, search_term) >= (int)sizeof(command)) {
        fprintf(stderr, "Command buffer too small.\n");
        DeleteFileA(js_path);
        return;
    }
#else
    if (snprintf(command, sizeof(command), "node \"%s\" \"%s\"", js_path, search_term) >= (int)sizeof(command)) {
        fprintf(stderr, "Command buffer too small.\n");
        unlink(js_path);
        return;
    }
#endif

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr,
            "\n[Recipe Finder Error]\n"
            "TheKitchn recipe parser failed to run Node.js command.\n"
            "Requirements:\n"
            "  - Node.js installed\n"
            "  - Playwright installed: npm install -g playwright\n"
            "  - Playwright browsers: npx playwright install\n\n"
            "Defaulting to TheKitchn search page...\n");

        char fallback_title[256];
        char fallback_url[512];
        snprintf(fallback_title, sizeof(fallback_title),
                 "Click to see \"%s\" on TheKitchn Website", search_term);
        snprintf(fallback_url, sizeof(fallback_url),
                 "https://www.thekitchn.com/search?q=%s", search_term);

        add_link(out, fallback_title, "", fallback_url, link_set);

#ifdef _WIN32
        DeleteFileA(js_path);
#else
        unlink(js_path);
#endif
        return;
    }

    char buffer[32768];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, pipe);
    buffer[len] = '\0';
    pclose(pipe);

#ifdef _WIN32
    DeleteFileA(js_path);
#else
    unlink(js_path);
#endif

    // Parse JSON results
    struct json_object *parsed_json = json_tokener_parse(buffer);
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr,
            "\n[Recipe Finder Error]\n"
            "TheKitchn parser returned invalid JSON.\n"
            "Node.js output was:\n%s\n"
            "Defaulting to TheKitchn search page...\n", buffer);

        char fallback_title[256];
        char fallback_url[512];
        snprintf(fallback_title, sizeof(fallback_title),
                 "Click to see \"%s\" on TheKitchn Website", search_term);
        snprintf(fallback_url, sizeof(fallback_url),
                 "https://www.thekitchn.com/search?q=%s", search_term);

        add_link(out, fallback_title, "", fallback_url, link_set);
        return;
    }

    // Process parsed JSON array to add individual recipe links
    size_t n = json_object_array_length(parsed_json);
    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj, *url_obj;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);
            add_link(out, title, "", url, link_set);
        }
    }

    json_object_put(parsed_json);

    // If no results, add fallback search page link
    if (*out == NULL) {
        char fallback_title[256];
        char fallback_url[512];
        snprintf(fallback_title, sizeof(fallback_title),
                 "Click to see \"%s\" on TheKitchn Website", search_term);
        snprintf(fallback_url, sizeof(fallback_url),
                 "https://www.thekitchn.com/search?q=%s", search_term);

        add_link(out, fallback_title, "", fallback_url, link_set);
    }
}


// ==================
// ==================



// NY Times Cooking Recipe C Parser (does not need JavaScript)

static void parse_nyt(GumboNode *root, GList **links, GHashTable *link_set, const char *search_term) {
    (void)root;

    // Default to "chicken" if no search term provided
    const char *term = (search_term && *search_term) ? search_term : "chicken";
    fprintf(stderr, "[DEBUG] NYT search term: %s\n", term);

    // URL-encode the search term for fallback URL
    char *encoded_term = url_encode(term);

    // Compose the search URL
    char search_url[512];
    snprintf(search_url, sizeof(search_url), "https://cooking.nytimes.com/search?q=%s", encoded_term);

    // Call external Node.js scraper script with term as argument
    // (Assuming you have a Node script like thekitchn does, for NYT)
    // For demonstration, assume command string:
    char command[1024];
#ifdef _WIN32
    snprintf(command, sizeof(command), "cmd /c node nyt_cooking_scraper.js \"%s\"", term);
#else
    snprintf(command, sizeof(command), "node nyt_cooking_scraper.js \"%s\"", term);
#endif

    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr,
            "[ERROR] Failed to run Node.js scraper for NYT Cooking.\n"
            "Please ensure Node.js and dependencies are installed.\n");

        // Fallback with search term included
        const char *term = (search_term && *search_term) ? search_term : "chicken";
        char *fallback_link = g_strdup_printf(
            "Click to see %s recipes on the NY Times Cooking Website\x1f%s",
            term, search_url);
        *links = g_list_prepend(*links, fallback_link);
        g_free(encoded_term);
        return;
    }

    char buffer[32768];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    int status = pclose(fp);

    if (status != 0 || len == 0) {
        fprintf(stderr, "[WARNING] Node.js scraper returned no data or failed.\n");
    // Fallback link with search term included in the button text
    char *fallback_link = g_strdup_printf(
        "Click to see %s on the NY Times Cooking Website\x1f%s",
        search_term,
        search_url);
    *links = g_list_prepend(*links, fallback_link);
    g_free(encoded_term);
    return;
    }


    // Parse JSON output from Node.js scraper
    struct json_object *parsed_json = json_tokener_parse(buffer);
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr, "[WARNING] Invalid JSON output from Node.js scraper.\n");
        char *fallback_link = g_strdup_printf(
            "Click to see %s\x1f%s",
            " on the NY Times Cooking Website", search_url);
        *links = g_list_prepend(*links, fallback_link);
        g_free(encoded_term);
        if (parsed_json) json_object_put(parsed_json);
        return;
    }

    // Extract recipe titles and URLs from JSON array
    size_t n = json_object_array_length(parsed_json);
    fprintf(stderr, "[DEBUG] Found %zu NYT recipe results\n", n);

    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj = NULL, *url_obj = NULL;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {

            const char *title = json_object_get_string(title_obj);
            const char *url_path = json_object_get_string(url_obj);
            if (!title || !url_path) continue;

            char *full_url = g_strdup_printf("https://cooking.nytimes.com%s", url_path);

            if (!g_hash_table_contains(link_set, full_url)) {
                char *link_data = g_strdup_printf("%s\x1f%s", title, full_url);
                *links = g_list_prepend(*links, link_data);
                g_hash_table_add(link_set, g_strdup(full_url));
                fprintf(stderr, "[DEBUG] Added NYT recipe: \"%s\" [%s]\n", title, full_url);
                g_free(link_data);
            }

            g_free(full_url);
        }
    }

    json_object_put(parsed_json);

    if (*links == NULL) {
        fprintf(stderr, "[INFO] No NY Times links found, adding fallback.\n");
        char *fallback_link = g_strdup_printf(
            "Click to see %s\x1f%s",
            " on the NY Times Cooking Website", search_url);
        *links = g_list_prepend(*links, fallback_link);
    }

    *links = g_list_reverse(*links);
    g_free(encoded_term);
}


// ------------------------------

// Helper for NYTimes: URL Encoding
static char *url_encode(const char *str) {
    if (!str) return g_strdup("");

    GString *encoded = g_string_new(NULL);

    for (const unsigned char *p = (const unsigned char *)str; *p; p++) {
        // Unreserved characters according to RFC 3986
        // ALPHA / DIGIT / "-" / "." / "_" / "~" do not need encoding
        if (isalnum(*p) || *p == '-' || *p == '.' || *p == '_' || *p == '~') {
            g_string_append_c(encoded, *p);
        } else {
            // Encode all other bytes as %XX hex
            g_string_append_printf(encoded, "%%%02X", *p);
        }
    }

    return g_string_free(encoded, FALSE);
}



// ===============
// ===============



// Saveur.com C parser (does not use JavaScript)
// JM Note: The French word Saveur translates to Flavor or Savor in English.
// This function uses a plural-to-singular search-term conversion to increase
// the chances of getting recipe hits on Saveur.

static void parse_saveur(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    printf("\nStarting parse_saveur()\n");

    const char *term = (search_term && *search_term) ? search_term : "chicken";
    printf("Input search term:        %s\n", term);

    char singular_term[256];
    singularize(term, singular_term, sizeof(singular_term));
    printf("Singularized search term: %s\n\n", singular_term);

    char *encoded_term = url_encode(singular_term);
    char url[1024];
    snprintf(url, sizeof(url), "https://www.saveur.com/search/%s", encoded_term);
    free(encoded_term);

    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl.\n");
        return;
    }

    char *html = calloc(1, 1);  // Empty string to start
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_saveur_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || !html || strlen(html) == 0) {
        fprintf(stderr, "Failed to fetch Saveur page.\n");
        free(html);
        return;
    }

    GumboOutput *output = gumbo_parse(html);
    search_for_saveur_links(output->root, out, link_set);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    free(html);

    if (*out == NULL) {
        char *encoded_term_fallback = url_encode(term);
        char fallback_url[1024];
        snprintf(fallback_url, sizeof(fallback_url), "https://www.saveur.com/search/%s", encoded_term_fallback);
        free(encoded_term_fallback);

        char link_text[256];
        snprintf(link_text, sizeof(link_text), "Search Saveur.com for %s recipes", term);

        add_link(out, link_text, "", fallback_url, link_set);
    }

    printf("Finished parse_saveur()\n");
}

// --------------------

// Saveur Helper: Write libcurl data into a dynamic buffer.
// This handles chunks of data as they are received from an HTTP request.
// It accumulates chunks of downloaded HTML data from libcurl into a
// dynamically growing C string buffer that resizes as more data is received.

static size_t write_saveur_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char **response_ptr = (char **)userp;

    char *new_buffer = realloc(*response_ptr, strlen(*response_ptr) + realsize + 1);
    if (!new_buffer) return 0;

    *response_ptr = new_buffer;
    strncat(*response_ptr, contents, realsize);
    return realsize;
}

// --------------------


// Saveur Helper: Recursively search GumboNode tree for recipe/article links
static void search_for_saveur_links(GumboNode *node, GList **out, GHashTable *link_set) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    if (node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute *href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href && (strstr(href->value, "/recipe/") || strstr(href->value, "/article/"))) {
            const char *url = href->value;

            // Extract visible text (if any)
            GString *title_buf = g_string_new(NULL);
            extract_saveur_text(node, title_buf);

            const char *title = NULL;
            if (title_buf->len > 0) {
                title = g_strstrip(title_buf->str);
            } else {
                // Fallback: extract slug from URL
                const char *last_slash = strrchr(url, '/');
                if (last_slash && *(last_slash + 1) == '\0') {
                    // URL ends with '/', so move back to previous slash
                    const char *prev = last_slash - 1;
                    while (prev > url && *prev != '/') prev--;
                    last_slash = prev;
                }

                if (last_slash) {
                    char slug[256];
                    snprintf(slug, sizeof(slug), "%s", last_slash + 1);

                    // Strip trailing slash from slug
                    size_t len = strlen(slug);
                    if (len > 0 && slug[len - 1] == '/') {
                        slug[len - 1] = '\0';
                    }

                    // Replace dashes with spaces
                    for (char *p = slug; *p; p++) {
                        if (*p == '-') *p = ' ';
                    }

                    // Capitalize first letter of each word
                    bool capitalize_next = true;
                    for (char *p = slug; *p; p++) {
                    if (capitalize_next && isalpha((unsigned char)*p)) {
                        *p = toupper((unsigned char)*p);
                          capitalize_next = false;

                    } else {

                          *p = tolower((unsigned char)*p);
                }

                if (*p == ' ') {
                    capitalize_next = true;
                }
            }

                    title = slug;
                    g_string_assign(title_buf, slug);

                } else {

                    // Fallback title if URL can't be parsed
                    title = "Untitled";
                }
            }

    // Debug prints
    printf("[SAVEUR DEBUG -- RAW URL]: \"%s\"\n", url);
    printf("[SAVEUR DEBUG -- ENHANCED TITLE]: \"%s\"\n", title);

            // Normalize and add the URL
            if (strstr(url, "https://") || strstr(url, "http://")) {
                add_link(out, title, "", url, link_set);
            } else {
                char full_url[1024];
                snprintf(full_url, sizeof(full_url), "https://www.saveur.com%s", url);
                add_link(out, title, "", full_url, link_set);
            }

            g_string_free(title_buf, TRUE);
        }
    }

    // Recurse into children
    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        search_for_saveur_links(children->data[i], out, link_set);
    }
}


// --------------------


// Saveur Helper: Recursively extract visible text from a GumboNode
//  (e.g. inside <a>)  to build proper recipe link titles.
static void extract_saveur_text(GumboNode *node, GString *out) {
    if (node->type == GUMBO_NODE_TEXT) {
        g_string_append(out, node->v.text.text);
    } else if (node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_TEMPLATE) {
        GumboVector *children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            extract_saveur_text(children->data[i], out);
        }
    }
}



// ==================
// ==================



// Serious Eats JavaScript
static const char *seriouseats_js_code =
"const { chromium } = require('playwright');\n"
"(async () => {\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const page = await browser.newPage();\n"
"  const term = process.argv[2] || 'chicken';\n"
"  const url = `https://www.seriouseats.com/search?q=${encodeURIComponent(term)}`;\n"
"\n"
"  await page.goto(url, { waitUntil: 'domcontentloaded', timeout: 20000 });\n"
"\n"
"  await page.evaluate(async () => {\n"
"    const distance = 300;\n"
"    let lastHeight = document.body.scrollHeight;\n"
"    while (true) {\n"
"      window.scrollBy(0, distance);\n"
"      await new Promise(resolve => setTimeout(resolve, 200));\n"
"      const newHeight = document.body.scrollHeight;\n"
"      if (newHeight === lastHeight) break;\n"
"      lastHeight = newHeight;\n"
"    }\n"
"  });\n"
"\n"
"  await page.waitForSelector('a[href*=\"-recipe\"]', { timeout: 5000 });\n"
"\n"
"  const results = await page.evaluate(() => {\n"
"    const seen = new Set();\n"
"    const anchors = Array.from(document.querySelectorAll('a'));\n"
"    return anchors.map(a => {\n"
"      const url = a.href;\n"
"      const title = a.querySelector('h3, h4, span')?.innerText?.trim() || a.innerText.trim() || 'Untitled';\n"
"      return { title, url };\n"
"    }).filter(item => {\n"
"      return item.url.startsWith('https://www.seriouseats.com/') &&\n"
"             item.url.endsWith('-recipe') &&\n"
"             !seen.has(item.url) && seen.add(item.url);\n"
"    });\n"
"  });\n"
"\n"
"  console.log(JSON.stringify(results, null, 2));\n"
"  await browser.close();\n"
"})().catch(() => process.exit(1));\n";


// --------------------------------

// Serious Eats Parser
static void parse_seriouseats(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char temp_filename[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "Failed to get Windows temp path.\n");
        return;
    }

    if (!GetTempFileNameA(temp_path, "se", 0, temp_filename)) {
        fprintf(stderr, "Failed to get Windows temp file name.\n");
        return;
    }

    FILE *tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        fprintf(stderr, "Failed to create temp JS file.\n");
        DeleteFileA(temp_filename);
        return;
    }
#else
    if (snprintf(temp_filename, sizeof(temp_filename), "/tmp/seriouseats_XXXXXX.js") >= (int)sizeof(temp_filename)) {
        fprintf(stderr, "Temp filename buffer too small.\n");
        return;
    }

    int fd = mkstemps(temp_filename, 3); // ".js"
    if (fd == -1) {
        perror("mkstemps");
        return;
    }

    FILE *tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("fdopen");
        close(fd);
        unlink(temp_filename);
        return;
    }
#endif

    fputs(seriouseats_js_code, tmp_fp);
    fclose(tmp_fp);

    // Command buffer to execute Node.js
    char command[2048];

#ifdef _WIN32
    const char *appdata = getenv("APPDATA");
    if (!appdata) {
        fprintf(stderr, "APPDATA environment variable not set.\n");
        DeleteFileA(temp_filename);
        return;
    }

    const char *program_files = getenv("ProgramFiles");

    char node_path_full[MAX_PATH];
    snprintf(node_path_full, sizeof(node_path_full), "%s\\npm\\node_modules", appdata);

    char node_exe[MAX_PATH];
    if (program_files) {
        snprintf(node_exe, sizeof(node_exe), "\"%s\\nodejs\\node.exe\"", program_files);
    } else {
        snprintf(node_exe, sizeof(node_exe), "node");
    }

    if (snprintf(command, sizeof(command),
                 "cmd /c \"set NODE_PATH=%s&& %s \"%s\" \"%s\"\"",
                 node_path_full, node_exe, temp_filename, search_term) >= (int)sizeof(command)) {
        fprintf(stderr, "Command buffer too small.\n");
        DeleteFileA(temp_filename);
        return;
    }
#else
    const char *node_path = "/opt/homebrew/lib/node_modules";
    if (snprintf(command, sizeof(command),
                 "NODE_PATH=\"%s\" node \"%s\" \"%s\"",
                 node_path, temp_filename, search_term) >= (int)sizeof(command)) {
        fprintf(stderr, "Command buffer too small.\n");
        unlink(temp_filename);
        return;
    }
#endif

    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr,
            "\n[Recipe Finder Error]\n"
            "Serious Eats parser failed to run Node.js command.\n"
#ifdef _WIN32
            "Please ensure:\n"
            "  - Node.js is installed (https://nodejs.org)\n"
            "  - Playwright is installed (`npm install -g playwright`)\n"
            "  - Browsers are installed (`playwright install`)\n"
            "  - Node.exe is available in PATH or in %%ProgramFiles%%\\nodejs\\node.exe\n\n"
#else
            "Please ensure:\n"
            "  - Node.js is installed (https://nodejs.org)\n"
            "  - Playwright is installed (`npm install -g playwright`)\n"
            "  - Browsers are installed (`playwright install`)\n"
            "  - Node is available in PATH\n\n"
#endif
            "Defaulting to Serious Eats search page...\n\n");

#ifdef _WIN32
        DeleteFileA(temp_filename);
#else
        unlink(temp_filename);
#endif

        add_link(out, "Click to see Serious Eats Search Page", "", "https://www.seriouseats.com/recipes", link_set);
        return;
    }

    char buffer[8192];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    pclose(fp);

#ifdef _WIN32
    DeleteFileA(temp_filename);
#else
    unlink(temp_filename);
#endif

    struct json_object *parsed_json = json_tokener_parse(buffer);
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr,
            "\n[Recipe Finder Error]\n"
            "Serious Eats parser failed or returned invalid data.\n\n"
            "Please ensure:\n"
            "  - Node.js is installed (https://nodejs.org)\n"
            "  - Playwright is installed (`npm install -g playwright`)\n"
            "  - Browsers are installed (`playwright install`)\n"
            "  - Node is available in PATH\n\n"
            "Defaulting to Serious Eats search page...\n\n");

        add_link(out, "Click to see Serious Eats Search Page", "", "https://www.seriouseats.com/recipes", link_set);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj, *url_obj;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);

            if (title && url && strlen(title) > 0 && strlen(url) > 0) {
                add_link(out, title, "", url, link_set);
            }
        }
    }

    json_object_put(parsed_json);

    if (*out == NULL) {
        add_link(out, "Click to see Serious Eats Search Page", "", "https://www.seriouseats.com/recipes", link_set);
    }
}


// ==================
// ==================



// SimplyRecipes recipe parser (No JavaScript used)
// Adds recipe links found in anchor tags with href containing
//    "simplyrecipes.com/recipes/"
// Adds fallback link once per search via added_fallback flag from caller.

static void parse_simplyrecipes(GumboNode *n, GList **out, GHashTable *link_set, const char *search_term) {
    (void)search_term;  // Silence unused parameter warning

    if (n->type != GUMBO_NODE_ELEMENT)
        return;

    if (n->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute *href = gumbo_get_attribute(&n->v.element.attributes, "href");
        if (href && strstr(href->value, "simplyrecipes.com/recipes/")) {
            const char *title = href->value;
            if (n->v.element.children.length > 0) {
                GumboNode *c = (GumboNode *)n->v.element.children.data[0];
                if (c->type == GUMBO_NODE_TEXT)
                    title = c->v.text.text;
            }
            add_link(out, title, "", href->value, link_set);
        }
    }

    // Recurse through children

    for (unsigned i = 0; i < n->v.element.children.length; ++i)
        parse_simplyrecipes((GumboNode *)n->v.element.children.data[i], out, link_set, search_term);
}



// ===============
// ===============



// Smitten Kitchen JavaScript

const char *smittenkitchen_js_code =
"const axios = require('axios');\n"
"const cheerio = require('cheerio');\n"
"\n"
"async function searchSmittenKitchen(term) {\n"
"  const url = `https://smittenkitchen.com/?s=${encodeURIComponent(term)}`;\n"
"  try {\n"
"    const { data } = await axios.get(url);\n"
"    const $ = cheerio.load(data);\n"
"    const results = [];\n"
"\n"
"    $('.post-list article').each((i, elem) => {\n"
"      const title = $(elem).find('h2.entry-title a').text().trim();\n"
"      const url = $(elem).find('h2.entry-title a').attr('href');\n"
"      if (title && url) {\n"
"        results.push({ title, url });\n"
"      }\n"
"    });\n"
"\n"
"    return results;\n"
"  } catch (e) {\n"
"    console.error('[SmittenKitchen] Search failed:', e.message);\n"
"    return [];\n"
"  }\n"
"}\n"
"\n"
"if (require.main === module) {\n"
"  const term = process.argv[2] || '';\n"
"  searchSmittenKitchen(term).then(results => {\n"
"    console.log(JSON.stringify(results));\n"
"  });\n"
"}\n";


// --------------------------------


// Smitten Kitchen C function

static void parse_smittenkitchen(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char script_path[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "Failed to get Windows temp path.\n");
        return;
    }
    if (!GetTempFileNameA(temp_path, "smk", 0, script_path)) {
        fprintf(stderr, "Failed to get temp file name for smittenkitchen script.\n");
        return;
    }
    FILE *fp = fopen(script_path, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create temp JS file for smittenkitchen.\n");
        return;
    }
    fputs(smittenkitchen_js_code, fp);
    fclose(fp);
    char command[1024];
    snprintf(command, sizeof(command), "cmd /c node \"%s\" \"%s\"", script_path, search_term);
#else
    strcpy(script_path, "/tmp/smittenkitchen_XXXXXX.js");
    int fd = mkstemps(script_path, 3);
    if (fd == -1) {
        perror("mkstemps (smittenkitchen)");
        return;
    }
    FILE *fp = fdopen(fd, "w");
    if (!fp) {
        perror("fdopen (smittenkitchen)");
        close(fd);
        return;
    }
    fputs(smittenkitchen_js_code, fp);
    fclose(fp);
    char command[1024];
    snprintf(command, sizeof(command), "node \"%s\" \"%s\"", script_path, search_term);
#endif

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr, "[SmittenKitchen] Failed to run Node.js command.\n");

        char fallback_url[512];
        snprintf(fallback_url, sizeof(fallback_url), "https://smittenkitchen.com/?s=%s", search_term);
        char fallback_title[512];
        snprintf(fallback_title, sizeof(fallback_title), "Search for %s on Smitten Kitchen Website", search_term);
        add_link(out, fallback_title, "", fallback_url, link_set);

#ifdef _WIN32
        DeleteFileA(script_path);
#else
        unlink(script_path);
#endif
        return;
    }

    char buffer[32768];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, pipe);
    buffer[len] = '\0';
    pclose(pipe);

#ifdef _WIN32
    DeleteFileA(script_path);
#else
    unlink(script_path);
#endif

    struct json_object *parsed_json = json_tokener_parse(buffer);
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr, "[SmittenKitchen] Invalid JSON returned.\n");

        char fallback_url[512];
        snprintf(fallback_url, sizeof(fallback_url), "https://smittenkitchen.com/?s=%s", search_term);
        char fallback_title[512];
        snprintf(fallback_title, sizeof(fallback_title), "Search for \"%s\" on Smitten Kitchen Website", search_term);
        add_link(out, fallback_title, "", fallback_url, link_set);

        if (parsed_json) json_object_put(parsed_json);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj, *url_obj;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);
            add_link(out, title, "", url, link_set);
        }
    }

    json_object_put(parsed_json);

    if (*out == NULL) {
        char fallback_url[512];
        snprintf(fallback_url, sizeof(fallback_url), "https://smittenkitchen.com/?s=%s", search_term);
        char fallback_title[512];
       snprintf(fallback_title, sizeof(fallback_title), "Search for \"%s\" on Smitten Kitchen Website", search_term);
       add_link(out, fallback_title, "", fallback_url, link_set);

    }
}



// ===============
// ===============



// The Spruce Eats JavaScript
static const char *spruce_js_code =
"const { chromium } = require('playwright');\n"
"(async () => {\n"
"  const browser = await chromium.launch({ headless: true });\n"
"  const page = await browser.newPage();\n"
"  const term = process.argv[2] || 'chicken';\n"
"  const url = `https://www.thespruceeats.com/search?q=${encodeURIComponent(term)}`;\n"
"  await page.goto(url, { waitUntil: 'domcontentloaded', timeout: 10000 });\n"
"  await page.waitForSelector('a.card__title-link', { timeout: 8000 });\n"
"  const results = await page.evaluate(() => {\n"
"    const seen = new Set();\n"
"    return Array.from(document.querySelectorAll('a.card__title-link'))\n"
"      .filter(a => a.href.includes('/recipes/') && !seen.has(a.href) && seen.add(a.href))\n"
"      .slice(0, 10)\n"
"      .map(a => ({\n"
"        title: a.innerText.trim().replace(/\\s+/g, ' '),\n"
"        url: a.href\n"
"      }));\n"
"  });\n"
"  if (!results || results.length === 0) {\n"
"    console.error('No results found.');\n"
"    process.exit(1);\n"
"  }\n"
"  console.log(JSON.stringify(results, null, 2));\n"
"  await browser.close();\n"
"})().catch(async err => {\n"
"  console.error('Error during scraping:', err);\n"
"  process.exit(1);\n"
"});\n";


// --------------------------------

static void parse_spruceeats(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    printf("Starting parse_spruceeats()\n");

    const char *term = (search_term && *search_term) ? search_term : "chicken";
    printf("Search term: %s\n", term);

    char temp_filename[512];
    FILE *tmp_fp = NULL;
    char command[1024];

#ifdef _WIN32
    // Create temporary JS file on Windows
    char temp_path[MAX_PATH];
    if (!GetTempPathA(sizeof(temp_path), temp_path)) {
        fprintf(stderr, "[WARN] Failed to get temporary path.\n");
        return;
    }

    // JM Note: There is a subtle but important detail of the Windows API.
    // The third argument (0) below is the uUnique parameter.
    // When set to 0, Windows automatically generates a unique file name
    // using the current system time, creates the file, and returns its path.
    // The resulting file name will have the "spr" prefix, the system time, and 
    // then a ".tmp" extension.
    // The temp file will be deleted later in the code.

    if (!GetTempFileNameA(temp_path, "spr", 0, temp_filename)) {
        fprintf(stderr, "[WARN] Failed to get temporary file name.\n");
        return;
    }

    tmp_fp = fopen(temp_filename, "w");
    if (!tmp_fp) {
        perror("[WARN] fopen failed");
        return;
    }

#else
    strcpy(temp_filename, "/tmp/spruceeats_XXXXXX.js");
    int fd = mkstemps(temp_filename, 3);
    if (fd == -1) {
        perror("[WARN] mkstemps failed");
        return;
    }
    tmp_fp = fdopen(fd, "w");
    if (!tmp_fp) {
        perror("[WARN] fdopen failed");
        close(fd);
        unlink(temp_filename);
        return;
    }
#endif

    if (!spruce_js_code) {
        fprintf(stderr, "[WARN] spruce_js_code is NULL\n");
#ifdef _WIN32
        fclose(tmp_fp);
#else
        fclose(tmp_fp);
        unlink(temp_filename);
#endif
        return;
    }

    printf("Writing temporary JS script file: %s\n", temp_filename);
    if (fputs(spruce_js_code, tmp_fp) < 0) {
        perror("[WARN] Failed to write JS script to temporary file");
#ifdef _WIN32
        fclose(tmp_fp);
#else
        fclose(tmp_fp);
        unlink(temp_filename);
#endif
        return;
    }

    fclose(tmp_fp);

#ifdef _WIN32
    snprintf(command, sizeof(command),
             "node \"%s\" \"%s\"",
             temp_filename, term);
#else
    snprintf(command, sizeof(command),
             "NODE_PATH=\"/usr/local/lib/node_modules\" node \"%s\" \"%s\"",
             temp_filename, term);
#endif

    printf("Running command: %s\n", command);

    char fallback[1024];
    snprintf(fallback, sizeof(fallback),
             "https://www.thespruceeats.com/search?q=%s", term);
    char link_text[256];
    snprintf(link_text, sizeof(link_text),
             "Click to see %s recipes on The Spruce Eats website", term);
    add_link(out, link_text, "", fallback, link_set);
    printf("Added fallback recipe link preemptively\n");

    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr, "[WARN] Unable to run Node script via popen().\n");
#ifdef _WIN32
        unlink(temp_filename);
#endif
        return;
    }

    printf("Reading output from JS script...\n");
    char buffer[8192];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';

    int status = pclose(fp);
#ifdef _WIN32
    unlink(temp_filename);
#else
    unlink(temp_filename);
#endif

    if (status != 0) {
        printf("spruceeats Node script exited with status %d\n", status);
        printf("Defaulting to fallback recipe link.\n");
        return;
    }

    printf("Bytes read: %zu\n", len);
    if (len == 0) {
        printf("[WARN] No data received from JS output.\n");
        return;
    }

    printf("Raw JS output:\n%s\n", buffer);

    struct json_object *parsed_json = json_tokener_parse(buffer);
    if (!parsed_json) {
        printf("[WARN] Could not parse JSON output.\n");
        return;
    }

    if (!json_object_is_type(parsed_json, json_type_array)) {
        printf("[WARN] Parsed JSON is not an array as expected.\n");
        json_object_put(parsed_json);
        return;
    }

    printf("Parsed JSON is an array.\n");

    size_t result_count = json_object_array_length(parsed_json);
    printf("Number of results: %zu\n", result_count);

    for (size_t i = 0; i < result_count; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        if (!item) {
            printf("[WARN] Null item at index %zu\n", i);
            continue;
        }

        struct json_object *title_obj = NULL, *url_obj = NULL;
        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);
            printf("Adding link: title=\"%s\", url=\"%s\"\n", title, url);
            add_link(out, title, "", url, link_set);
        } else {
            printf("[WARN] Missing title or url in item at index %zu\n", i);
        }
    }

    json_object_put(parsed_json);
    printf("Finished parse_spruceeats()\n");
}



// ===============
// ===============


// Taste of Home JavaScript

const char *tasteofhome_js_code =
"const axios = require('axios');\n"
"const cheerio = require('cheerio');\n"
"\n"
"async function searchTasteOfHome(term) {\n"
"  const url = `https://www.tasteofhome.com/?s=${encodeURIComponent(term)}`;\n"
"  try {\n"
"    const { data } = await axios.get(url);\n"
"    const $ = cheerio.load(data);\n"
"    const results = [];\n"
"\n"
"    $('.component-river-item').each((i, elem) => {\n"
"      const title = $(elem).find('h3 a').text().trim();\n"
"      const url = $(elem).find('h3 a').attr('href');\n"
"      if (title && url) {\n"
"        const fullUrl = url.startsWith('http') ? url : `https://www.tasteofhome.com${url}`;\n"
"        results.push({ title, url: fullUrl });\n"
"      }\n"
"    });\n"
"\n"
"    return results;\n"
"  } catch (e) {\n"
"    console.error('[TasteOfHome] Search failed:', e.message);\n"
"    return [];\n"
"  }\n"
"}\n"
"\n"
"if (require.main === module) {\n"
"  const term = process.argv[2] || '';\n"
"  searchTasteOfHome(term).then(results => {\n"
"    console.log(JSON.stringify(results));\n"
"  });\n"
"}\n";


// --------------------------------


// Taste of Home C function

static void parse_tasteofhome(GumboNode *unused, GList **out, GHashTable *link_set, const char *search_term) {
    (void)unused;

    char script_path[512];

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) {
        fprintf(stderr, "Failed to get Windows temp path.\n");
        return;
    }
    if (!GetTempFileNameA(temp_path, "toh", 0, script_path)) {
        fprintf(stderr, "Failed to get temp file name for tasteofhome script.\n");
        return;
    }
    FILE *fp = fopen(script_path, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create temp JS file for tasteofhome.\n");
        return;
    }
    fputs(tasteofhome_js_code, fp);
    fclose(fp);
    char command[1024];
    snprintf(command, sizeof(command), "cmd /c node \"%s\" \"%s\"", script_path, search_term);
#else
    strcpy(script_path, "/tmp/tasteofhome_XXXXXX.js");
    int fd = mkstemps(script_path, 3);
    if (fd == -1) {
        perror("mkstemps (tasteofhome)");
        return;
    }
    FILE *fp = fdopen(fd, "w");
    if (!fp) {
        perror("fdopen (tasteofhome)");
        close(fd);
        return;
    }
    fputs(tasteofhome_js_code, fp);
    fclose(fp);
    char command[1024];
    snprintf(command, sizeof(command), "node \"%s\" \"%s\"", script_path, search_term);
#endif

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr, "[TasteOfHome] Failed to run Node.js command.\n");

        char fallback_url[1024];
        char fallback_title[512];
        snprintf(fallback_url, sizeof(fallback_url), "https://www.tasteofhome.com/?s=%s", search_term);
        snprintf(fallback_title, sizeof(fallback_title), "Search for \"%s\" on Taste of Home Website", search_term);
        add_link(out, fallback_title, "", fallback_url, link_set);

#ifdef _WIN32
        DeleteFileA(script_path);
#else
        unlink(script_path);
#endif
        return;
    }

    char buffer[32768];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, pipe);
    buffer[len] = '\0';
    pclose(pipe);

#ifdef _WIN32
    DeleteFileA(script_path);
#else
    unlink(script_path);
#endif

    struct json_object *parsed_json = json_tokener_parse(buffer);
    if (!parsed_json || !json_object_is_type(parsed_json, json_type_array)) {
        fprintf(stderr, "[TasteOfHome] Invalid JSON returned.\n");

        char fallback_url[1024];
        char fallback_title[512];
        snprintf(fallback_url, sizeof(fallback_url), "https://www.tasteofhome.com/?s=%s", search_term);
        snprintf(fallback_title, sizeof(fallback_title), "Search for \"%s\" on Taste of Home Website", search_term);
        add_link(out, fallback_title, "", fallback_url, link_set);

        if (parsed_json) json_object_put(parsed_json);
        return;
    }

    size_t n = json_object_array_length(parsed_json);
    for (size_t i = 0; i < n; ++i) {
        struct json_object *item = json_object_array_get_idx(parsed_json, i);
        struct json_object *title_obj, *url_obj;

        if (json_object_object_get_ex(item, "title", &title_obj) &&
            json_object_object_get_ex(item, "url", &url_obj)) {
            const char *title = json_object_get_string(title_obj);
            const char *url = json_object_get_string(url_obj);
            add_link(out, title, "", url, link_set);
        }
    }

    json_object_put(parsed_json);

    if (*out == NULL) {
        char fallback_url[1024];
        char fallback_title[512];
        snprintf(fallback_url, sizeof(fallback_url), "https://www.tasteofhome.com/?s=%s", search_term);
        snprintf(fallback_title, sizeof(fallback_title), "Search for \"%s\" on Taste of Home Website", search_term);
        add_link(out, fallback_title, "", fallback_url, link_set);
    }
}



// ===============
// ===============



// YummlyRecipes C function (JavaScript is not needed)
// Note: This new website has some quirky food blogs.

static void parse_yummlyrecipes(GumboNode *n, GList **out, GHashTable *link_set, const char *search_term) {
    static bool added_fallback = false;

    // Add a fallback link to the main Yummly Recipes search page once
    if (!added_fallback) {
        add_link(out, "Click to see Yummly Recipes Search Page", "", "https://www.yummlyrecipes.com/", link_set);
        added_fallback = true;
    }

    if (n->type != GUMBO_NODE_ELEMENT)
        return;

    if (n->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute *href = gumbo_get_attribute(&n->v.element.attributes, "href");
        if (href && strstr(href->value, "/search/label/") && strstr(href->value, "yummlyrecipes.com")) {
            const char *url = href->value;

            // Extract the slug from the URL (e.g., "CheesyChickenCasserole")
            const char *slug = strrchr(url, '/');
            slug = (slug && *(slug + 1)) ? slug + 1 : url;

            // Prefer anchor text for title, but always convert it properly
            const char *anchor_text = extract_anchor_text(n);
            char title[256];

            if (anchor_text && *anchor_text) {
                slug_to_title(anchor_text, title, sizeof(title));
            } else {
                slug_to_title(slug, title, sizeof(title));
            }

            // Check if title matches the search term (case-insensitive)
            if (!search_term || !*search_term || contains_word_case_insensitive(title, search_term)) {
                add_link(out, title, "", url, link_set);
            }
        }
    }

    // Perform a depth-first search (DFS) through the HTML DOM tree.
    // Recursively visit all child nodes of the current element.
    // For each <a> tag containing Yummly recipe links, add_link() ensures
    // uniqueness by checking the link_set (a GHashTable).
    //
    // Example nested structure commonly found on recipe blogs:
    // <div class="post-preview">
    //   <article>
    //     <section>
    //       <h2>
    //         <a href="https://www.yummlyrecipes.com/search/label/cheesy-chicken-casserole">
    //           Cheesy Chicken Casserole
    //         </a>
    //       </h2>
    //     </section>
    //   </article>
    // </div>

    for (unsigned i = 0; 
         i < n->v.element.children.length;  // Iterate through child nodes
         ++i)  // Move to next child
    {
        parse_yummlyrecipes(
            (GumboNode *)n->v.element.children.data[i],
            out,
            link_set,
            search_term
        );
    }
}



// ==========================================================================
//  ***  END OF JAVASCRIPT AND C RECIPE PARSERS  ***
// ==========================================================================

