'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

let sharedLib = ffi.Library('./libgpxparser', {
  'gpxFileToJson': [ 'string', [ 'string'] ],
  'gpxFileToRouteJson': [ 'string', [ 'string'] ],
  'gpxFileToTrackJson': [ 'string', [ 'string'] ],
  'componentDataToJSON': [ 'string' , [ 'string', 'string', 'int'] ],
  'editComponentName': [ 'string' , [ 'string', 'string', 'string', 'int'] ],
  'createGPXFromJSON': [ 'string' , [ 'string', 'string'] ],
  'addRouteToFile': [ 'string' , [ 'string', 'string'] ],
  'addWaypointToRoute': [ 'string' , [ 'string', 'string', 'string'] ],
  'findRoutePathData': [ 'string' , [ 'string', 'double', 'double', 'double', 'double', 'double'] ],
  'findTrackPathData': [ 'string' , [ 'string', 'double', 'double', 'double', 'double', 'double'] ],
  'routepointsToJSON': [ 'string' , [ 'string', 'int'] ]
});



app.get('/getFileTable', function(req, res){
  let directory_name = "uploads";

  var array = [];
  var files;

  fs.readdir("uploads", (err, files) => {
    if (err) {
      console.error("Could not list the directory");
      console.log("end of files");
    }
    else {
      console.log("\nCurrent directory filenames:");
      files.forEach(file => {
        console.log(file);
        if (path.extname(file) == ".gpx"){
          let string = sharedLib.gpxFileToJson("uploads/".concat(file));
          if (string.localeCompare("ERROR: FILE NOT VALID")) {
            let gpx = JSON.parse(string);
            gpx.name = file;
            array.push(gpx);
          }
        }
      })
    } 
    res.send(
      {
        gpxArray: array
      }
    );
  });

  console.log("array2"+array);
});

app.get('/getFileComponents', function(req, res){
  let directory_name = "uploads";

  var routeArray = [];
  var trackArray = [];
  var file = req.query.fileName;
  console.log("the file is "+file);
  if (path.extname(file) == ".gpx"){
    let routeString = sharedLib.gpxFileToRouteJson("uploads/".concat(file));
    let trackString = sharedLib.gpxFileToTrackJson("uploads/".concat(file));
    
      routeArray = JSON.parse(routeString);
      trackArray = JSON.parse(trackString);
      console.log(routeArray);
      console.log(trackArray);
    
  }

  res.send(
    {
      routeArray: routeArray,
      trackArray: trackArray
    }
  );
});

app.get('/getComponentData', function(req, res){
  let directory_name = "uploads";

  var dataArray = [];
  var file = req.query.fileName;
  console.log(req.query.option+req.query.index);
  console.log("the file is "+file);
  if (path.extname(file) == ".gpx"){
    let dataString = sharedLib.componentDataToJSON("uploads/".concat(file), req.query.option, parseInt(req.query.index, 10));
      dataArray = JSON.parse(dataString);
      console.log("the component string is "+dataString);
  }

  res.send(
    {
      dataArray: dataArray
    }
  );
});

app.get('/changeComponentName', function(req, res){
  let directory_name = "uploads";
  var dataString;
  var file = req.query.fileName;
  console.log("newName = "+req.query.newName+"component = "+req.query.component+req.query.index);
  console.log("the file is "+file);
  if (path.extname(file) == ".gpx"){
      dataString = sharedLib.editComponentName("uploads/".concat(file), req.query.newName, req.query.component, parseInt(req.query.index, 10));
      console.log("the component string is "+dataString);
  }

  res.send(
    {
      dataString: dataString
    }
  );
});

app.get('/createGPX', function(req, res){
  let directory_name = "uploads";
  var dataString;
  var file = req.query.fileName;
  console.log("file = "+req.query.fileName+" string = "+req.query.jsonString);
  console.log("the file is "+file);
  if (path.extname(file) == ".gpx"){
      dataString = sharedLib.createGPXFromJSON("uploads/".concat(file), req.query.jsonString);
      console.log("the component string is "+dataString);
  }
  else{
     dataString = "FALSE";
  }

  res.send(
    {
      dataString: dataString
    }
  );
});

app.get('/addRoute', function(req, res){
  let directory_name = "uploads";
  var dataString;
  var file = req.query.fileName;
  console.log("file = "+req.query.fileName+" string = "+req.query.jsonString+" routeName = "+req.query.routeName);
  console.log("the file is "+file);
  if (path.extname(file) == ".gpx"){
      var obj = new Object();
      var array = new Array();
      obj.name = req.query.routeName;
      array = JSON.parse(req.query.jsonString);

      var nameJson = JSON.stringify(obj);
      dataString = sharedLib.addRouteToFile("uploads/".concat(file), nameJson );
      for (var i = 0, l = array.length; i < l; i++) {
        dataString = sharedLib.addWaypointToRoute("uploads/".concat(file), req.query.routeName, JSON.stringify(array[i]));
        console.log("the component string is "+dataString);
      }
  }
  else{
     dataString = "FALSE";
  }

  res.send(
    {
      dataString: dataString
    }
  );
});

app.get('/getBetween', function(req, res){
  let directory_name = "uploads";

  var array = [];
  var gpx = new Array();
  var trackArray = new Array();
  var routeArray = new Array();
  var routeString;
  var trackString;
  fs.readdir("uploads", (err, files) => {
    if (err) {
      console.error("Could not list the directory");
      console.log("end of files");
    }
    else {
      console.log("\nCurrent directory filenames:");
      files.forEach(file => {
        console.log(file);
        console.log("number = "+req.query.Tol);
        if (path.extname(file) == ".gpx"){
          routeString = sharedLib.findRoutePathData("uploads/".concat(file), req.query.SourceLat, req.query.SourceLon, req.query.DestLat, req.query.DestLon, req.query.Tol);
          trackString = sharedLib.findTrackPathData("uploads/".concat(file), req.query.SourceLat, req.query.SourceLon, req.query.DestLat, req.query.DestLon, req.query.Tol);
          if (routeString.localeCompare("FALSE")) {
            console.log("routeString = "+routeString);
            gpx = JSON.parse(routeString);
            if (gpx.length>0) {
              routeArray = [].concat(routeArray, gpx);
            }
          }
          if (trackString.localeCompare("FALSE")) {
            console.log("trackString = "+trackString);
            gpx = JSON.parse(trackString);
            if (gpx.length > 0) {
              trackArray = [].concat(trackArray, gpx);
            }
          }
        }
      })
    } 
    console.log("routeArray = "+JSON.stringify(routeArray));
    console.log("trackArray = "+JSON.stringify(trackArray));
    res.send(
      {
        routeArray: routeArray,
        trackArray: trackArray
      }
    );
  });

});

//CHECK CONNECTION----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
app.get('/loginCheck', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);

      connection.execute("CREATE TABLE IF NOT EXISTS FILE(gpx_id INT AUTO_INCREMENT PRIMARY KEY, file_name VARCHAR(60) NOT NULL, ver DECIMAL(2,1) NOT NULL, creator VARCHAR(256) NOT NULL)");
      connection.execute("CREATE TABLE IF NOT EXISTS ROUTE(route_id INT AUTO_INCREMENT PRIMARY KEY, route_name VARCHAR(256), route_len FLOAT(15,7) NOT NULL, gpx_id INT NOT NULL, FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE)");
      connection.execute("CREATE TABLE IF NOT EXISTS POINT(point_id INT AUTO_INCREMENT PRIMARY KEY, point_index INT NOT NULL, latitude DECIMAL(11,7) NOT NULL, longitude DECIMAL(11,7) NOT NULL, point_name VARCHAR(256), route_id INT NOT NULL, FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE)");
  }catch(e){
      check = false;
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      check: check
    }
  );
});

//ADD FILES TO SQL TABLE----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
app.get('/FILETABLE', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  try{
      // create the connection
      const mysql = require('mysql2/promise');
      connection = await mysql.createConnection(req.query.dbConf);
      await connection.execute("DELETE FROM FILE");
      
      await connection.execute("ALTER TABLE FILE AUTO_INCREMENT = 1");
      await connection.execute("ALTER TABLE ROUTE AUTO_INCREMENT = 1");
      await connection.execute("ALTER TABLE POINT AUTO_INCREMENT = 1");
      var files;

      var gpxInsertArray = [];
      var routeInsertArray = [];
      var waypointInsertArray = [];
      var route_index = 0;
      fs.readdir("uploads", async(err, files) => {
        var count1 = 0;
        files.forEach(file => {
          console.log(file);
          var count2 = 0;
              if (path.extname(file) == ".gpx"){
                let string = sharedLib.gpxFileToJson("uploads/".concat(file));
                if (string.localeCompare("ERROR: FILE NOT VALID")) {
                  count1++;
                let gpx = JSON.parse(string);
                  gpx.name = file;
                  gpxInsertArray.push("INSERT INTO FILE ( file_name, ver, creator ) VALUES ( '"+file+"', "+gpx.version+", '"+gpx.creator+"' )");
                }
              
              //ROUTES
              var routeArray = new Object();
              let routeString = sharedLib.gpxFileToRouteJson("uploads/".concat(file));
              if (routeString.localeCompare("ERROR: FILE NOT VALID")) {
                console.log("the route string is "+routeString);
                routeArray = JSON.parse(routeString);

                
                
                routeArray.forEach(routeArray => {
                  count2++;
                  route_index++;
                  var waypointArray = new Object();
                  routeInsertArray.push("INSERT INTO ROUTE ( route_name, route_len, gpx_id ) VALUES ( '"+routeArray.name+"', "+routeArray.len+", "+count1+" )");
                  console.log("fileCount is: " + count1);
                  console.log("------------> "+routeInsertArray.length);

                  //WAYPOINTS
                  let waypointString = sharedLib.routepointsToJSON("uploads/".concat(file), count2);
                  waypointArray = JSON.parse(waypointString);
                  console.log("Way String is: "+waypointString);

                  var count3 = -1;
                  waypointArray.forEach(waypointArray => {
                    count3++;
                    if (!waypointArray.name.localeCompare("None")) {
                      waypointInsertArray.push("INSERT INTO POINT ( point_index, latitude, longitude, route_id ) VALUES ( "+count3+", "+waypointArray.lat+", "+waypointArray.lon+", "+route_index+" )");
                    }
                    else{
                      waypointInsertArray.push("INSERT INTO POINT ( point_index, latitude, longitude, point_name, route_id ) VALUES ( "+count3+", "+waypointArray.lat+", "+waypointArray.lon+", "+waypointArray.name+", "+route_index+" )");
                    }
                  });
                });
            }
          }
        });
        //console.log("WayInsert String is: "+waypointInsertArray);
        var i;
        for (i = 0; i < gpxInsertArray.length; i++) {
          try {
            connection = await mysql.createConnection(req.query.dbConf);
            await connection.execute(gpxInsertArray[i]);
          } catch (e) {
            check = false;
            console.log("2Query error: "+e);
          }finally {
            if (connection && connection.end) connection.end();
          } 
        }
        for (i = 0; i < routeInsertArray.length; i++) {
          try {
            connection = await mysql.createConnection(req.query.dbConf);
            await connection.execute(routeInsertArray[i]);
          } catch (e) {
            check = false;
            console.log("2Query error: "+e);
          }finally {
            if (connection && connection.end) connection.end();
          } 
        }
        for (i = 0; i < waypointInsertArray.length; i++) {
          try {
            connection = await mysql.createConnection(req.query.dbConf);
            await connection.execute(waypointInsertArray[i]);
          } catch (e) {
            check = false;
            console.log("2Query error: "+e);
          } finally {
            if (connection && connection.end) connection.end();
          }
        }
      });
      console.log("------------> "+routeInsertArray.length);

      
  }catch(e){
      check = false;
      console.log("1Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      check: check
    }
  );
  
});

app.get('/CLEAR', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  try{
      // create the connection
      const mysql = require('mysql2/promise');
      connection = await mysql.createConnection(req.query.dbConf);
      await connection.execute("DELETE FROM FILE");
      
      await connection.execute("ALTER TABLE FILE AUTO_INCREMENT = 1");
      await connection.execute("ALTER TABLE ROUTE AUTO_INCREMENT = 1");
      await connection.execute("ALTER TABLE POINT AUTO_INCREMENT = 1");
      

  }catch(e){
      check = false;
      console.log("1Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
});

//Routes by Length
app.get('/routeLen', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);
      const [rows1, fields1] = await connection.execute("SELECT * FROM ROUTE ORDER BY route_len ASC");
      var array = rows1;

  }catch(e){
      var array =[];
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      rows: array
    }
  );
});

//Routes by Name
app.get('/routeName', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);
      const [rows1, fields1] = await connection.execute("SELECT * FROM ROUTE ORDER BY route_name");
      var array = rows1;

  }catch(e){
      var array =[];
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      rows: array
    }
  );
});

//File Routes by Name
app.get('/fileRouteName', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);
      const [rows1, fields1] = await connection.execute("SELECT * FROM ROUTE WHERE (gpx_id = (SELECT gpx_id FROM FILE WHERE (file_name = '"+req.query.file+"' ) ) ) ORDER BY route_name");
      var array = rows1;

  }catch(e){
      var array =[];
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      rows: array
    }
  );
});

app.get('/fileRouteLen', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);
      const [rows1, fields1] = await connection.execute("SELECT * FROM ROUTE WHERE (gpx_id = (SELECT gpx_id FROM FILE WHERE (file_name = '"+req.query.file+"' ) ) ) ORDER BY route_len ASC");
      var array = rows1;

  }catch(e){
      var array =[];
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      rows: array
    }
  );
});

app.get('/routePoints', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);
      const [rows1, fields1] = await connection.execute("SELECT * FROM POINT WHERE (route_id = '"+req.query.index+"' ) ORDER BY point_index");
      var array = rows1;

  }catch(e){
      var array =[];
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      rows: array
    }
  );
});

app.get('/filePointsName', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);
      const [rows1, fields1] = await connection.execute("SELECT * FROM POINT WHERE route_id IN ( SELECT route_id FROM ROUTE WHERE (gpx_id = (SELECT gpx_id FROM FILE WHERE (file_name = '"+req.query.file+"' ) ) ) ORDER BY route_name )");
      var array = rows1;

  }catch(e){
      var array =[];
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      rows: array
    }
  );
});

app.get('/filePointsLen', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);
      const [rows1, fields1] = await connection.execute("SELECT * FROM POINT WHERE route_id IN ( SELECT route_id FROM ROUTE WHERE (gpx_id = (SELECT gpx_id FROM FILE WHERE (file_name = '"+req.query.file+"' ) ) ) ORDER BY route_len ASC)");
      var array = rows1;

  }catch(e){
      var array =[];
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      rows: array
    }
  );
});

app.get('/STATUS', async function(req , res){
  const mysql = require('mysql2/promise');
  let connection;
  let check = true;
  
  try{
      // create the connection
      connection = await mysql.createConnection(req.query.dbConf);
      const [files, field1] = await connection.execute("SELECT COUNT(*) AS fCount FROM FILE");
      const [routes, field2] = await connection.execute("SELECT COUNT(*) AS rCount FROM ROUTE");
      const [points, field3] = await connection.execute("SELECT COUNT(*) AS pCount FROM POINT");
      var fileCount = files;
      var routeCount = routes;
      var pointCount = points;

  }catch(e){
      console.log("Query error: "+e);
  }finally {
      if (connection && connection.end) connection.end();
  }
  res.send(
    {
      fileCount: fileCount,
      routeCount: routeCount,
      pointCount: pointCount
    }
  );
});

//Sample endpoint
app.get('/endpoint1', function(req , res){
  let retStr = req.query.data1 + " " + req.query.data2;
  res.send(
    {
      somethingElse: retStr
    }
  );
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);




