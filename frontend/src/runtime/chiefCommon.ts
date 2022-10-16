/*
 * Chief Userscript support (in-game)
 */
import type Button from "../menu/components/Button";
import type ColorPicker from "../menu/components/ColorPicker";
import type Control from "../menu/components/Control";
import type FilePath from "../menu/components/FilePath";
import type FilePicker from "../menu/components/FilePicker";
import type Link from "../menu/components/Link";
import type Select from "../menu/components/Select";
import type { Set, HeadlessSet } from "../menu/components/Set";
import type Slider from "../menu/components/Slider";
import type Switch from "../menu/components/Switch";
import type Text from "../menu/components/Text";
import setLocalStorage from "../setLocalStorage";
import currentSite from "../site";
import type useLocalStorage from "../useLocalStorage";
import { sourceMappingURL } from "./common";
import type htm from "htm";
import MagicString from "magic-string";
import type { FunctionComponent, ReactElement } from "react";
import type React from "react";

export const renderSettings: FunctionComponent[] = [];
interface CallSettingsData {
  React: typeof React;
  // expose `htm` to allow for manipulating JSX
  html: typeof htm;
  // expose components for building a GUI extension
  UI: {
    Button: typeof Button;
    ColorPicker: typeof ColorPicker;
    Control: typeof Control;
    FilePath: typeof FilePath;
    FilePicker: typeof FilePicker;
    Link: typeof Link;
    Select: typeof Select;
    Set: typeof Set;
    HeadlessSet: typeof HeadlessSet;
    Slider: typeof Slider;
    Switch: typeof Switch;
    Text: typeof Text;
  };
  // important hook for lightweight configs
  useLocalStorage: typeof useLocalStorage;
}

interface ExportedUserscriptData {
  /**
   * Extend the client settings GUI.
   */
  Settings?: (data: CallSettingsData) => ReactElement;
}

type ExportUserscriptCallback = (data: ExportedUserscriptData) => void;

export type UserscriptContext = (
  code: string,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  console: typeof import("../console").default,
  getSite: () => typeof currentSite,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  setLocalStorage: typeof import("../setLocalStorage").default,
  exportUserscript: ExportUserscriptCallback
) => void;

/**
 * Run a Chief userscript
 */
export default function executeUserScript(
  script: string,
  code: string,
  exportUserscript: ExportUserscriptCallback
) {
  // eslint-disable-next-line no-new-func
  const run = new Function(
    "code",
    "console",
    "getSite",
    "setLocalStorage",
    "exportUserscript",
    "eval(code)"
  ) as UserscriptContext;

  const magic = new MagicString(code);

  run(
    magic.toString() +
      "//# " +
      sourceMappingURL(
        magic.generateMap({
          source: new URL("file:" + script).toString(),
        })
      ),
    console,
    () => currentSite,
    setLocalStorage,
    exportUserscript
  );
}