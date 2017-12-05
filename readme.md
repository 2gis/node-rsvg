# Node.JS Binding for LibRSVG

**LibRSVG** is a SVG rendering library, which parses SVG files and renders them in various formats. The formats include:

 *  PNG
 *  PDF
 *  PS
 *  SVG
 *  Raw memory buffer image

[![Build Status](https://travis-ci.org/2gis/node-rsvg.svg?branch=master)](https://travis-ci.org/2gis/node-rsvg)

This repository is fork of https://github.com/walling/node-rsvg and has windows support and don't need to export some variables for successfully building on OSX.
Also this fork fixes problem with zero-sized buffer rendered in some real rare cases.

## Basic Usage

Here is a simple example. Look in `index.js` for more documentation.

```javascript
var Rsvg = require('librsvg').Rsvg;
var fs = require('fs');

// Create SVG render instance.
var svg = new Rsvg();

// When finishing reading SVG, render and save as PNG image.
svg.on('finish', function() {
  console.log('SVG width: ' + svg.width);
  console.log('SVG height: ' + svg.height);
  fs.writeFile('tiger.png', svg.render({
    format: 'png',
    width: 600,
    height: 400
  }).data);
});

// Stream SVG file into render instance.
fs.createReadStream('tiger.svg').pipe(svg);
```

## Possible breaking changes

Before v0.6.0 in case of error `getBaseURI` returned `null`.
Since v0.6.0 possibly breaking change was introduced: now `getBaseURI` method always returns string. In case of error an empty string is returned, so it might affect user code that relies on strict equality to `null`. 

## Installation

First install the LibRSVG library and header files. Usually you have to look for a *development* package version. You must also have a functioning build tool chain including `pkg-config`. You can find instructions for different operating systems below. After that, you simply run:

```bash
npm install librsvg
```

Library versions known to work:

 *  LibRSVG 2.26+
 *  Cairo 1.8.8+

#### Ubuntu:

```bash
sudo apt-get install librsvg2-dev
```

#### RedHat / OpenSUSE:

```bash
sudo yum install librsvg2-devel
```

#### Mac OS X:

```bash
brew install librsvg
```

If, after installing LibRSVG through homebrew you are experiencing issues installing this module, try manually exporting the package config with this command:

```bash
export PKG_CONFIG_PATH=/opt/X11/lib/pkgconfig
```

Then try reinstalling this module. For further information, [see this thread](https://github.com/Homebrew/homebrew/issues/14123).

#### Windows:

You will need cairo and librsvg-2 libraries which is bundled in GTK.
Go to http://www.gtk.org/download/win64.php (or http://www.gtk.org/download/win32.php for 32-bit node)
and download the all-in-one bundle (these instructions used the following zip http://win32builder.gnome.org/gtk+-bundle_3.6.4-20131201_win64.zip).
Unzip the contents in C:\GTK (if you want to change this you must define -GTK_Root=c:\another\path shell variable to npm or node-gyp to reflect your changes),
and add "C:\GTK\bin;" to the PATH environment variable in Windows, it's necessary for node-rsvg runtime to load those libs.
