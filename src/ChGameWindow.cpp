#include "./ChGameWindow.h"
#include "../utils/JsonUtil.h"
#include "../utils/StringUtil.h"
#include "./JSMessage.h"
#include "./LoadRes.h"
#include "./LobbySeeker.h"
#include "./Log.h"
#include "./resource.h"

ChGameWindow::ChGameWindow(ClientFolder &_folder, AccountManager &_accounts,
                           ChWindows &_windows, Vector2 _scale,
                           std::wstring _title)
    : ChScriptedWindow(_folder, _windows, _scale, _title), accounts(_accounts),
      lastPointerPoll(now()) {
  if (!loadResource(CSS_GAME_1, gameCSS1))
    clog::error << "Failure loading game.css" << clog::endl;

  if (!loadResource(CSS_GAME_2, gameCSS2))
    clog::error << "Failure loading AccountManager.css" << clog::endl;

  // convert to wstring immediately
  std::string mGameJS;

  if (loadResource(JS_GAME, mGameJS))
    gameJS = ST::wstring(mGameJS);
  else
    clog::error << "Failure loading game.js" << clog::endl;

  rawInput.usUsagePage = 0x01;
  rawInput.usUsage = 0x02;
}

void ChGameWindow::injectJS() { injectRuntimeScript(gameJS); }

void ChGameWindow::registerEvents() {
  ChScriptedWindow::registerEvents();

  EventRegistrationToken token;

  webview->add_NavigationStarting(
      Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2NavigationStartingEventArgs *args) -> HRESULT {
            if (mouseHooked)
              unhookMouse();
            return S_OK;
          })
          .Get(),
      &token);
}

JSMessage msgFct(unsigned short event, std::vector<double> nums) {
  JSMessage msg(event);
  for (double num : nums)
    msg.args.PushBack(rapidjson::Value(num), msg.allocator);
  return msg;
}

void ChGameWindow::dispatch() {
  ChScriptedWindow::dispatch();

  if (!open)
    return;

  bool active = GetActiveWindow() == m_hWnd;

  if (!active && mouseHooked)
    unhookMouse();

  time_t lastPoll = now() - lastPointerPoll;

  if (lastPoll > 1500 && mouseHooked) {
    clog::error << "Pointer lock timeout: " << lastPoll << "ms" << clog::endl;
    unhookMouse();
  }

  if (mouseHooked) {
    long long nw = now();
    long long delta = nw - then;

    if (delta > mouseInterval) {
      then = nw - (delta % mouseInterval);
      sendMessage(msgFct(IM::mousemove, {movebuffer.x, movebuffer.y}));
      movebuffer.clear();
    }
  }
}

LRESULT ChGameWindow::onInput(UINT uMsg, WPARAM wParam, LPARAM lParam,
                              BOOL &fHandled) {
  unsigned size = sizeof(RAWINPUT);
  static RAWINPUT raw[sizeof(RAWINPUT)];
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size,
                  sizeof(RAWINPUTHEADER));

  if (raw->header.dwType == RIM_TYPEMOUSE) {
    RAWMOUSE mouse = raw->data.mouse;
    USHORT flags = mouse.usButtonFlags;

    if (flags & RI_MOUSE_WHEEL)
      sendMessage(msgFct(
          IM::mousewheel,
          {double((*(short *)&mouse.usButtonData) / WHEEL_DELTA) * -100}));
    if (flags & RI_MOUSE_BUTTON_1_DOWN)
      sendMessage(msgFct(IM::mousedown, {0}));
    if (flags & RI_MOUSE_BUTTON_1_UP)
      sendMessage(msgFct(IM::mouseup, {0}));
    if (flags & RI_MOUSE_BUTTON_2_DOWN)
      sendMessage(msgFct(IM::mousedown, {2}));
    if (flags & RI_MOUSE_BUTTON_2_UP)
      sendMessage(msgFct(IM::mouseup, {2}));
    if (flags & RI_MOUSE_BUTTON_3_DOWN)
      sendMessage(msgFct(IM::mousedown, {1}));
    if (flags & RI_MOUSE_BUTTON_3_UP)
      sendMessage(msgFct(IM::mouseup, {1}));
    if (flags & RI_MOUSE_BUTTON_4_DOWN)
      sendMessage(msgFct(IM::mousedown, {3}));
    if (flags & RI_MOUSE_BUTTON_4_UP)
      sendMessage(msgFct(IM::mouseup, {3}));
    if (flags & RI_MOUSE_BUTTON_5_DOWN)
      sendMessage(msgFct(IM::mousedown, {4}));
    if (flags & RI_MOUSE_BUTTON_5_UP)
      sendMessage(msgFct(IM::mouseup, {4}));
    if (mouse.lLastX || mouse.lLastY)
      movebuffer += Vector2(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
  }

  return S_OK;
}

LRESULT CALLBACK ChGameWindow::mouseMessage(int code, WPARAM wParam,
                                            LPARAM lParam) {
  return 1;
}

void ChGameWindow::hookMouse() {
  clog::debug << "Hooking mouse" << clog::endl;

  INPUT input = {};
  input.type = INPUT_MOUSE;
  input.mi.dx = 0;
  input.mi.time = 0;
  input.mi.dwExtraInfo = NULL;
  input.mi.dwFlags = (MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP |
                      MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_MIDDLEUP);
  SendInput(1, &input, sizeof(INPUT));

  rawInput.dwFlags = RIDEV_INPUTSINK;
  rawInput.hwndTarget = m_hWnd;
  RegisterRawInputDevices(&rawInput, 1, sizeof(rawInput));

  mouseHook =
      SetWindowsHookEx(WH_MOUSE_LL, *mouseMessage, getHinstance(), NULL);
  mouseHooked = true;
}

void ChGameWindow::unhookMouse() {
  clog::debug << "Unhooked mouse" << clog::endl;

  rawInput.dwFlags = RIDEV_REMOVE;
  rawInput.hwndTarget = NULL;
  RegisterRawInputDevices(&rawInput, 1, sizeof(rawInput));

  UnhookWindowsHookEx(mouseHook);
  mouseHooked = false;
}

ChGameWindow::~ChGameWindow() {
  if (mouseHooked)
    unhookMouse();
}

void ChGameWindow::seekGame() {
  if (!folder.config["game"]["seek"]["F4"].GetBool())
    return;

  std::unique_lock seekLock(seeking);

  if (!folder.config["game"]["seek"]["custom_logic"].GetBool()) {
    webview->Stop();
    webview->Navigate(L"https://krunker.io/");
  }

  postMessage(
      JSMessage(IM::get_ping_region),
      [this](const rapidjson::Value &value) -> void {
        std::string region(value.GetString(), value.GetStringLength());

        new std::thread([this, region] {
          std::string configMode =
              JT::string(folder.config["game"]["seek"]["mode"]);

          size_t mode = -1;

          for (size_t mi = 0; mi < seekModes.size(); mi++)
            if (seekModes[mi] == configMode)
              mode = mi;

          std::string gameURL = seekLobby(
              region, mode, folder.config["game"]["seek"]["customs"].GetBool(),
              ST::lowercase(JT::string(folder.config["game"]["seek"]["map"])));

          pendingNavigations.push_back(ST::wstring(gameURL).c_str());
        });
      },
      nullptr);
}

void ChGameWindow::handleMessage(JSMessage msg) {
  ChScriptedWindow::handleMessage(msg);

  switch (msg.event) {
  case IM::pointer:
    lastPointerPoll = now();
    if (msg.args[0].GetBool() && !mouseHooked)
      hookMouse();
    else if (!msg.args[0].GetBool() && mouseHooked)
      unhookMouse();

    break;
  case IM::seek_game:
    seekGame();

    break;
  case IM::account_password: {
    JSMessage res(msg.args[0].GetInt());
    std::string dec;
    std::string accountName = JT::string(msg.args[1]);

    if (!accounts.data.contains(accountName)) {
      res.args.PushBack(rapidjson::Value(rapidjson::kNullType), res.allocator);
      res.args.PushBack(
          rapidjson::Value("Account doesn't exist", res.allocator),
          res.allocator);
    } else if (!accounts.decrypt(accounts.data[accountName].password, dec)) {
      res.args.PushBack(rapidjson::Value(rapidjson::kNullType), res.allocator);
      res.args.PushBack(rapidjson::Value("Unknown", res.allocator),
                        res.allocator);
    } else {
      res.args.PushBack(rapidjson::Value(dec.data(), dec.size(), res.allocator),
                        res.allocator);
    }

    if (!sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  case IM::account_remove: {

    accounts.data.erase(JT::string(msg.args[0]));
    accounts.save();

    JSMessage res(IM::account_regen);
    res.args.PushBack(accounts.dump(res.allocator), res.allocator);
    if (!sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  case IM::account_set: {

    std::string name = JT::string(msg.args[0]);
    Account &account = accounts.data[name];
    account.color = JT::string(msg.args[1]);
    account.order = msg.args[2].GetInt();
    accounts.save();

    JSMessage res(IM::account_regen);
    res.args.PushBack(accounts.dump(res.allocator), res.allocator);
    if (!sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  // and creation
  case IM::account_set_password: {

    std::string enc;
    if (accounts.encrypt(JT::string(msg.args[1]), enc)) {
      Account account;
      account.color = JT::string(msg.args[2]);
      account.order = msg.args[3].GetInt();
      account.username = JT::string(msg.args[0]);
      account.password = enc;
      accounts.data[account.username] = account;
      accounts.save();

      JSMessage res(IM::account_regen);
      res.args.PushBack(accounts.dump(res.allocator), res.allocator);
      if (!sendMessage(res))
        clog::error << "Unable to send " << res.dump() << clog::endl;
    } else
      clog::error << "Failure encrypting password" << clog::endl;
  } break;
  case IM::account_list: {

    JSMessage res(msg.args[0].GetInt());
    res.args.PushBack(accounts.dump(res.allocator), res.allocator);
    if (!sendMessage(res))
      clog::error << "Unable to send " << res.dump() << clog::endl;
  } break;
  }
}

rapidjson::Value ChGameWindow::getUserStyles(
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator) {
  rapidjson::Value value = ChScriptedWindow::getUserStyles(allocator);

  {
    rapidjson::Value row(rapidjson::kArrayType);
    row.PushBack("game.css", allocator);
    row.PushBack(rapidjson::Value(gameCSS1.data(), gameCSS1.size(), allocator),
                 allocator);
    value.PushBack(row, allocator);
  }

  {
    rapidjson::Value row(rapidjson::kArrayType);
    row.PushBack("AccountManager.css", allocator);
    row.PushBack(rapidjson::Value(gameCSS2.data(), gameCSS2.size(), allocator),
                 allocator);
    value.PushBack(row, allocator);
  }

  return value;
}
