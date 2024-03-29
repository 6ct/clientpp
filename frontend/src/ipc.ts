import EventEmitter from "./EventEmitter";
import { IM, LogType } from "./IPCMessages";

export const ipcConsole = {
  log: (...args: unknown[]) =>
    ipc.send(IM.log, LogType.info, args.join(" ") + "\n"),
  warn: (...args: unknown[]) =>
    ipc.send(IM.log, LogType.warn, args.join(" ") + "\n"),
  error: (...args: unknown[]) =>
    ipc.send(IM.log, LogType.error, args.join(" ") + "\n"),
  debug: (...args: unknown[]) =>
    ipc.send(IM.log, LogType.debug, args.join(" ") + "\n"),
};

class IPC extends EventEmitter {
  ongoing_posts = new Set<number>();
  send(id: number, ...data: unknown[]) {
    if (typeof id !== "number")
      throw new TypeError(`Event must be a number. Recieved '${id}'`);
    chrome.webview.postMessage(JSON.stringify([id, ...data]));
    return true;
  }
  post<T = unknown>(event: number, ...data: unknown[]) {
    let id = 0xff; // reserved

    for (; id < 0xffff; id++) if (!this.ongoing_posts.has(id)) break;

    return new Promise<T>((resolve, reject) => {
      this.once(id, (data, err) => {
        this.ongoing_posts.delete(id);
        if (err) reject(err);
        else resolve(data);
      });
      this.send(event, id, ...data);
    });
  }
}

const ipc = new IPC();

chrome.webview.addEventListener("message", ({ data }) => {
  window.event = undefined;
  ipc.emit(...data);
});

export { IM, ipc as default };
