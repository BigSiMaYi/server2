// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 GAME_FISHLORD_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// GAME_FISHLORD_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。



#ifdef GAME_LANDLORD_EXPORTS
#define GAME_LANDLORD_API __declspec(dllexport)
#else
#define GAME_LANDLORD_API __declspec(dllimport)
#endif


#if defined(_MSC_VER)

#ifdef GAME_LANDLORD_EXPORTS
#define GAME_LANDLORD_API __declspec(dllexport)
#else
#define GAME_LANDLORD_API __declspec(dllimport)
#endif

#elif defined(LINUX)
#ifdef GAME_LANDLORD_EXPORTS
#define GAME_LANDLORD_API __attribute__((visibility("default")))
#else
#define GAME_LANDLORD_API
#endif

#else
//  do nothing and hope for the best?
#define EXPORT
#define IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

extern "C" {

	GAME_LANDLORD_API void* get_game_engine();

	GAME_LANDLORD_API void* get_packet_mgr();
}
