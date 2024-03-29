#pragma once
// This file was automatically generated by ipc-messages.js. Do not modify this
// file manually!

namespace LogType {
constexpr const unsigned short info = 0;
constexpr const unsigned short error = 1;
constexpr const unsigned short warn = 2;
constexpr const unsigned short debug = 3;
}; // namespace LogType

namespace IM {
constexpr const unsigned short rpc_update = 0;
constexpr const unsigned short rpc_clear = 1;
constexpr const unsigned short saveConfig = 2;
constexpr const unsigned short shell_open = 3;
constexpr const unsigned short fullscreen = 4;
constexpr const unsigned short update_meta = 5;
constexpr const unsigned short revert_meta = 6;
constexpr const unsigned short reloadConfig = 7;
constexpr const unsigned short browse_file = 8;
constexpr const unsigned short mousedown = 9;
constexpr const unsigned short mouseup = 10;
constexpr const unsigned short mousemove = 11;
constexpr const unsigned short mousewheel = 12;
constexpr const unsigned short pointer = 13;
constexpr const unsigned short open_devtools = 14;
constexpr const unsigned short log = 15;
constexpr const unsigned short relaunch_webview = 16;
constexpr const unsigned short close_window = 17;
constexpr const unsigned short reload_window = 18;
constexpr const unsigned short seek_game = 19;
constexpr const unsigned short account_list = 20;
constexpr const unsigned short account_password = 21;
constexpr const unsigned short account_set = 22;
constexpr const unsigned short account_remove = 23;
constexpr const unsigned short account_set_password = 24;
constexpr const unsigned short account_regen = 25;
constexpr const unsigned short update_menu = 26;
constexpr const unsigned short get_ping_region = 27;
}; // namespace IM
