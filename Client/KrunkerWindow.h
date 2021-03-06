#pragma once
#include <mutex>
#include <string>
#include <functional>
#include <atlbase.h>
#include <atlwin.h>
#include <atlenc.h>
#include <json.hpp>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>
#include "./JSMessage.h"
#include "../Utils/IOUtil.h"
#include "./ClientFolder.h"
#include "./IPCMessages.h"
#include "./Points.h"

class KrunkerWindow : public CWindowImpl<KrunkerWindow> {
public:
	enum class Status {
		Ok,
		UserDataExists,
		FailCreateUserData,
		RuntimeFatal,
		MissingRuntime,
		UnknownError,
		AlreadyOpen,
		NotImplemented,
	};
	enum class Type {
		Game,
		Social,
		Editor,
		Documents,
		Scripting,
	};
	static long long now();
private:
	bool seeking = false;
	std::mutex mtx;
	std::function<bool(JSMessage)> on_unknown_message;
	std::function<void()> on_webview2_startup;
	std::function<void()> on_destroy_callback;
	std::vector<std::wstring> additional_command_line;
	std::vector<std::wstring> additional_block_hosts;
	std::vector<std::wstring> pending_navigations;
	std::vector<JSMessage> pending_messages;
	std::wstring og_title;
	RAWINPUTDEVICE raw_input;
	HHOOK mouse_hook = 0;
	bool mouse_hooked = false;
	std::time_t last_pointer_poll;
	nlohmann::json runtime_data();
	void hook_mouse();
	void unhook_mouse();
	void register_events();
	void handle_message(JSMessage msg);
	static LRESULT CALLBACK mouse_message(int code, WPARAM wParam, LPARAM lParam);
	Status call_create_webview(std::function<void()> callback = nullptr);
	std::wstring cmdline();
	std::string status_name(COREWEBVIEW2_WEB_ERROR_STATUS status);
	bool send_resource(ICoreWebView2WebResourceRequestedEventArgs* args, int resource, std::wstring mime);
	void load_userscripts(nlohmann::json* data = nullptr);
	LRESULT on_input(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
	LRESULT on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
	LRESULT on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
public:
	Type type;
	wil::com_ptr<ICoreWebView2Controller> control;
	wil::com_ptr<ICoreWebView2> webview;
	wil::com_ptr<ICoreWebView2Environment> env;
	std::wstring title;
	bool open = false;
	bool fullscreen = false;
	HRESULT last_herror = 0;
	Vector2 scale;
	RECT saved_size;
	DWORD saved_style = 0;
	DWORD saved_ex_style = 0;
	COREWEBVIEW2_COLOR ColorRef(COLORREF color);
	HINSTANCE get_hinstance();
	bool create_window(HINSTANCE inst, int cmdshow);
	bool enter_fullscreen();
	bool exit_fullscreen();
	bool resize_wv();
	bool monitor_data(RECT& rect);
	bool monitor_data(Vector2& pos, Vector2& size);
	Status create_webview(std::wstring cmdline, std::wstring directory, std::function<void()> callback);
	bool can_fullscreen = false;
	COLORREF background = RGB(28, 28, 28);
	ClientFolder& folder;
	Status create(HINSTANCE inst, int cmdshow, std::function<void()> callback = nullptr);
	Status get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback = nullptr);
	void on_dispatch();
	KrunkerWindow(ClientFolder& folder, Type type, Vector2 scale, std::wstring title, std::function<void()> webview2_startup = nullptr, std::function<bool(JSMessage)> unknown_message = nullptr, std::function<void()> on_destroy = nullptr);
	~KrunkerWindow();
	BEGIN_MSG_MAP(KrunkerWindow)
		MESSAGE_HANDLER(WM_DESTROY, on_destroy)
		MESSAGE_HANDLER(WM_SIZE, on_resize)
		MESSAGE_HANDLER(WM_INPUT, on_input)
	END_MSG_MAP()
};