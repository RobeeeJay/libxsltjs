libxsltjs
=========

libxslt bindings for node.js



Credits
=========

Based heavily on the work of Brian Suh and his node_xslt module.
https://github.com/bsuh/node_xslt



IMPORTANT
=========

This is very much a WIP, I'm just trying to get compatibility with the parts of node-xslt I was using to work, before I worry about anything else.

Use at your own risk!!!



INSTALLATION
=========

	npm install libxsltjs

IF you cloned it from Github then you need to go to the module's directory and run this:

	node-gyp configure build


Include it in your node project with:

	var xslt = require("libxsltjs");
