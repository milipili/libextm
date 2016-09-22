#ifndef LIBNYEXTM_H
#define LIBNYEXTM_H
#include <string.h>
#include <stdint.h>

#ifndef NY_EXPORT
#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef __GNUC__
#    define LIBNANYC_VISIBILITY_EXPORT   __attribute__ ((dllexport))
#    define LIBNANYC_VISIBILITY_IMPORT   __attribute__ ((dllimport))
#  else
#    define LIBNANYC_VISIBILITY_EXPORT   __declspec(dllexport) /* note: actually gcc seems to also supports this syntax */
#    define LIBNANYC_VISIBILITY_IMPORT   __declspec(dllimport) /* note: actually gcc seems to also supports this syntax */
#  endif
#else
#  define LIBNANYC_VISIBILITY_EXPORT     __attribute__((visibility("default")))
#  define LIBNANYC_VISIBILITY_IMPORT     __attribute__((visibility("default")))
#endif

#if defined(_DLL) && !defined(LIBNANYC_DLL_EXPORT)
#  define LIBNANYC_DLL_EXPORT
#endif

/*!
** \macro NY_EXPORT
** \brief Export / import a libnany symbol (function)
*/
#if defined(LIBNANYC_DLL_EXPORT)
#	define NY_EXPORT LIBNANYC_VISIBILITY_EXPORT
#else
#	define NY_EXPORT LIBNANYC_VISIBILITY_IMPORT
#endif
#endif /* NY_EXPORT */






#ifdef __cplusplus
extern "C" {
#endif

/*
** extm (Extensions Manager)
**    - source: different sources for the extension (local, git, svn, webdav...)
**    - runner: local thread or external process to run a set of extensions
**    - exension: attached to an source and to a runner (if installed and loaded)
**
**  [ext. manager]  <->  [source]    <->    [extension]  <->  [runner]
**                     local, git...                         process...
**
**
** -- Manual management
** \code
** nyextm_add(extm, "github", nyobj_str("nany-lang/nany"));
** \endcode
**
** -- For automatic management:
**
** Depository: offers a consistent snapshot of extensions (versions are fixed
** and only minor fixes will be accepted for updates)
**
** \code
** nextm_depository_source_add(extm, "npp", "https://notepad-plus-plus.org/packages/official/");
** nyextm_depository_channel_set(extm, "npp", "stable/2016.09");
** nyextm_add(extm, "pkg", nyobj_str("npp-minimal"));
** nyextm_add(extm, "pkg", nyobj_str("npp-extra-tools"));
** nyextm_add(extm, "pkg", nyobj_str("npp-i18n-english")); // or manually...
** nyextm_add(extm, "pkg", nyobj_str("npp-i18n-french"));
** nyextm_add(extm, "pkg", nyobj_str("npp-log-monitoring"));
** nyextm_add(extm, "pkg", nyobj_str("npp-file-explorer"));
** nyextm_add(extm, "pkg", nyobj_str("npp-syntax-css"));
** nyextm_add(extm, "pkg", nyobj_str("npp-syntax-cpp"));
** nyextm_add(extm, "pkg", nyobj_str("npp-syntax-d"));
** nyextm_add(extm, "pkg", nyobj_str("npp-syntax-javascript"));
** nyextm_add(extm, "pkg", nyobj_str("npp-syntax-nany"));
** nyextm_add(extm, "pkg", nyobj_str("npp-syntax-fortran77"));
**
** // Extra packages from the community, not enabled by default via installer
** // (not enabled by default since only maintained by the communauty)
** // (packages could be transparent redirection to the original github repository)
** // (but if hosted, can apply restrictions, like no executable, no dll, no side effects...)
** nextm_depository_source_add(extm, "npp-community", "https://notepad-plus-plus.org/packages/community/");
** nyextm_depository_channel_set(extm, "npp-community", "testing");
**
** nyextm_depository_sync(extm);
** nyextm_install_all(extm);
** nyextm_update_all(extm);
** \endcode
**
** For offline installation:
** \code
** nextm_depository_source_add(extm, "local", "C:\\somewhere\\installer\\...");
** [...]
** \endcode
*/


/*
** nyextm-depository --create <private-path>
** nyextm-depository --import-private-keys=FILE <private-path>
** nyextm-depository --add-package=FILE|FOLDER <private-path>
** nyextm-depository --add-manifest=FILE|FOLDER <private-path>
** nyextm-depository --stage-add=NAME <private-path>
** nyextm-depository --stage-delete=NAME <private-path>
** nyextm-depository --build <private-path>
** nyextm-depository --deploy <private-path> <htdocs-path>
**
** structure:
**     <htdocs>/stages/all.json
**     <htdocs>/stages/<stage>.json
**     <htdocs>/stages/<stage>.digest
**     <htdocs>/pkgs/<letter>/<letter2>/<letter3>/package-name-version-arch.manifest [if exists]
**     <htdocs>/pkgs/<letter>/<letter2>/<letter3>/package-name-version-arch.nypkg [if exists]
*/


/*! \name Common types */
/*@{*/
/*! Unique ID at runtime */
typedef struct { char bytes[16]; } nyeid_t;

/*! Create a new string representation of an id (ex: "{9479134c-2a25-487d-83a9-4989ec7749a4}") */
NY_EXPORT char* nyeid_tostr(const nyeid_t*);

/*! (Re)Build an id from its string representation */
NY_EXPORT char* nyeid_fromstr(nyeid_t*, const char* cstr, uint32_t len);

/*! Generate a new ID */
NY_EXPORT void nyeid_generate(nyeid_t*);

/*! Get if an id is invalid */
NY_EXPORT nybool_t nyeid_is_null(const nyeid_t*);


enum nyext_err_t
{
	/*! No error */
	nyexr_none,
	/*! Failed (generic error) */
	nyexr_failed,
	/*! Not found */
	nyexr_not_found,
	/*! Duplicate entry found */
	nyexr_duplicate,
	/*! When a source is not ready */
	nyexr_not_available,
	/*! Timeout has been reached */
	nyexr_timeout,
};


#ifndef LIBNANYC_ANYSTR_T
#define LIBNANYC_ANYSTR_T
typedef struct
{
	const char* cstr;
	uint32_t size;
}
nyanystr_t;

/*! Creat ean nyanystr_t from a c-string */
static inline nyanystr_t nycstr(const char* text)
{
	return nyanystr_t{text, strlen(text)};
}

/*! Creat ean nyanystr_t from a c-string */
static inline nyanystr_t nycstr(const char* text)
{
	return nyanystr_t{text, strlen(text)};
}

/*! Creat ean nyanystr_t from a c-string */
static inline void nycstr_duplicate(nyanystr_t* out, const nyanystr_t* src)
{
	uint32_t len = src->size;
	const char* srcstr = src->cstr;
	char* str = malloc(sizeof(char) * (len + 1));
	memcpy(str, srcstr, len);
	str[len] = '\0';
	out->cstr = str;
	out->size = len;
}

#endif

#ifndef LIBNANYC_NYBOOL_T
#define LIBNANYC_NYBOOL_T
enum nybool_t { nyfalse, nytrue };
#endif

#ifndef LIBNANYC_VERSION_T
#define LIBNANYC_VERSION_T
typedef struct
{
	uint32_t hi;
	uint32_t lo;
	uint32_t patch;
	char metadata[12]; /*zero-terminated*/
}
nyversion_t;
#endif

/*! Create an array of c-strings */
NY_EXPORT char** nystrarray_create(uint32_t count);
/*! Free an array of c-strings (and free all elements) */
NY_EXPORT void nystrarray_free(char**);
/*! Copy a string and assign it to the Nth element (free the previous if not null) */
NY_EXPORT void nystrarray_set(char**, uint32_t index, const char* str);
/*! Copy a string and assign it to the Nth element (free the previous if not null) */
NY_EXPORT void nystrarray_set_ex(char**, uint32_t index, const char* str, uint32_t len);
/*@}*/




/*! \name Extension managers */
/*@{*/
/*! Extension manager */
typedef struct nyextm_t nyextm_t;

typedef struct
{
	/*! Default extension family (ex: "nany", "python", "native", ...) */
	nyanystr_t default_family;
	/*! The extenion manager is about to be destroyed */
	void (*on_extm_created)(nyextm_t*, void*);
	/*! The extenion manager is about to be destroyed */
	void (*on_extm_destroyed)(nyextm_t*, void*);
}
nyextm_cf_t;


/*! Create a new extension manager */
nyextm_t* nyextm_create(const nyextm_cf_t*, void* userdata);
/*! Increment the ref count of an extension manager */
NY_EXPORT void nyextm_ref(nyextm_t*);
/*! Dec the ref count and destroy the extension manager if equals 0 */
NY_EXPORT void nyextm_unref(nyextm_t*);
/*@}*/



/*! \name Sources */
/*@{*/
/*! source for an extension */
typedef struct nyextm_ext_source_t nyextm_ext_source_t;

/*! Configuration for a new source */
typedef struct
{
	/*! Read the content of an inner file */
	nyext_err_t (*read_file)(nyextm_t*, nyextm_ext_source_t*, const char* path, uint32_t len, nyanystr_t* content);
	/*! Read the content of an inner folder */
	nyext_err_t (*read_folder)(nyextm_t*, nyextm_ext_source_t*, const char* path, uint32_t len, nyanystr_t* content);
	/*! Get the size (in bytes) occupied by the extension */
	size_t (*fetch_local_size)(nyextm_t*, nyextm_ext_source_t*);
	/*! Get the local folder (if available) */
	const char* (*get_local_folder)(nyextm_t*, const nyextm_ext_source_t*, uint32_t* length);

	/*! Create a new source (for a specific extension) */
	nyextm_ext_source_t* (*open)(nyextm_t*, const char*, uint32_t len, nyobj_t* params);
	/*! +ref */
	void (*ref)(nyextm_ext_source_t*);
	/*! -ref*/
	void (*unref)(nyextm_ext_source_t*);

	/*! Make it locally available for loading (`git clone` for example) */
	nyext_err_t (*install)(nyextm_t*, nyextm_ext_source_t*, nybool_t force_reinstall);
	/*! Update the latest version (`git pull` for example) */
	nyext_err_t (*update)(nyextm_t*, nyextm_ext_source_t*, const char* branch, uint32_t blen);
}
nyextm_source_cf_t;


/*!
** \brief Register a new source adapter
**
** \code
** nyextm_source_register(extm, "git", &my_src_cf);
** \endcode
*/
NY_EXPORT nybool_t nyextm_source_register(nyextm_t*, const char* id, const nyextm_source_cf_t*);

NY_EXPORT nybool_t nyextm_source_register_ex(nyextm_t*, const char* id, uint32_t idlen, const nyextm_source_cf_t*);

/*! Remove a source handler */
NY_EXPORT nybool_t nyextm_source_deregister(nyextm_t*, const char* id, uint32_t len);
/*@}*/




/*! \name Runners */
/*@{*/
/*! Runner connector types */
enum nyextm_runner_connector_t
{
	/*! Unknown runner type */
	nyrct_unknown,
	/*! Internal support */
	nyrct_internal,
	/*! External process */
	nyrct_process,
};


/*! Information about a particular runner */
typedef struct
{
	/*! Runner definition ID */
	nyeid_t id;
	/*! runner name ID */
	nyanystr_t name;
	/*! runner caption (Human Readable name) */
	nyanystr_t caption;
	/*! runner connector type */
	nyextm_runner_connector_t connector;
	/*! Path or cmd to execute, according the connector type (if any) */
	nyanystr_t path;
	/*! runner version */
	nyversion_t version;
	/*! extension family (ex: "nany", "python", "native", ...) */
	nyanystr_t family;

	/*! The maximum number of slots (extensions) per instance */
	uint32_t max_slots_per_instance;
	/*! The maximum number of instances for this runner */
	uint32_t max_instances;
	/*! Timeout (in ms) before quitting when idle (no extension loaded) */
	uint32_t idle_timeout;
}
nyextm_runner_cf_t;


/*! Information about a particular shard */
typedef struct
{
	/*! Runner instance ID at runtime */
	nyeid_t rid;
	/*! Information about the runner itself */
	const nyextm_runner_cf_t* runner;
	/*! The number of slots currently used in this instance */
	uint32_t slots_used_count;
	/*! Process ID of the runner */
	uint32_t process_id;
	/*! Flag to determine whether this instance uses admin privileges */
	uint32_t has_admin_privileges:1;
}
nyextm_runner_usage_t;




/*! Register a new runner */
NY_EXPORT nyext_err_t nyextm_runner_register(nyextm_t*, const nyextm_runner_cf_t*);
/*! Deregister an runner */
NY_EXPORT nyext_err_t nyextm_runner_deregister(nyextm_t*, nyeid_t);

/*! Try to find an runner according its id */
NY_EXPORT nybool_t nyextm_runner_find(nyextm_t*, nyeid_t*, const char* id);
/*! Try to find an runner according its id (ex) */
NY_EXPORT nybool_t nyextm_runner_find_ex(nyextm_t*, nyeid_t*, const char* id, uint32_t len);


/*! Find an empty slot or create a new one for a given extension family */
NY_EXPORT nyext_err_t nyextm_runner_allocate_slot(nyextm_t*, nyeid_t*, const char* family, uint32_t flen);
/*! Spawn a new instance for a given runner */
NY_EXPORT nyext_err_t nyextm_runner_spawn(nyextm_t*, nyeid_t*);

/*! Stop a runner and kill it after a timeout (!= 0) if necessary */
NY_EXPORT nyext_err_t nyextm_runner_stop(nyextm_t*, const nyeid_t*, uint32_t timeout_ms);
/*! Stop all runners and kill it after a timeout (!= 0) if necessary */
NY_EXPORT nyext_err_t nyextm_runner_stop_all(nyextm_t*, uint32_t timeout_ms);
/*@}*/




/*! \name Watcher, to monitor extension ativity */
/*@{*/
/*! Opaque struct to a watcher */
typedef struct nyextm_watcher_t nyextm_watcher_t;

typedef struct
{
	/*! An extension has been updated (incremental update) */
	void (*on_extension_updated)(nyextm_t*, const nyextm_ext_usage_t*, void* userdata);
	/*! An extension has been added */
	void (*on_extension_registered)(nyextm_t*, const nyextm_ext_info_t*, void* userdata);
	/*! An extension has been removed */
	void (*on_extension_deregistered)(nyextm_t*, const nyextm_ext_info_t*, void* userdata);

	/*! A new runner has been registered */
	void (*on_runner_registered)(nyextm_t*, const nyextm_runner_cf_t*, void* userdata);
	/*! An runner has been deregistered */
	void (*on_runner_deregistered)(nyextm_t*, const nyextm_runner_cf_t*, void* userdata);
	/*! A new shard has been started */
	void (*on_runner_started)(nyextm_t*, const nyextm_runner_usage_t*, void* userdata);
	/*! A shard has been stopped */
	void (*on_runner_stopped)(nyextm_t*, const nyextm_runner_usage_t*, void* userdata);

	/*! A source adapter has been added */
	void  (*on_source_added)(nyextm_t*, const char* id, uint32_t, void* userdata);
	/*! A source adapter has been removed */
	void  (*on_source_removed)(nyextm_t*, const char* id, uint32_t, void* userdata);

	/*! A bulk update has begun */
	void  (*on_bulk_update_begin)(nyextm_t*, void* userdata);
	/*! A bulk update has ended */
	void  (*on_bulk_update_end)(nyextm_t*, void* userdata);

	/*! The watcher has been initialized (an initial complete snapshot has been provided) */
	void  (*on_watcher_ready)(nyextm_t*, void* userdata);
	/*! The watch is about to be destroyed */
	void  (*on_watcher_destroying)(void*);

	/*! Time interval (in ms) between two incremental snapshot updates */
	/* recommended: 2000 for UI applications */
	uint32_t tick_ms;
	/*! Delay (in ms) before the first complete snapshot */
	/* recommended: 100 for UI applications */
	uint32_t delay_ms;
}
nyextm_watcher_cf_t;


/*! Create a new watcher */
nyextm_watcher_t* nyextm_watcher_create(nyextm_t*, const nyextm_watcher_cf_t*, void* userdata);
/*! Increment the internal ref count of a watcher */
NY_EXPORT void nyextm_watcher_ref(nyextm_watcher_t*);
/*! Decrement the internal ref count of a watcher and destroy it if required */
NY_EXPORT void nyextm_watcher_unref(nyextm_watcher_t*);
/*! Remove all callbacks from a watcher */
NY_EXPORT void nyextm_watcher_clear(nyextm_watcher_t*);
/*! Force a refresh */
NY_EXPORT void nyextm_watcher_refresh(nyextm_watcher_t*);
/*@}*/




/*! Information about a single extension */
typedef struct
{
	/*! Unique Extension ID at runtime */
	nyeid_t rid;
	/*! Extenion caption */
	nyanystr_t id;
	/*! Extenion caption */
	nyanystr_t caption;
	/*! Description */
	nyanystr_t description;
	/*! Extension version */
	nyversion_t version;
	/*! extension family (ex: "nany", "python", "native", ...) */
	nyanystr_t family;
	/*! Website URL (for documentation) */
	nyanystr_t website_url;
	/*! web link for bug reporting */
	nyanystr_t website_bugreport;
	/*! Local path where the extension can be found (if any) */
	nyanystr_t localpath;

	/*! Flag to determine whether the extension has been found */
	uint32_t is_installed:1;
	/*! Flag to determine whether the extension can be unloaded/reloaded */
	uint32_t is_unloadable:1;
	/*! Flag to determine whether the extension can be updated */
	uint32_t is_updatable:1;
	/*! Flag to determine whether the extension can be updated when loaded */
	uint32_t is_updatable_when_loaded:1;
	/*! Flag to determine whether the runner has loaded this ext with admin privileges */
	uint32_t is_loaded_with_privileges:1;
	/*! Flag to determine whether the runner is able to report the memory usage for this ext. */
	uint32_t has_memory_usage:1;
	/*! Flag to determine whether the runner is able to report the number of opened files for this ext. */
	uint32_t has_file_count:1;
	/*! Flag to determine whether the runner is able to report the number of threads for this ext. */
	uint32_t has_thread_count:1;
	/*! Flag to determine whether the runner is able to report the number of jobs for this ext. */
	uint32_t has_job_count:1;
	/*! Flag to determine whether the runner can be loaded asynchronously */
	uint32_t can_load_async:1;
	/*! Flag to determine whether the extension requires privileges */
	uint32_t requires_privileges:1;
	/*! Flag to determine whether the extension requires a dedicated runner (not shared with other extensions) */
	uint32_t requires_dedicated_runner:1;

	/*! Total number of capabilities */
	uint32_t capabilities_count;
	/*! All capabilities (strictly ordered) (ex: "git", "engine-nany", "webdav"...)*/
	const char** capabilities;

	/*! source */
	nyextm_source_info_t* source;
	/*! Parameters given to the source for this extension */
	nyobj_t* source_params;

	/*! reserved for internal extension implementation */
	void* reserved[4];
}
nyextm_ext_info_t;


/*! Usage Information about an extension */
typedef struct
{
	/*! Extension ID */
	nyeid_t rid;
	/*! runner instance ID to which the extension is attached to */
	nyeid_t runner_instance_id;
	/*! The extension itself */
	const nyextm_ext_info_t* extension;

	/*! Extension state */
	uint32_t is_loaded:1;
	/*! Flag to determine whether the extension is currently updating */
	uint32_t is_updating:1;

	/*! Timestamp the last time the extension has been loaded */
	int64_t last_loaded;
	/*! Timestamp of the last extension info update */
	/* (can be used to refresh info from the 'extension' or 'runner_instance_id' fields) */
	int64_t last_info_update;
	/*! Memory usage for the extension (in bytes) (if available) */
	uint64_t memory_usage;
	/*! The total number of files opened (if available) */
	uint32_t file_opened_count;
	/*! The total number of threads used by the extension (if available) */
	uint32_t thread_count;
	/*! The total number of jobs currently running or in queue (if available) */
	uint32_t job_count;
}
nyextm_ext_usage_t;






/*! Find all extensions with a given capability */
NY_EXPORT nyeid_t* nyextm_find_from_capabilities(nyextm_t*, const char* name, uint32_t* count);

/*! Find all extensions with a given capability */
NY_EXPORT nyeid_t* nyextm_find_from_capabilities_ex(nyextm_t*, const char* name, uint32_t nlen, uint32_t* count);

/*! Get the list of all capabilities for all extensions (strictly ordered) */
/* \see nystrarray_free() */
NY_EXPORT char** nyextm_get_all_capabilities(nyextm_t*, uint32_t* count);


/*!
** \brief Load an extension
**
** \code
** nyextm_add(extm, "pkg", nyobj_str("my-official-package"));
** nyextm_add(extm, "local", nyobj_str("/home/user/.myapp/extensions/myextension"));
** nyextm_add(extm, "github", nyobj_str("nany-lang/nany"));
** \endcode
*/
NY_EXPORT nyeid_t nyextm_add(nyextm_t*, const char* source, nyobj_t*);

NY_EXPORT nyeid_t nyextm_add_ex(nyextm_t*, const char* source, uint32_t slen,
	nyobj_t*, uint32_t order /*= 0*/, nybool_t autoload);

/*! Unload then remove the extension from the list */
NY_EXPORT nybool_t nyextm_remove(nyextm_t*, const nyeid_t*);

/*! Unload a particular extension */
NY_EXPORT nyext_err_t nyextm_unload(nyextm_t*, const nyeid_t*);

/*! Load a particular extension */
NY_EXPORT nyext_err_t nyextm_load(nyextm_t*, const nyeid_t*);

/*! Reload a particular extension */
NY_EXPORT nyext_err_t nyextm_reload(nyextm_t*, const nyeid_t*);

/*! Update a particular extension */
NY_EXPORT nyext_err_t nyextm_update(nyext_t*, const nyeid_t*, int reload);

/*! Install a particular extension */
NY_EXPORT nyext_err_t nyextm_install(nyextm_t*, const nyeid_t*);

/*! Get if an extension is loaded */
NY_EXPORT nybool_t nyextm_is_loaded(nyextm_t*, const nyeid_t*);


/*! Load all registered extensions */
NY_EXPORT nyext_err_t nyextm_install_all(nyextm_t*);
/*! Load all non running extensions */
NY_EXPORT nyext_err_t nyextm_load_all(nyextm_t*);
/*! Unload all running extensions */
NY_EXPORT nyext_err_t nyextm_unload_all(nyextm_t*);
/*! Reload all extensions */
NY_EXPORT nyext_err_t nyextm_reload_all(nyextm_t*);

/*! Update all extensions if possible */
NY_EXPORT nyext_err_t nyextm_update_all(nyextm_t*, nybool_t can_unload_first, nybool_t auto_reload);




NY_EXPORT nyext_err_t nyextm_get_extension_info(nyext_t*, const nyeid_t*, nyextm_ext_info_t* out);

/*! Get the list of all */
NY_EXPORT nyeid_t* nyext_list_all_extensions(nyext_t*, uint32_t* count);





/* ------------- REPOSITORY, LATER WIP ---------- */

enum nyextm_depository_action_t
{
	nyefut_added,
	nyefut_removed,
	nyefut_update,
};

typedef struct
{
	nyanystr_t labelid;
	nyextm_depository_action_t action;
	struct { uint8_t r, g, b; } color;
	uint32_t badge_count;
	nybool_t is_category;
}
nyextm_depository_bulk_label_sync_t;

typedef struct
{
	/*! URL of the repository */
	nyanystr_t url;
	/*! Caption */
	nyanystr_t caption;
	/*! Public key */
	nyanystr_t public_key;
	/*! Webpage */
	nyanystr_t website_url;
	/*! Total number of Extension for this repository */
	uint32_t extension_count;
}
nyextm_depository_source_info_t;


typedef struct
{
	void (*on_labels_bulk_update)(nyextm_depository_t*, const nyextm_depository_bulk_label_sync_t*, uint32_t count);

	void (*on_source_added)(nyextm_depository_t*, const nyextm_depository_source_info_t*);
	void (*on_source_updated)(nyextm_depository_t*, const nyextm_depository_source_info_t*);
	void (*on_source_removed)(nyextm_depository_t*, const nyextm_depository_source_info_t*);
}
nyextm_depository_cf_t;


nyextm_depository_t* nyextm_depository_create();

nyext_err_t nyextm_depository_source_add(nyextm_depository_t*, const char* url, uint32_t len,
	const char* publickey, uint32_t pklen);

nybool_t nyextm_depository_sync(nyextm_depository_t*);


#ifdef __cplusplus
}
#endif

#endif /* LIBNYEXTM_H */
