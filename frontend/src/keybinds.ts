import "./menu/createUI";
import "./resources";
import { createKeybind } from "./Keybind";
import ipc, { IM } from "./ipc";
import getConfig, { setConfig } from "./menu/useConfig";
import currentSite from "./site";

window.addEventListener("beforeunload", (event) => {
  // Catch beforeunload event
  event.stopImmediatePropagation();
});

try {
  Reflect.defineProperty(window, "closeClient", {
    get: () => () => ipc.send(IM.close_window),
    set: () => {
      //
    },
    configurable: true,
  });
} catch (err) {
  //
}

if (currentSite === "game") {
  createKeybind("F4", (event) => {
    if (event.altKey) ipc.send(IM.close_window);
    else ipc.send(IM.seek_game);
  });

  createKeybind("F11", () => {
    const config = getConfig();
    config.render.fullscreen = !config.render.fullscreen;
    setConfig(config);
    ipc.send(IM.fullscreen);
  });
}

// CTRL + SHIFT + I
createKeybind("KeyI", (event) => {
  if (event.ctrlKey && event.shiftKey) ipc.send(IM.open_devtools);
});

createKeybind("F5", () => ipc.send(IM.reload_window));

ipc.on(IM.get_ping_region, (id) => {
  ipc.send(id, localStorage.pingRegion7);
});
