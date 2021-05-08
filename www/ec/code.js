/**
 * Blockly Demos: Code
 *
 * Copyright 2012 Google Inc.
 * https://developers.google.com/blockly/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @fileoverview JavaScript for Blockly's Code demo.
 * @author fraser@google.com (Neil Fraser)
 */
'use strict';

/**
 * Create a namespace for the application.
 */
var Code = {};

/**
 * Lookup for names of supported languages.  Keys should be in ISO 639 format.
 */
Code.LANGUAGE_NAME = {
  'ar': 'العربية',
  'be-tarask': 'Taraškievica',
  'br': 'Brezhoneg',
  'ca': 'Català',
  'cs': 'Česky',
  'da': 'Dansk',
  'de': 'Deutsch',
  'el': 'Ελληνικά',
  'en': 'English',
  'es': 'Español',
  'fa': 'فارسی',
  'fr': 'Français',
  'he': 'עברית',
  'hrx': 'Hunsrik',
  'hu': 'Magyar',
  'ia': 'Interlingua',
  'is': 'Íslenska',
  'it': 'Italiano',
  'ja': '日本語',
  'ko': '한국어',
  'mk': 'Македонски',
  'ms': 'Bahasa Melayu',
  'nb': 'Norsk Bokmål',
  'nl': 'Nederlands, Vlaams',
  'oc': 'Lenga d\'òc',
  'pl': 'Polski',
  'pms': 'Piemontèis',
  'pt-br': 'Português Brasileiro',
  'ro': 'Română',
  'ru': 'Русский',
  'sc': 'Sardu',
  'sk': 'Slovenčina',
  'sr': 'Српски',
  'sv': 'Svenska',
  'th': 'ภาษาไทย',
  'tlh': 'tlhIngan Hol',
  'tr': 'Türkçe',
  'uk': 'Українська',
  'vi': 'Tiếng Việt',
  'zh-hans': '簡體中文',
  'zh-hant': '正體中文'
};

/**
 * List of RTL languages.
 */
Code.LANGUAGE_RTL = ['ar', 'fa', 'he'];

/**
 * Blockly's main workspace.
 * @type {Blockly.WorkspaceSvg}
 */
Code.workspace = null;

/**
 * Extracts a parameter from the URL.
 * If the parameter is absent default_value is returned.
 * @param {string} name The name of the parameter.
 * @param {string} defaultValue Value to return if paramater not found.
 * @return {string} The parameter value or the default value if not found.
 */
Code.getStringParamFromUrl = function(name, defaultValue) {
  var val = location.search.match(new RegExp('[?&]' + name + '=([^&]+)'));
  return val ? decodeURIComponent(val[1].replace(/\+/g, '%20')) : defaultValue;
};

/**
 * Get the language of this user from the URL.
 * @return {string} User's language.
 */
Code.getLang = function() {
  var lang = Code.getStringParamFromUrl('lang', '');
  if (Code.LANGUAGE_NAME[lang] === undefined) {
    // Default to English.
    lang = 'en';
  }
  return lang;
};

/**
 * Is the current language (Code.LANG) an RTL language?
 * @return {boolean} True if RTL, false if LTR.
 */
Code.isRtl = function() {
  return Code.LANGUAGE_RTL.indexOf(Code.LANG) != -1;
};

/**
 * Load blocks saved on App Engine Storage or in session/local storage.
 * @param {string} defaultXml Text representation of default blocks.
 */
Code.loadBlocks = function(defaultXml) {
  try {
    var loadOnce = window.sessionStorage.loadOnceBlocks;
  } catch(e) {
    // Firefox sometimes throws a SecurityError when accessing sessionStorage.
    // Restarting Firefox fixes this, so it looks like a bug.
    var loadOnce = null;
  }
  if ('BlocklyStorage' in window && window.location.hash.length > 1) {
    // An href with #key trigers an AJAX call to retrieve saved blocks.
    BlocklyStorage.retrieveXml(window.location.hash.substring(1));
  } else if (loadOnce) {
    // Language switching stores the blocks during the reload.
    delete window.sessionStorage.loadOnceBlocks;
    var xml = Blockly.Xml.textToDom(loadOnce);
    Blockly.Xml.domToWorkspace(Code.workspace, xml);
  } else if (defaultXml) {
    // Load the editor with default starting blocks.
    var xml = Blockly.Xml.textToDom(defaultXml);
    Blockly.Xml.domToWorkspace(Code.workspace, xml);
  } else if ('BlocklyStorage' in window) {
    // Restore saved blocks in a separate thread so that subsequent
    // initialization is not affected from a failed load.
    window.setTimeout(BlocklyStorage.restoreBlocks, 0);
  }
};

/**
 * Save the blocks and reload with a different language.
 */
Code.changeLanguage = function() {
  // Store the blocks for the duration of the reload.
  // This should be skipped for the index page, which has no blocks and does
  // not load Blockly.
  // MSIE 11 does not support sessionStorage on file:// URLs.
  if (typeof Blockly != 'undefined' && window.sessionStorage) {
    var xml = Blockly.Xml.workspaceToDom(Code.workspace);
    var text = Blockly.Xml.domToText(xml);
    window.sessionStorage.loadOnceBlocks = text;
  }

  var languageMenu = document.getElementById('languageMenu');
  var newLang = encodeURIComponent(
      languageMenu.options[languageMenu.selectedIndex].value);
  var search = window.location.search;
  if (search.length <= 1) {
    search = '?lang=' + newLang;
  } else if (search.match(/[?&]lang=[^&]*/)) {
    search = search.replace(/([?&]lang=)[^&]*/, '$1' + newLang);
  } else {
    search = search.replace(/\?/, '?lang=' + newLang + '&');
  }

  window.location = window.location.protocol + '//' +
      window.location.host + window.location.pathname + search;
};

/**
 * Bind a function to a button's click event.
 * On touch enabled browsers, ontouchend is treated as equivalent to onclick.
 * @param {!Element|string} el Button element or ID thereof.
 * @param {!Function} func Event handler to bind.
 */
Code.bindClick = function(el, func) {
  if (typeof el == 'string') {
    el = document.getElementById(el);
  }
  el.addEventListener('click', func, true);
  el.addEventListener('touchend', func, true);
};

/**
 * Compute the absolute coordinates and dimensions of an HTML element.
 * @param {!Element} element Element to match.
 * @return {!Object} Contains height, width, x, and y properties.
 * @private
 */
Code.getBBox_ = function(element) {
  var height = element.offsetHeight;
  var width = element.offsetWidth;
  var x = 0;
  var y = 0;
  do {
    x += element.offsetLeft;
    y += element.offsetTop;
    element = element.offsetParent;
  } while (element);
  return {
    height: height,
    width: width,
    x: x,
    y: y
  };
};

/**
 * User's language (e.g. "en").
 * @type {string}
 */
Code.LANG = Code.getLang();

/**
 * List of tab names.
 * @private
 */
Code.TABS_ = ['blocks', 'javascript', 'xml'];

Code.selected = 'blocks';

/**
 * Switch the visible pane when a tab is clicked.
 * @param {string} clickedName Name of tab clicked.
 */
Code.tabClick = function(clickedName) {
  // If the XML tab was open, save and render the content.
  if (document.getElementById('tab_xml').className == 'tabon') {
    var xmlTextarea = document.getElementById('content_xml');
    var xmlText = xmlTextarea.value;
    var xmlDom = null;
    try {
      xmlDom = Blockly.Xml.textToDom(xmlText);
    } catch (e) {
      var q =
          window.confirm(MSG['badXml'].replace('%1', e));
      if (!q) {
        // Leave the user on the XML tab.
        return;
      }
    }
    if (xmlDom) {
      Code.workspace.clear();
      Blockly.Xml.domToWorkspace(Code.workspace, xmlDom);
    }
  }

  if (document.getElementById('tab_blocks').className == 'tabon') {
    Code.workspace.setVisible(false);
  }
  // Deselect all tabs and hide all panes.
  for (var i = 0; i < Code.TABS_.length; i++) {
    var name = Code.TABS_[i];
    document.getElementById('tab_' + name).className = 'taboff';
    document.getElementById('content_' + name).style.visibility = 'hidden';
  }

  // Select the active tab.
  Code.selected = clickedName;
  document.getElementById('tab_' + clickedName).className = 'tabon';
  // Show the selected pane.
  document.getElementById('content_' + clickedName).style.visibility =
      'visible';
  Code.renderContent();
  if (clickedName == 'blocks') {
    Code.workspace.setVisible(true);
  }
  Blockly.fireUiEvent(window, 'resize');
};

/**
 * Populate the currently selected pane with content generated from the blocks.
 */
Code.renderContent = function() {
  var content = document.getElementById('content_' + Code.selected);
  // Initialize the pane.
  if (content.id == 'content_xml') {
    var xmlTextarea = document.getElementById('content_xml');
    var xmlDom = Blockly.Xml.workspaceToDom(Code.workspace);
    var xmlText = Blockly.Xml.domToPrettyText(xmlDom);
    xmlTextarea.value = xmlText;
    xmlTextarea.focus();
  } else if (content.id == 'content_javascript') {
    var code = Blockly.JavaScript.workspaceToCode(Code.workspace);
    content.textContent = code;
  }
};

/**
 * Initialize Blockly.  Called on page load.
 */
Code.init = function() {
  Code.initLanguage();

  var rtl = Code.isRtl();
  var container = document.getElementById('content_area');
  var onresize = function(e) {
    var bBox = Code.getBBox_(container);
    for (var i = 0; i < Code.TABS_.length; i++) {
      var el = document.getElementById('content_' + Code.TABS_[i]);
      el.style.top = bBox.y + 'px';
      el.style.left = bBox.x + 'px';
      // Height and width need to be set, read back, then set again to
      // compensate for scrollbars.
      el.style.height = bBox.height + 'px';
      el.style.height = (2 * bBox.height - el.offsetHeight) + 'px';
      el.style.width = bBox.width + 'px';
      el.style.width = (2 * bBox.width - el.offsetWidth) + 'px';
    }
    // Make the 'Blocks' tab line up with the toolbox.
    if (Code.workspace && Code.workspace.toolbox_.width) {
      document.getElementById('tab_blocks').style.minWidth =
          (Code.workspace.toolbox_.width - 38) + 'px';
          // Account for the 19 pixel margin and on each side.
    }
  };
  onresize();
  window.addEventListener('resize', onresize, false);

  var toolbox = document.getElementById('toolbox');
  Code.workspace = Blockly.inject('content_blocks',
      {grid:
          {spacing: 25,
           length: 3,
           colour: '#ccc',
           snap: true},
       media: 'media/',
       rtl: rtl,
       toolbox: toolbox,
       zoom:
           {controls: true,
            wheel: true}
      });

  // Add to reserved word list: Local variables in execution environment (runJS)
  // and the infinite loop detection function.
  Blockly.JavaScript.addReservedWords('code,timeouts,checkTimeout');

  Code.loadBlocks('');

  Code.tabClick(Code.selected);    
    
  Code.bindClick('trashButton',
      function() {Code.discard(); Code.renderContent();});
  Code.bindClick('runButton', Code.run);
  document.getElementById('linkButton').className = 'disabled';
    
  for (var i = 0; i < Code.TABS_.length; i++) {
    var name = Code.TABS_[i];
    Code.bindClick('tab_' + name,
        function(name_) {return function() {Code.tabClick(name_);};}(name));
  }
    Blockly.Xml.domToWorkspace(Code.workspace,
        document.getElementById('startBlocks'));
};

/**
 * Initialize the page language.
 */
Code.initLanguage = function() {
  // Set the HTML's language and direction.
  var rtl = Code.isRtl();
  document.dir = rtl ? 'rtl' : 'ltr';
  document.head.parentElement.setAttribute('lang', Code.LANG);

  // Inject language strings.
  document.getElementById('tab_blocks').textContent = MSG['blocks'];

  document.getElementById('linkButton').title = MSG['linkTooltip'];
  document.getElementById('runButton').title = MSG['runTooltip'];
  document.getElementById('trashButton').title = MSG['trashTooltip'];

  var textVars = document.getElementsByClassName('textVar');
  for (var i = 0, textVar; textVar = textVars[i]; i++) {
    textVar.textContent = MSG['textVariable'];
  }
  var listVars = document.getElementsByClassName('listVar');
  for (var i = 0, listVar; listVar = listVars[i]; i++) {
    listVar.textContent = MSG['listVariable'];
  }
};

Code.handlePending = function() {
    Code.in_progress = false;

    // send anything that might be pending
    if(Code.pending.length > 0) {
	// send pending ...
	var item = Code.pending.shift();
	//console.log("send pending:" + item);	    
	Code.send(item);
    }
}
    
Code.send = function(chr) {
    console.log("Sending", chr);
    
    if(Code.characteristic === undefined)
	return;

    if(Code.encoder === undefined)
	Code.encoder = new TextEncoder();

    // check if no other transfer is in progress
    if(Code.in_progress) {
	//console.log("in progress, add to pending");
	Code.pending.push(chr);
	//console.log("pending now: " + Code.pending);
	return;
    }
    
    // TODO: make sure at most 20 Bytes are sent
    console.log("writeValue()");
    Code.in_progress = true;

    // for ftDuino just send the characters directly
    var characteristic = [ undefined, undefined ];
    var value = [ undefined, undefined ];
    if("uart" in Code.characteristic) {
	// ftduino
	characteristic[0] = Code.characteristic["uart"]
	value[0] = Code.encoder.encode(chr);
    } else {
	// ft controllers
	characteristic[0] = Code.characteristic["m1"]
	characteristic[1] = Code.characteristic["m2"]

	value = [ new Uint8Array([0]), new Uint8Array([0])];	    
	if(chr == 'r') // drive right -> only power m1
	    value[0] = new Uint8Array([100]);
	else if(chr == 'R') // turn right -> power m1 forward and m2 backward
	    value = [ new Uint8Array([100]), new Uint8Array([-100])];
	else if(chr == 'l') // drive left -> only power m2
	    value[1] = new Uint8Array([100]);
	else if(chr == 'L') // turn left -> power m1 backward and m2 forward
	    value = [ new Uint8Array([-100]), new Uint8Array([100])];
	else if(chr == 'f') // forward
	    value = [ new Uint8Array([100]), new Uint8Array([100])];
	else if(chr == 'b') // backward
	    value = [ new Uint8Array([-100]), new Uint8Array([-100])];
    }

    if(characteristic[0]) {    
	characteristic[0].writeValue(value[0]).then(function() {
	    console.log("writeValue(0,",value[0],") ok", characteristic);

	    // send second half if required
	    if(characteristic[1]) {
		characteristic[1].writeValue(value[1]).then(function() {
		    console.log("writeValue(1,",value[1],") ok");
		    Code.handlePending();		    
		}).catch(error => {
		    console.log("writeValue(1) error: " + error);
		    Code.handlePending();
		});
	    } else	    
		Code.handlePending();
	}).catch(error => {
            console.log("writeValue(0) error: " + error);
	    Code.handlePending();
	});
    } else {
	console.log("command ignored");
	Code.handlePending();
    }
}

Code.connect = function(run) {
    Code.device = undefined;
    Code.server = undefined;
    Code.service = undefined;
    Code.characteristic = {};

    navigator.bluetooth.requestDevice( {
	// this will not find the fischertechnik controllers ...
//	filters: [
//	    { services: ['0000ffe0-0000-1000-8000-00805f9b34fb']},
//	    { services: ['8ae883b4-ad7d-11e6-80f5-76304dec7eb7']},
//	    { services: ['2e58327e-c5c5-11e6-9d9d-cec0c932ce01']},
//	    { name: 'BT Control Receiver'},
//	    { name: 'BT Smart Controller'}
//	],
	acceptAllDevices: true,
	optionalServices: [
	    '0000ffe0-0000-1000-8000-00805f9b34fb',
	    '8ae883b4-ad7d-11e6-80f5-76304dec7eb7',
	    '2e58327e-c5c5-11e6-9d9d-cec0c932ce01' ]
    }).then(device => {
        console.log("Device",device, "found. Connecting ...");
	Code.device = device;
        return device.gatt.connect();
    }).then(server => {
	Code.server = server;
        console.log("Connected. Searching for primary service ...");
	// search for device specific service for ftDuino, Bt Smart Controller or BT Control Receiver
	var service = '0000ffe0-0000-1000-8000-00805f9b34fb';
	if(Code.device.name == 'BT Control Receiver')
	    service = '2e58327e-c5c5-11e6-9d9d-cec0c932ce01'
	else if(Code.device.name == 'BT Smart Controller')
	    service = '8ae883b4-ad7d-11e6-80f5-76304dec7eb7'
        return server.getPrimaryService(service);
    }).then(service => {
	Code.service = service
        console.log("Primary service found. Requesting characteristic ...");
	// search for first device specific characteristic. Will be the only one for the ftDuinos UART,
	// will be M1 for the ft controllers
	var characteristic = '0000ffe1-0000-1000-8000-00805f9b34fb';
	if(Code.device.name == 'BT Control Receiver')
	    characteristic = '2e583378-c5c5-11e6-9d9d-cec0c932ce01'
	else if(Code.device.name == 'BT Smart Controller')
	    characteristic = '8ae8860c-ad7d-11e6-80f5-76304dec7eb7'	
        return service.getCharacteristic(characteristic);
    }).then(characteristic => {
        console.log("First characteristic found.");
	Code.pending = [ ]
	Code.in_progress = false
	if((Code.device.name == 'BT Control Receiver') || (Code.device.name == 'BT Smart Controller')) {
	    Code.characteristic["m1"] = characteristic

	    if(Code.device.name == 'BT Control Receiver')
		characteristic = '2e583666-c5c5-11e6-9d9d-cec0c932ce01'
	    else
		characteristic = '8ae88b84-ad7d-11e6-80f5-76304dec7eb7'
	    console.log("requesting second characteristic");
	    return Code.service.getCharacteristic(characteristic);
	}	
	else {
	    Code.characteristic["uart"] = characteristic
	    // we don't need a second characteristic, so return an empty promise
	    return new Promise((resolve) => { resolve(42); });
	}
    }).then(characteristic => {
	if((Code.device.name == 'BT Control Receiver') || (Code.device.name == 'BT Smart Controller')) {
            console.log("Second characteristic found:", characteristic);
	    Code.characteristic["m2"] = characteristic
	}
	    
	run();
    }).catch(error => {
        alert("Error: " + error);
    });
}

var initInterpreter = function(interpreter, scope) {
    var wrapper = function(chr, time, callback) {
	console.log("send: " + chr + " time: " + time);
	Code.send(chr);
	setTimeout(callback, time);  // some delay to keep things slow
    };
    interpreter.setProperty(scope, 'send',
	    interpreter.createAsyncFunction(wrapper));
}

/**
 * Execute the user's code.
 * Just a quick and dirty eval.  Catch infinite loops.
 */
Code.run = function() {
    if (!('Interpreter' in window)) {
        // Interpreter lazy loads and hasn't arrived yet.  Try again later.
        setTimeout(Code.runJS, 250);
        return;
    }
    
    if(navigator.bluetooth === undefined) {
	alert("Your browser does not support Web Bluetooth.\n\n"+
	      "Please try e.g. Google Chrome and make sure "+
	      "Web Bluetooth is enabled.");
	return;
    }

    // not connected yet? Connect and then run
    if(Code.characteristic === undefined) {
	Code.connect(Code.runJS);
	return;
    }

    // else just run
    Code.runJS();
}
    
Code.runJS = function() {
  if(Code.running) {
      console.log("Stopping");
      Code.running = false;
    return;
  }

  console.log("Running");
  var code = Blockly.JavaScript.workspaceToCode(Code.workspace);
  try {
      var interpreter = new Interpreter(code, initInterpreter);
      Code.running = true;
      var runner = function() {
          if(interpreter.run() && Code.running) {
              setTimeout(runner, 10);
          } else {
	      Code.send('x');
	      Code.running = false;
          }
      }
      runner();

  } catch (e) {
    alert(MSG['badCode'].replace('%1', e));
  }
};

/**
 * Discard all blocks from the workspace.
 */
Code.discard = function() {
  var count = Code.workspace.getAllBlocks().length;
  if (count < 2 ||
      window.confirm(MSG['discard'].replace('%1', count))) {
    Code.workspace.clear();
    window.location.hash = '';
  }
};

// Load the Code demo's language strings.
document.write('<script src="msg/' + Code.LANG + '.js"></script>\n');
// Load Blockly's language strings.
document.write('<script src="msg/js/' + Code.LANG + '.js"></script>\n');

window.addEventListener('load', Code.init);
