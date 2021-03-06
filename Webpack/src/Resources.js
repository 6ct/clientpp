'use strict';

var { css, js } = require('./Runtime'),
	site = require('./Site'),
	utils = require('./libs/Utils'),
	{ ipc } = require('./IPC'),
	console = require('./Console'),
	LegacyUserscript = require('./LegacyUserscript'),
	ChiefUserscript = require('./ChiefUserscript'),
	add_css = () => {
		for(let [ name, data ] of Object.entries(css)){
			let url = URL.createObjectURL(new Blob([ data ], { type: 'text/css' }));
			
			let link = document.head.appendChild(Object.assign(document.createElement('link'), {
				rel: 'stylesheet',
				href: url,
			}));

			link.addEventListener('load', () => {
				URL.revokeObjectURL(url);
			});
			
			document.head.appendChild(link);
		}
	};

module.exports = menu => {
	for(let [ name, [ data, metadata, errors ] ] of Object.entries(js)){
		if(metadata){
			if(errors)for(let error of errors)console.error(error);
			else new ChiefUserscript(name, metadata).run(data, site, menu);
		}else{ // legacy idkr, unknown
			// quick fix
			if(data.includes('// ==UserScript==') && site != 'game')continue;
			
			let module = { exports: {} },
				func,
				context = {
					module,
					exports: module.exports,
					console,
				};
			
			try{
				func = eval(`(function(${Object.keys(context)}){${data}//# sourceURL=${name}\n})`);
			}catch(err){
				console.warn(`Error parsing userscript: ${name}\n`, err);
				ipc.console.error(`Error parsing userscript ${name}:\n${err}`);
				break;
			}
			
			// try{...}catch(err){...} doesnt provide: line, column
			
			try{
				func(...Object.values(context));
				
				let userscript = new LegacyUserscript(module.exports);
				
				userscript.run();
			}catch(err){
				console.warn(`Error executing userscript: ${name}\n`, err);
				ipc.console.error(`Error executing userscript ${name}:\n${err}`);
				break;
			}
		}
	}
};

new MutationObserver((mutations, observer) => {
	for(let mutation of mutations){
		for(let node of mutation.addedNodes){
			if(node.nodeName == 'LINK' && new URL(node.href || '/', location).pathname == '/css/main_custom.css'){
				add_css();
				observer.disconnect();
			}
		}
	}
}).observe(document, {
	childList: true,
	subtree: true,
});