'use strict';

try{
	Object.defineProperties(window, {
		onbeforeunload: { writable: false, value(){} },
		closeClient: { writable: false, value(){ ipc.send(IM.close_window) } },
	});
}catch(err){
	
}

require('./FilePicker');

var ExtendMenu = require('./libs/ExtendMenu'),
	Keybind = require('./libs/Keybind'),
	utils = require('./libs/Utils'),
	RPC = require('./RPC'),
	console = require('./console'),
	{ ipc, IM } = require('./IPC'),
	{ config: runtime_config, js } = require('./Runtime'),
	site = require('./Site'),
	run_resources = require('./Resources');

class Menu extends ExtendMenu {
	rpc = new RPC();
	save_config(){
		ipc.send(IM.save_config, this.config);
	}
	config = runtime_config;
	default_config = require('../../Resources/Config.json');
	constructor(){
		super('Game Settings', 'Client');
		
		var Client = this.category('Client');
		
		Client.control('Github', {
			type: 'linkfunction',
			value(){
				ipc.send(IM.shell_open, 'url', 'https://github.com/6ct/clientpp');
			},
		});
		
		Client.control('Discord', {
			type: 'linkfunction',
			value(){
				ipc.send(IM.shell_open, 'url', 'https://discord.gg/4r47ZwdSQj');
			},
		});
		
		Client.control('DevTools [F10]', {
			type: 'boolean',
			walk: 'client.devtools',
		});
		
		var Folder = this.category('Folders');
		
		Folder.control('Scripts', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send(IM.shell_open, 'scripts'),
		});
		
		Folder.control('Styles', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send(IM.shell_open, 'styles'),
		});
		
		Folder.control('Resource Swapper', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send(IM.shell_open, 'swapper'),
		});
		
		var Render = this.category('Rendering');
		
		Render.control('Fullscreen', {
			type: 'boolean',
			walk: 'render.fullscreen',
		}).on('change', (value, init) => !init && ipc.send(IM.fullscreen));
		
		Render.control('Uncap FPS', {
			type: 'boolean',
			walk: 'render.uncap_fps',
		}).on('change', (value, init) => !init && ipc.send(IM.relaunch_webview));
		
		Render.control('Angle backend', {
			type: 'dropdown',
			walk: 'render.angle',
			value: {
				Default: 'default',
				'Direct3D 11 on 12': 'd3d11on12',
				'Direct3D 11': 'd3d11',
				'Direct3D 9': 'd3d9',
				'OpenGL': 'gl',
			},
		}).on('change', (value, init) => !init && ipc.send(IM.relaunch_webview));
		
		Render.control('Color profile', {
			type: 'dropdown',
			walk: 'render.color',
			value: {
				Default: 'default',
				'SRGB': 'srgb',
				'RGB': 'generic-rgb',
			},
		}).on('change', (value, init) => !init && ipc.send(IM.relaunch_webview));
		
		var Game = this.category('Game');
		
		Game.control('Seek new Lobby [F4]', {
			type: 'boolean',
			walk: 'game.seek.F4',
		});
		
		Game.control('Seek map', {
			type: 'textbox',
			walk: 'game.seek.map',
			placeholder: 'Empty for any map',
		});
		
		var modes = {
			Any: '',
		};
		
		for(let name of ["Free for All", "Team Deathmatch", "Hardpoint", "Capture the Flag", "Parkour", "Hide & Seek", "Infected", "Race", "Last Man Standing", "Simon Says", "Gun Game", "Prop Hunt", "Boss Hunt", "unused", "unused", "Stalker", "King of the Hill", "One in the Chamber", "Trade", "Kill Confirmed", "Defuse", "Sharp Shooter", "Traitor", "Raid", "Blitz", "Domination", "Squad Deathmatch", "Kranked FFA"])modes[name] = name;
		
		Game.control('Seek mode', {
			type: 'dropdown',
			walk: 'game.seek.mode',
			value: {
				Any: '',
				...modes,
			},
		});
		
		Game.control('Seek customs', {
			type: 'boolean',
			walk: 'game.seek.customs',
		});
		
		var RPC = this.category('Discord RPC');
		
		RPC.control('Enabled', {
			type: 'boolean',
			walk: 'rpc.enabled',
		}).on('change', (value, init) => {
			if(init)return;
			if(!value)ipc.send(IM.rpc_clear);
			else{
				ipc.send(IM.rpc_init);
				this.rpc.update(true);
			}
		});
		
		RPC.control('Show username', {
			type: 'boolean',
			walk: 'rpc.name',
		}).on('change', (value, init) => !init && this.rpc.update(true));
		
		var Window = this.category('Window');
		
		Window.control('Replace Icon & Title', {
			type: 'boolean',
			walk: 'window.meta.replace',
		}).on('change', (value, init) => {
			if(init)return;
			
			if(value)ipc.send(IM.update_meta);
			else ipc.send(IM.revert_meta);
		});
		
		Window.control('New Title', {
			type: 'textbox',
			walk: 'window.meta.title',
		}).on('change', (value, init) => {
			if(!init && this.config.window.meta.replace)
				ipc.send(IM.update_meta);
		});
		
		Window.control('New Icon', {
			type: 'filepicker',
			walk: 'window.meta.icon',
			title: 'Select a new Icon',
			filters: {
				'Icon': '*.ico',
				'All types': '*.*',
			},
		}).on('change', (value, init) => {
			if(!init && this.config.window.meta.replace)
				ipc.send(IM.update_meta);
		});
		
		this.attach();
	}
	update(){
		for(let category of this.categories)category.update(true);
	}
};

new Keybind('F4', event => {
	if(event.altKey)ipc.send(IM.close_window);
	else ipc.send(IM.seek_game, localStorage.pingRegion7);
});

new Keybind('F11', () => {
	ipc.send(IM.toggle_fullscreen);
});

new Keybind('F10', event => {
	ipc.send(IM.open_devtools);
});

if(site == 'game'){
	require('./Fixes');
	require('./AccountManager');
	let menu = window.TEST = new Menu();
	run_resources(menu);
	menu.update();
	
	ipc.on(IM.update_menu, config => {
		menu.config = config;
		menu.update();
	});
}else run_resources();

new Keybind('F5', event => ipc.send(IM.reload_window));