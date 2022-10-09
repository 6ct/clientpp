import RPC from "../RPC";
import ipc, { IM } from "../ipc";
import { renderSettings } from "../runtime/chief";
import Button from "./components/Button";
import FilePath from "./components/FilePath";
import Link from "./components/Link";
import Select from "./components/Select";
import { HeadlessSet, Set } from "./components/Set";
import Switch from "./components/Switch";
import Text from "./components/Text";
import { useConfig } from "./useConfig";
import type { MouseEvent } from "react";

const seek_modes = [
  "Free for All",
  "Team Deathmatch",
  "Hardpoint",
  "Capture the Flag",
  "Parkour",
  "Hide & Seek",
  "Infected",
  "Race",
  "Last Man Standing",
  "Simon Says",
  "Gun Game",
  "Prop Hunt",
  "Boss Hunt",
  "unused",
  "unused",
  "Stalker",
  "King of the Hill",
  "One in the Chamber",
  "Trade",
  "Kill Confirmed",
  "Defuse",
  "Sharp Shooter",
  "Traitor",
  "Raid",
  "Blitz",
  "Domination",
  "Squad Deathmatch",
  "Kranked FFA",
];

function open_in_shell(event: MouseEvent<HTMLAnchorElement>) {
  event.preventDefault();
  ipc.send(IM.shell_open, "url", event.currentTarget.href);
}

const rpc = new RPC();

export default function Menu() {
  const [config, setConfig] = useConfig();

  return (
    <>
      <HeadlessSet>
        <Link
          title="GitHub"
          href="https://github.com/6ct/clientpp"
          onClick={open_in_shell}
        />
        <Link
          title="Discord"
          href="https://discord.gg/4r47ZwdSQj"
          onClick={open_in_shell}
        />
        <Switch
          title="Devtools"
          defaultChecked={config.client.devtools}
          onChange={(event) => {
            config.client.devtools = event.currentTarget.checked;
            setConfig(config);
          }}
        />
      </HeadlessSet>
      <Set title="Folders">
        <Button
          title="Scripts"
          text="Open"
          onClick={() => ipc.send(IM.shell_open, "scripts")}
        />
        <Button
          title="Styles"
          text="Open"
          onClick={() => ipc.send(IM.shell_open, "styles")}
        />
        <Button
          title="Resource Swapper"
          text="Open"
          onClick={() => ipc.send(IM.shell_open, "swapper")}
        />
      </Set>
      <Set title="Rendering">
        <Switch
          title="Fullscreen"
          defaultChecked={config.render.fullscreen}
          onChange={(event) => {
            config.render.fullscreen = event.currentTarget.checked;
            setConfig(config);
            ipc.send(IM.fullscreen);
          }}
        />
        <Switch
          title="Uncap FPS"
          defaultChecked={config.render.uncap_fps}
          description="Restart Required"
          attention
          onChange={(event) => {
            config.render.uncap_fps = event.currentTarget.checked;
            setConfig(config);
            ipc.send(IM.relaunch_webview);
          }}
        />
        <Select
          title="Angle backend"
          defaultValue={config.render.angle}
          description="Restart Required"
          attention
          onChange={(event) => {
            config.render.angle = event.currentTarget.value;
            setConfig(config);
            ipc.send(IM.relaunch_webview);
          }}
        >
          <option value="default">Default</option>
          <option value="d3d11on12">Direct3D 11 on 12</option>
          <option value="d3d11">Direct3D 11</option>
          <option value="d3d9">Direct3D 9 </option>
          <option value="gl">OpenGL</option>
        </Select>
        <Select
          title="Color profile"
          defaultValue={config.render.color}
          description="Restart Required"
          attention
          onChange={(event) => {
            config.render.color = event.currentTarget.value;
            setConfig(config);
            ipc.send(IM.relaunch_webview);
          }}
        >
          <option value="default">Default</option>
          <option value="d3d11on12">SRGB</option>
          <option value="generic-rgb">RGB</option>
        </Select>
      </Set>
      <Set title="Game">
        <Switch
          title="Seek new Lobby [F4]"
          defaultChecked={config.game.seek.F4}
          onChange={(event) => {
            config.game.seek.F4 = event.currentTarget.checked;
            setConfig(config);
          }}
        />
        <Switch
          title="Custom seek (enables map, mode, and customs)"
          defaultChecked={config.game.seek.custom_logic}
          onChange={(event) => {
            config.game.seek.custom_logic = event.currentTarget.checked;
            setConfig(config);
          }}
        />
        <Text
          title="Seek Map"
          placeholder="Map ID"
          defaultValue={config.game.seek.map}
          onChange={(event) => {
            config.game.seek.map = event.currentTarget.value;
            setConfig(config);
          }}
        />
        <Select
          title="Seek mode"
          defaultValue={config.game.seek.mode}
          onChange={(event) => {
            config.game.seek.mode = event.currentTarget.value;
            setConfig(config);
          }}
        >
          <option value="">Any</option>
          {seek_modes.map((mode, i) => (
            <option key={i}>{mode}</option>
          ))}
        </Select>
        <Switch
          title="Seek customs"
          defaultChecked={config.game.seek.customs}
          onChange={(event) => {
            config.game.seek.customs = event.currentTarget.checked;
            setConfig(config);
          }}
        />
      </Set>
      <Set title="Discord RPC">
        <Switch
          title="Enabled"
          defaultChecked={config.rpc.enabled}
          onChange={(event) => {
            config.rpc.enabled = event.currentTarget.checked;
            setConfig(config);
            if (config.rpc.enabled) {
              ipc.send(IM.rpc_init);
              rpc.update(true);
            } else {
              ipc.send(IM.rpc_clear);
            }
          }}
        />
        <Switch
          title="Show username"
          defaultChecked={config.rpc.name}
          onChange={(event) => {
            config.rpc.enabled = event.currentTarget.checked;
            setConfig(config);
            ipc.send(IM.rpc_clear);
            ipc.send(IM.rpc_init);
            rpc.update(true);
          }}
        />
      </Set>
      <Set title="Window">
        <Switch
          title="Replace Icon & Title "
          defaultChecked={config.window.meta.replace}
          onChange={(event) => {
            config.window.meta.replace = event.currentTarget.checked;
            setConfig(config);
            if (config.window.meta.replace) ipc.send(IM.update_meta);
            else ipc.send(IM.revert_meta);
          }}
        />
        <Text
          title="New Title"
          defaultValue={config.window.meta.title}
          onChange={(event) => {
            config.window.meta.title = event.currentTarget.value;
            setConfig(config);
            if (config.window.meta.replace) ipc.send(IM.update_meta);
          }}
        />
        <FilePath
          title="New Icon"
          defaultValue={config.window.meta.icon}
          onPick={(value) => {
            config.window.meta.icon = value;
            setConfig(config);
            if (config.window.meta.replace) ipc.send(IM.update_meta);
          }}
        >
          <option value="*.ico">Icon</option>
          <option value="*.*">All types</option>
        </FilePath>
      </Set>
      {renderSettings.map((Settings, i) => (
        <Settings key={i} />
      ))}
    </>
  );
}
