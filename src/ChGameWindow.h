#pragma once
#include "./ChScriptedWindow.h"

class ChGameWindow : public ChScriptedWindow {
private:
  // Window-Frontend messaging:

  AccountManager &accounts;

  // Mouse hooking:

  static LRESULT CALLBACK mouseMessage(int code, WPARAM wParam, LPARAM lParam);
  LRESULT onInput(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &fHandled);
  void hookMouse();
  void unhookMouse();
  RAWINPUTDEVICE rawInput;
  HHOOK mouseHook = 0;
  bool mouseHooked = false;
  std::time_t lastPointerPoll;

  long long mouseHz = 244;
  long long mouseInterval = 1000 / mouseHz;
  long long then = now();
  Vector2 movebuffer;

  std::string gameCSS1;
  std::string gameCSS2;
  std::wstring gameJS;

  std::mutex seeking;
  void seekGame();

protected:
  // game CSS
  virtual rapidjson::Value getUserStyles(
      rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator)
      override;

  // account messages
  virtual void handleMessage(JSMessage msg) override;

  // game.js
  virtual void injectJS() override;

  // mouse hooks
  virtual void registerEvents() override;

  BEGIN_MSG_MAP(ChGameWindow)
  CHAIN_MSG_MAP(ChWindow)
  MESSAGE_HANDLER(WM_INPUT, onInput)
  END_MSG_MAP()
public:
  // init resources
  ChGameWindow(ClientFolder &folder, AccountManager &accounts,
               ChWindows &windows, Vector2 scale, std::wstring title);
  // release hook
  ~ChGameWindow();
  // pointer poll
  virtual void dispatch() override;
};
