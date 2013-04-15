libxsltjs
=========

libxslt bindings for node.js



Credits
=========

Based heavily on the work of Brian Suh and his node_xslt module.
https://github.com/bahblah/node_xslt



IMPORTANT
=========

This is very much a WIP, I'm just trying to get compatibility with the parts of node-xslt I was using to work, before I worry about anything else.

Use at your own risk!!!



INSTALLATION
=========

	npm install libxsltjs

And then from the module's directory:

	node-gyp configure build


Include it in your node project with:

	var xslt = require("libxsltjs");
