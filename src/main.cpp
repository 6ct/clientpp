
#include "./main.h"
#include "../utils/JsonUtil.h"
#include "../utils/StringUtil.h"
#include "./AccountManager.h"
#include "./ChWindow.h"
#include "./ClientFolder.h"
#include "./Log.h"
#include "./Updater.h"
#include "./WebView2Installer.h"
#include "./resource.h"
#include <ShellScalingApi.h>
#include <WinUser.h>
#include <discord_register.h>
#include <discord_rpc.h>
#include <functional>
#include <shellapi.h>
#include <sstream>
#include <thread>

constexpr const char *version = CLIENT_VERSION_STRING;
constexpr const wchar_t *title = L"Chief Client";
constexpr const char *discordRPC = "899137303182716968";

namespace {
HICON mainIcon;
HACCEL hAccTable;
AccountManager *accounts;
ClientFolder *folder;
ChWindows *windows;

int messages() {
  while (true) {
    MSG msg;
    BOOL ret = GetMessage(&msg, 0, 0, 0);
    if (!ret)
      break;
    windows->dispatch();
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (windows->shouldQuit())
      break;
  }

  return EXIT_SUCCESS;
}

bool testRuntimes() {
  if (WebView2Installer::isInstalled())
    return true;

  WebView2Installer installer;

  if (MessageBox(NULL,
                 L"You are missing runtimes. Install the WebView2 Runtime?",
                 title, MB_YESNO) == IDYES) {
    WebView2Installer::Error error;
    MessageBox(NULL, L"Relaunch the client after installation is complete.",
               title, MB_OK);
    if (!installer.install(error))
      switch (error) {
      case WebView2Installer::Error::CantOpenProcess:
        clog::error << "CantOpenProcess during WebView2 installation"
                    << clog::endl;
        MessageBox(NULL,
                   (L"Unable to open the installer. You will need to run the "
                    L"exe manually. Go to: " +
                    installer.binPath)
                       .c_str(),
                   title, MB_OK);
        break;
      case WebView2Installer::Error::NoBytesDownloaded:
        clog::error << "NoBytesDownloaded during WebView2 installation"
                    << clog::endl;
        MessageBox(NULL,
                   L"Unable to download MicrosoftEdgeWebview2Setup. Relaunch "
                   L"the client then try again.",
                   title, MB_OK);
        break;
      default:
        clog::error << "Unknown error " << (int)error
                    << " during WebView2 installation" << clog::endl;
        MessageBox(NULL, L"An unknown error occurred.", title, MB_OK);
        break;
      }
  } else
    MessageBox(NULL, L"Cannot continue without runtimes. Client will now exit.",
               title, MB_OK);

  return false;
}

void update() {
  UpdaterServing serving;

  if (!updatesAvailable(version, "https://6ct.github.io/serve/updates.json",
                        serving) ||
      MessageBox(NULL, L"A new client update is available. Download?", title,
                 MB_YESNO) == IDNO)
    return;

  ShellExecute(NULL, L"open", ST::wstring(serving.url).c_str(), L"", L"",
               SW_SHOW);
  exit(EXIT_SUCCESS);
}
} // namespace

HICON getMainIcon() { return mainIcon; }

int APIENTRY WinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance,
                     _In_ LPSTR cmdline, _In_ int nCmdShow) {
  DiscordEventHandlers presenceEvents;
  presenceEvents.disconnected = nullptr;
  presenceEvents.errored = nullptr;
  presenceEvents.joinGame = nullptr;
  presenceEvents.joinRequest = nullptr;
  presenceEvents.ready = nullptr;
  presenceEvents.spectateGame = nullptr;

  memset(&presenceEvents, 0, sizeof(presenceEvents));
  Discord_Initialize(discordRPC, &presenceEvents, 1, nullptr);

  mainIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MAINICON));

  hAccTable = LoadAccelerators(hInstance, L"WebView2Accel");

  folder = new ClientFolder();

  if (!folder->create(L"GC++")) {
    clog::debug << "Error creating folder" << clog::endl;
    MessageBox(NULL, L"Error creating folder. See Error.log.", title, MB_OK);
    return EXIT_FAILURE;
  }

  folder->loadConfig();

  accounts = new AccountManager(*folder);
  accounts->load();

  windows = new ChWindows(*folder, *accounts);

  if (!testRuntimes())
    return EXIT_FAILURE;

  if (HMODULE shcore = LoadLibrary(L"api-ms-win-shcore-scaling-l1-1-1.dll")) {
    using dec = decltype(::SetProcessDpiAwareness);
    if (std::function SetProcessDpiAwareness =
            (dec *)GetProcAddress(shcore, "SetProcessDpiAwareness")) {
      HRESULT sda = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
      if (!SUCCEEDED(sda))
        clog::error << "SetProcessDpiAwareness returned 0x" << std::hex
                    << HRESULT_CODE(sda) << clog::endl;
    } else
      clog::error << "Unable to get address of SetProcessDpiAwareness"
                  << clog::endl;
  }

  // checking updates causes delay
  if (folder->config["client"]["auto_update"].GetBool())
    new std::thread(update);

  switch (windows->navigate(UriW(L"https://krunker.io/"))) {

  case ChWindow::Status::Ok:
    return messages();
  case ChWindow::Status::MissingRuntime:
    clog::error
        << "WebView2 installation check failed. Unable to create game window."
        << clog::endl;
    return EXIT_FAILURE;
  case ChWindow::Status::UserDataExists:
    clog::error << "Unable to create user data folder." << clog::endl;
    MessageBox(NULL,
               L"Unable to create the user data folder. Delete the GC++ "
               L"folder in your documents then relaunch the client.",
               title, MB_OK | MB_ICONERROR);
    return EXIT_FAILURE;
  case ChWindow::Status::RuntimeFatal:
    clog::error << "Fatal Edge runtime error occured." << clog::endl;
    MessageBox(
        NULL,
        L"An unknown Edge runtime error occured. Try relaunching the client.",
        title, MB_OK | MB_ICONERROR);
    return EXIT_FAILURE;
  case ChWindow::Status::UnknownError:
  default: {
    std::wstringstream sstream;
    sstream << std::hex << ChWindow::getLastHError();

    MessageBox(
        NULL,
        (std::wstring(
             L"An unknown error ocurred during game creation. Create a issue "
             L"on GitHub ( https://github.com/6ct/clientpp/issues ) and "
             L"provide the following error details:\n") +
         L"Error code: 0x" + sstream.str())
            .c_str(),
        title, MB_OK | MB_ICONERROR);
  }
    return EXIT_FAILURE;
    ;
  }
}