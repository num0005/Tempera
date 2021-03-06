#pragma once

#include <precompile.h>
#include "../extended/addlog.h"
#include "../function_rewrite.h"
#include <engine_interface.h>
#include "memory_interface.h"
#include "players_interface.h"
#include "gamestate_interface.h"

extern "C" {
#include "../../include/lua_headers/lua.h"
#include "../../include/lua_headers/lauxlib.h"
#include "../../include/lua_headers/lualib.h"
}

enum LuaCallbackId {
	invalid                   = -1,
	//not working, but still available to hook into.
	on_load                   = 0,
	on_map_load               = 1,
	before_scenario_tags_load = 2,
	after_scenario_tags_load  = 3,
	// available, should be working
	before_game_tick          = 4,
	after_game_tick           = 5,
	post_initialize           = 6,
	post_dll_init             = 7,
	//end available, working hooks.
	max_callback_id
};

template <typename T = int>
static bool isValidCbId(T id) {
	return (id < LuaCallbackId::max_callback_id && id >= id);
}

//hacks upon hacks
static void registerLuaCallback(const std::string &cb_name, LuaCallbackId cb_type);

static int l_print(lua_State *L) {
	const bool tocmd = lua_toboolean(L, 1);
	const char *str  = lua_tostring(L, 2);
	Print(tocmd, str);
	return 0;
}

static int l_registerLuaCallback(lua_State *L) {
	const char *str = lua_tostring(L, 1);
	const int  id   = lua_tointeger(L, 2);
	if (isValidCbId(id)) {
		registerLuaCallback(str, (LuaCallbackId) id);
	}
	return 0;
}

//This code was (for the most part) copy-pasted from
//The tutorial series here: https://eliasdaler.wordpress.com/2013/10/20/lua_and_cpp_pt2/
class LuaScriptManager {
private:
	lua_State                             *L;
	int                                   level;
	std::vector<std::vector<std::string>> callbacks;
	std::string                           filename;
	bool                                  loaded = false;

public:

	bool IsLoaded() {
		return this->loaded;
	}

	/**
 * Tells our class to call this function when the associated CB id is triggered.
 * @param cb_name Name of Lua func to call.
 * @param cb_type On which event this func is called.
 */
	void registerLuaCallback(const std::string &cb_name, LuaCallbackId cb_type) {
		if (isValidCbId(cb_type)) {
			this->callbacks.at(cb_type).push_back(cb_name);
		}
	}

	void registerGlobalLuaFunction(const std::string &funcName, lua_CFunction funcAddr) {
		lua_pushcclosure(L, funcAddr, 0);
		lua_setglobal(L, funcName.c_str());
	}

	void beginLua() {
		Print(true, "Trying to run the script file: %s\n", this->filename.c_str());

		if (lua_pcall(L, 0, 0, 0)) {
			Print(true, "Error: script failed to run! (%s)\n", this->filename.c_str());
		}
	}

	explicit LuaScriptManager(const std::string &filename = "init.txt") {
		this->callbacks = std::vector<std::vector<std::string>>();
		this->callbacks.resize(LuaCallbackId::max_callback_id);

		Print(true, "Adding new callbacks list\n");

		for (int i = 0; i < LuaCallbackId::max_callback_id; i++) {
			this->callbacks.push_back(std::vector<std::string>());
		}

		Print(true, "Trying To load: %s\n", filename.c_str());

		L = luaL_newstate();
		luaL_openlibs(L);
		if (luaL_loadfile(L, filename.c_str())) {
			Print(true, "Error: script failed to load: (%s)\n\tDoes it exist?\n", filename.c_str());
			this->loaded = false;
			L = 0;
			return;
		}

		//Takes 1 Param - location to read value from
		registerGlobalLuaFunction("ReadByte", l_readByte);
		registerGlobalLuaFunction("Read32f", l_readFloat);
		registerGlobalLuaFunction("Read32i", l_readInt);
		registerGlobalLuaFunction("Read16i", l_readShort);

		//Takes 2 Params - location to write value, value to write to location.
		registerGlobalLuaFunction("WriteByte", l_writeByte);
		registerGlobalLuaFunction("Write32f", l_writeFloat);
		registerGlobalLuaFunction("Write32i", l_writeInt);
		registerGlobalLuaFunction("Write16i", l_writeShort);

		//2 Params - boolean - tells whether or not to write to console. always writes to dbg log.
		//2nd param - const char * string to print to the log.
		registerGlobalLuaFunction("DebugPrint", l_print);

		//2 Params
		registerGlobalLuaFunction("RegisterCallBack", l_registerLuaCallback);
		registerGlobalLuaFunction("GetMaxLocalPlayers", l_GetMaxLocalPlayers);

		registerGlobalLuaFunction("AreWeInMainMenu", l_InMainMenu);
		registerGlobalLuaFunction("GetEngineContext", l_GetEngineContext);

		this->filename = filename;
		this->loaded   = true;
	}

	~LuaScriptManager() {
		if (L) {
			lua_close(L);
		}
	}

	void printError(const std::string &variableName, const std::string &reason) {
		Print(true, "Error: can't get [%s]. Reason: %s\n", variableName, reason);
	}

	// Generic get
	template <typename T>
	T lua_get(const std::string &variableName) {
		return 0;
	}

	// Generic default get
	template <typename T>
	T lua_getdefault() {
		return 0;
	}

	// Generic default get
	template <typename T>
	T lua_getdefault(const std::string &variableName) {
		return 0;
	}

	template <>
	inline std::string lua_getdefault() {
		return "null";
	}

	template <>
	inline bool lua_get(const std::string &variableName) {
		return (bool) lua_toboolean(L, -1);
	}

	template <>
	inline float lua_get(const std::string &variableName) {
		if (!lua_isnumber(L, -1)) {
			printError(variableName, "Not a number");
		}
		return (float) lua_tonumber(L, -1);
	}

	template <>
	inline int lua_get(const std::string &variableName) {
		if (!lua_isnumber(L, -1)) {
			printError(variableName, "Not a number");
		}
		return (int) lua_tonumber(L, -1);
	}

	template <>
	inline std::string lua_get(const std::string &variableName) {
		std::string s = "null";
		if (lua_isstring(L, -1)) {
			s = std::string(lua_tostring(L, -1));
		} else {
			printError(variableName, "Not a string");
		}
		return s;
	}

	template <typename T>
	T get(const std::string &variableName) {
		if (!L) {
			printError(variableName, "Script is not loaded");
			return lua_getdefault<T>();
		}

		// variable succesfully on top of stack
		T result;
		if (lua_gettostack(variableName)) {
			result = lua_get<T>(variableName);
		} else {
			result = lua_getdefault<T>();
		}

		lua_pop(L, level + 1); // pop all existing elements from stack
		return result;
	}

	bool lua_gettostack(const std::string &variableName) {
		level                 = 0;
		std::string       var = "";
		for (unsigned int i   = 0; i < variableName.size(); i++) {
			if (variableName.at(i) == '.') {
				if (level == 0) {
					lua_getglobal(L, var.c_str());
				} else {
					lua_getfield(L, -1, var.c_str());
				}

				if (lua_isnil(L, -1)) {
					printError(variableName, var + " is not defined");
					return false;
				} else {
					var = "";
					level++;
				}
			} else {
				var += variableName.at(i);
			}
		}
		if (level == 0) {
			lua_getglobal(L, var.c_str());
		} else {
			lua_getfield(L, -1, var.c_str());
		}
		if (lua_isnil(L, -1)) {
			printError(variableName, var + " is not defined");
			return false;
		}

		return true;
	}

	void clean() {
		int n = lua_gettop(L);
		lua_pop(L, n);
	}

	std::vector<int> getIntVector(const std::string &name) {
		std::vector<int> v;
		lua_getglobal(L, name.c_str());
		if (lua_isnil(L, -1)) {
			return std::vector<int>();
		}
		lua_pushnil(L);
		while (lua_next(L, -2)) {
			v.push_back((int) lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
		clean();
		return v;
	}

	void call_void_lua_func(const std::string_view &funcName) {
		if (this->IsLoaded()) {
			lua_getglobal(L, std::string(funcName).c_str());
			lua_pcall(L, 0, 0, 0);
		}
	}

	// void call_void_lua_func(const std::string_view &funcName, float arg) {
	// 	lua_getglobal(L, std::string(funcName).c_str());
	// 	lua_pcall(L, 0, 0, 0);
	// }

	template <LuaCallbackId eventType>
	void call_lua_event_by_type() {
		if (!this || this->callbacks.empty() || !this->IsLoaded()) {
			return;
		}

		std::vector<std::string> cb_list = this->callbacks.at(eventType);

		Print(true, "Begin calling lua event by type: %d", eventType);

		if (cb_list.empty()) {
			return;
		}

		for (std::string_view elem : cb_list) {
			call_void_lua_func(elem);
		}
	}

	// template<LuaCallbackId eventType, typename T>
	// void call_lua_event_by_type(T arg) {
	// 	if (!this || this->callbacks.empty()) {
	// 		return;
	// 	}
	//
	// 	std::vector<std::string> cb_list = this->callbacks.at(eventType);
	//
	// 	Print(true, "Begin calling lua event by type");
	//
	// 	if (cb_list.empty()) {
	// 		return;
	// 	}
	//
	// 	for (std::string_view elem : cb_list) {
	// 		call_void_lua_func(elem);
	// 	}
	// }
};

static LuaScriptManager *LuaState;

/**
 * Called before VirtualProtect is run.
 */
inline static void InitializeLua() {
	Print(true, "Attempting to initialize luascript manager\n");
	LuaState = new LuaScriptManager(CurrentEngine.LUA_FILENAME);

	if (LuaState->IsLoaded()) {
		LuaState->beginLua();
	} else {
		Print(true, "Lua Failed to initialize!\n");
	}
}

inline static void registerLuaCallback(const std::string &cb_name, LuaCallbackId cb_type) {
	LuaState->registerLuaCallback(cb_name, cb_type);
}

void game_tick(int current_frame_tick);
void parse_for_connect_invert();
void post_dll_load();
