'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
var bodyParser = require('body-parser')
const app = express();
const path = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname + '/uploads')));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ 
  extended: true
}));

const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

//Get port number
const portNum = process.argv[2];

//Set static files
app.use(express.static(path.join(__dirname + '/images')));


//Declare C library functions
let parserLibrary = ffi.Library('./libsvgparser', {
  'getDescription': ['string', ['string', 'string']],
  'getTitle': ['string', ['string', 'string']],
  'rectListToJSONFromSVGFile': ['string', ['string', 'string']],
  'circleListToJSONFromSVGFile': ['string', ['string', 'string']],
  'pathListToJSONFromSVGFile': ['string', ['string', 'string']],
  'groupListToJSONFromSVGFile': ['string', ['string', 'string']],
  'SVGtoJSONFromSVGFile': ['string', ['string', 'string']],
  'changeSVGTitle': ['int', ['string', 'string', 'string']],
  'changeSVGDescription': ['int', ['string', 'string', 'string']],
  'addCircleToSVG': ['int', ['string', 'string', 'float', 'float', 'float', 'string']],
  'addRectToSVG': ['int', ['string', 'string', 'float', 'float', 'float', 'float', 'string']],
  'scaleShape': ['int', ['string', 'string', 'string', 'float']],
  'validSVGFile': ['int', ['string', 'string']],
  'userCreatedSVGFunction': ['int', ['string', 'string', 'string', 'string']],
  'otherAttributesFromShape': ['string', ['string', 'string', 'string', 'int']],
  'setAttributeInFile': ['int', ['string', 'string', 'string', 'int', 'string', 'string']],
});

app.get('/', function (req, res) {
  res.sendFile(path.join(__dirname + '/public/index.html'));
});

app.get('/style.css', function (req, res) {
  res.sendFile(path.join(__dirname + '/public/style.css'));
});

//Send obfuscated JS
app.get('/index.js', function (req, res) {
  fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function (err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, { compact: true, controlFlowFlattening: true });
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

app.post('/upload', function (req, res) {
  if (!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  //The mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function (err) {
    if (err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

app.get('/uploads/:name', function (req, res) {
  fs.stat('uploads/' + req.params.name, function (err, stat) {
    if (err == null) {
      res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: ' + err);
      res.send('');
    }
  });
});

app.get('/getUploadFiles', function (req, res) {
  let allFiles = [];
  let fileNames = fs.readdirSync('uploads'); //read in files from uploads folder

  for (let i = 0; i < fileNames.length; i++) {
    var newData = JSON.parse(parserLibrary.SVGtoJSONFromSVGFile('uploads/' + fileNames[i] + '', 'parser/svg.xsd'));
    allFiles.push(newData);
    allFiles[i].name = fileNames[i];
    var num = (fs.statSync('uploads/' + fileNames[i])["size"]) / 1000;
    allFiles[i].size = Math.round(num);
  }

  res.send({
    img: allFiles
  });
});

app.get('/userSelectedSVG', function (req, res) {
  var panelData = new Object();

  panelData.desc = parserLibrary.getDescription('uploads/' + req.query.file + '', 'parser/svg.xsd');
  panelData.title = parserLibrary.getTitle('uploads/' + req.query.file + '', 'parser/svg.xsd');

  var rectData = parserLibrary.rectListToJSONFromSVGFile('uploads/' + req.query.file + '', 'parser/svg.xsd');
  var circleData = parserLibrary.circleListToJSONFromSVGFile('uploads/' + req.query.file + '', 'parser/svg.xsd');
  var pathData = parserLibrary.pathListToJSONFromSVGFile('uploads/' + req.query.file + '', 'parser/svg.xsd');
  var groupData = parserLibrary.groupListToJSONFromSVGFile('uploads/' + req.query.file + '', 'parser/svg.xsd');

  panelData.rects = JSON.parse(rectData);
  panelData.circles = JSON.parse(circleData);
  panelData.paths = JSON.parse(pathData);
  panelData.groups = JSON.parse(groupData);

  res.send({
    panel: panelData
  })
});

app.get('/getAttributeFromShape', function (req, res) {
  var panelData = new Object();

  var tempObject = parserLibrary.otherAttributesFromShape('uploads/' + req.query.file, 'parser/svg.xsd', req.query.shape, req.query.number);
  panelData.otherAttributes = JSON.parse(tempObject);

  res.send({
    data: panelData
  })
});

app.post('/validateUpload', function (req, res) {
  if (!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function (err) {
    if (err) {
      return res.status(500).send(err);
    }

    if (parserLibrary.validSVGFile('uploads/' + uploadFile.name, 'parser/svg.xsd') != 1) {
      fs.unlinkSync('uploads/' + uploadFile.name);
      return res.status(406).send('FAILURE');
    }

    res.redirect('/');
  });
});

app.post('/userCreatedSVG', function (req, res) {
  if (parserLibrary.userCreatedSVGFunction('uploads/' + req.body.filename, 'parser/svg.xsd', req.body.title, req.body.desc) != 1) {
    console.log('Invalid Inputs');
    return res.status(406).send('Invalid Inputs');
  }

  res.redirect('/');
});

app.post('/changeTitle', function (req, res) {
  if (parserLibrary.changeSVGTitle('uploads/' + req.body.file + '', 'parser/svg.xsd', req.body.title) != 1) {
    console.log('Invalid Inputs');
    return res.status(406).send('Invalid Inputs');
  }

  res.redirect('/');
});

app.post('/changeDesc', function (req, res) {
  if (parserLibrary.changeSVGDescription('uploads/' + req.body.file + '', 'parser/svg.xsd', req.body.desc) != 1) {
    console.log('Invalid Inputs');
    return res.status(406).send('Invalid Inputs');
  }

  res.redirect('/');
});

app.post('/addRect', function (req, res) {
  if (parserLibrary.addRectToSVG('uploads/' + req.body.file, 'parser/svg.xsd', req.body.x, req.body.y, req.body.width, req.body.height, req.body.units) != 1) {
    console.log('invalid rect input');
    res.status(406).send('invalid rect input');
  }

  res.redirect('/');
});

app.post('/addCircle', function (req, res) {
  if (parserLibrary.addCircleToSVG('uploads/' + req.body.file, 'parser/svg.xsd', req.body.x, req.body.y, req.body.r, req.body.units) != 1) {
    console.log('invalid circle input');
    return res.status(406).send('invalid circle input');
  }

  res.redirect('/');
});

app.post('/scaleRects', function (req, res) {
  if (parserLibrary.scaleShape('uploads/' + req.body.file, 'parser/svg.xsd', 'rectangle', req.body.scale) != 1) {
    console.log('invalid scale factor');
    res.status(406).send('invalid scale factor');
  }

  res.redirect('/');
});

app.post('/scaleCircles', function (req, res) {
  if (parserLibrary.scaleShape('uploads/' + req.body.file, 'parser/svg.xsd', 'circle', req.body.scale) != 1) {
    console.log('invalid scale factor');
    return res.status(406).send('invalid scale factor');
  }

  res.redirect('/');
});

app.post('/setAttributeInShape', function (req, res) {
  if (parserLibrary.setAttributeInFile('uploads/' + req.body.file, 'parser/svg.xsd', req.body.shape, req.body.number, req.body.attrName, req.body.attrVal) != 1) {
    console.log('invalid attribute data');
    return res.status(406).send('invalid attribute data');
  }

  res.redirect('/');
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);