let dbConf;
let check = false;

// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(function() {
    // On page-load AJAX Example
    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/endpoint1',   //The server endpoint we are connecting to
        data: {
            data1: "Value 1",
            data2:1234.56
        },
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            jQuery('#blah').html("On page load, received string '"+data.somethingElse+"' from server");
            //We write the object to the console to show that the request was successful

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("On page load, received error from server");
            console.log(error); 
        }
    });

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    


    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getFileTable',   //The server endpoint we are connecting to
        data: {
        },          
        success: function (data) {
            if (data.gpxArray.length == 0) {
                $("#FILE-LOG-PANEL").hide();
                $("#NO-FILES").show();
            }
            else{
                $("#FILE-LOG-PANEL").show();
                $("#NO-FILES").hide();
                buildTable(data.gpxArray, 'fileTable');
                buildMenu(data.gpxArray, 'fileDropDownList');
                buildQueryMenu(data.gpxArray, 'fileList');
                listSelect();
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("something");
            console.log("error"); 
        }
    });

    $("#wayTableDiv").hide();
    $("#findTableDiv").hide();
    $("#blah").hide();
    
    
    $("#compTable").hide();
    $("#changeDiv").hide();
    $("#query").hide();
    $("#rTable").hide();
    $("#wTable").hide();
    $("#dbButtons").hide();
    alert("Please Enter your login");
});

//---------------------------------------------------------------------------------------------

$("#fileDropDownList").on('change', function(){
    console.log("list item selected");
    listSelect()
});

var onClick = function() {
    alert('OtherData');
};

$('#button').click(onClick);



//MAKE FILE TABLE----------------------------------------------------------------------------------------------
function buildTable(data, tableID){
    $("#fileTable tbody").empty();
    var table = document.getElementById(tableID);

    for (var i = 0; i < data.length; i++) {
        var row = `<tr>
                        <td><a href="uploads/${data[i].name}">${data[i].name}</a></td>
                        <td>${data[i].version}</td>
                        <td>${data[i].creator}</td>
                        <td>${data[i].numWaypoints}</td>
                        <td>${data[i].numRoutes}</td>
                        <td>${data[i].numTracks}</td>
                   </tr>`
        table.innerHTML += row;
    }
    document.getElementById(tableID).classList = "table table-striped table-condensed table-bordered table-nonfluid";
    $('#'+tableID+' tr:odd').css('background-color', 'light grey'); 
    $('#'+tableID+' tr:even').css('background-color', 'white'); 
}

//MAKE COMPONENT TABLE----------------------------------------------------------------------------------------------
function buildComponentTable(data1, data2, tableID){
    
    var table = document.getElementById(tableID);
    var row;
    var name;
    for (var i = 0; i < data1.length; i++) {
        name = "Route " + (i+1).toString();
        buttonName = "Route-" + (i+1).toString();
        
        row = `<tr>
                        <td><button type="button" class="components">${name}</button></td>
                        <td><button type="button" name=${buttonName} class="compNames">${data1[i].name}</button></td>
                        <td>${data1[i].numPoints}</td>
                        <td>${data1[i].len}</td>
                        <td>${data1[i].loop}</td>
                   </tr>`
        table.innerHTML += row;
    }
    for (var i = 0; i < data2.length; i++) {
        name = "Track " + (i+1).toString();
        buttonName = "Track-" + (i+1).toString();
        row = `<tr>
                        <td><button type="button" class="components">${name}</button></td>
                        <td><button type="button" name=${buttonName} class="compNames">${data2[i].name}</button></td>
                        <td>${data2[i].numPoints}</td>
                        <td>${data2[i].len}</td>
                        <td>${data2[i].loop}</td>
                   </tr>`
        table.innerHTML += row;
    }
    const wrapper = document.getElementById('componentTable');

    //add event listeners to component names----------------------------------------------------------------------
    let btns = document.querySelectorAll(".components");
    for (i of btns) {
        i.addEventListener('click', function() {
            var str = this.textContent.split(" ");
            jQuery.ajax({
                type: 'get',            //Request type
                dataType: 'json',       //Data type - we will use JSON for almost everything 
                url: '/getComponentData',   //The server endpoint we are connecting to
                data: {
                    fileName: document.getElementById("fileDropDownList").value,
                    option: str[0],
                    index: str[1]
                },        
                success: function (data) {
                    $('#blah').html(document.getElementById("fileDropDownList").value);
                    if (data.dataArray.length == 0) {
                        alert("NO OTHER DATA");
                    }
                    else{
                        var string = new String();
                        for (let i = 0; i < data.dataArray.length; i++) {
                            string += data.dataArray[i].name+" = "+data.dataArray[i].value+"\n";
                            
                        }
                        alert(string);
                        $('#blah').html(document.getElementById("fileDropDownList").value);
                    }
                },
                fail: function(error) {
                    // Non-200 return, do something with error
                    $('#blah').html("something");
                    console.log("error"); 
                    
                }
            });
            
        });
    }
    let but = document.querySelectorAll(".compNames");
    for (i of but) {
        i.addEventListener('click', function() {
            document.getElementById('currentName').value = this.textContent;
            document.getElementById('currentName').name = this.name;
        });
    }

    document.getElementById(tableID).classList = "table table-striped table-condensed table-bordered table-nonfluid";
    $('#'+tableID+' tr:odd').css('background-color', 'light grey'); 
    $('#'+tableID+' tr:even').css('background-color', 'white'); 
}



$('#nameChange').submit(function(e){
    let choiceStr =  document.getElementById('currentName').name;
    choiceStr = choiceStr.split('-');
    $('#blah').html("Form has data: "+$('#newName').val());
    e.preventDefault();
    //Pass data to the Ajax call, so it gets passed to the server
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/changeComponentName',   //The server endpoint we are connecting to
        data: {
            fileName: document.getElementById("fileDropDownList").value,
            newName: $('#newName').val(),
            component: choiceStr[0],
            index: choiceStr[1]
        },                
        success: function (data) {
            $('#blah').html(data.dataString);
            if (data.dataString.length == 0) {
                $('#blah').html("NO FILES");
            }
            else{
                jQuery.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/getFileTable',   //The server endpoint we are connecting to
                    data: {
                    },          
                    success: function (data) {
                        if (data.gpxArray.length == 0) {
                            $("#NO-FILES").show();
                            $("#FILE-LOG-PANEL").hide();
                        }
                        else{
                            $("#FILE-LOG-PANEL").show();
                            $("#NO-FILES").hide();
                            buildTable(data.gpxArray, 'fileTable');
                            listSelect();
                            $("#fileDropDownList").val(document.getElementById("fileDropDownList").value).change();
                        }
            
                    },
                    fail: function(error) {
                        // Non-200 return, do something with error
                        $('#blah').html("something");
                        console.log("error"); 
                    }
                });
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("something");
            console.log("error"); 
        }
    });
    document.getElementById("currentName").value = "";
    document.getElementById("newName").value = "";
    
    if (check == true) {
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/FILETABLE',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf
            },        
            success: function (data) {
            },
            fail: function(error) {
            }
        });
    }
});

function buildMenu(data, menuID){
    $('#fileDropDownList').empty();
    var menu = document.getElementById(menuID);

    for (var i = 0; i < data.length; i++) {
        document.getElementById(menuID).innerHTML += '<option>' + data[i].name + '</option>';
    }
}

function buildQueryMenu(data, menuID){
    $('#fileList').empty();
    var menu = document.getElementById(menuID);

    for (var i = 0; i < data.length; i++) {
        document.getElementById(menuID).innerHTML += '<option>' + data[i].name + '</option>';
    }
}

//MAKE LIST ----------------------------------------------------------------------------------------------
function listSelect() {
    console.log(document.getElementById("fileDropDownList").value);
    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getFileComponents',   //The server endpoint we are connecting to
        data: {
            fileName: document.getElementById("fileDropDownList").value
        },        
        success: function (data) {
            $('#blah').html(document.getElementById("fileDropDownList").value);
            if (data.routeArray.length == 0 && data.trackArray.length == 0) {
                $("#componentTable tbody").empty();
                $("#compTable").hide();
                $("#changeDiv").hide();
            }
            else{
                $('#blah').html(document.getElementById("fileDropDownList").value);
                $("#componentTable tbody").empty();
                if (data.routeArray.length > 0 || data.trackArray.length > 0) {
                    $("#componentTable tbody").empty();
                    $("#compTable").show();
                    $("#changeDiv").show();
                    buildComponentTable(data.routeArray, data.trackArray, 'componentTable');
                }
                else{
                    $("#componentTable tbody").empty();
                    $("#compTable").hide();
                    $("#changeDiv").hide();
                }
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("something");
            console.log("error"); 
        }
    });
}

//CREATE GPX---------------------------------------------------------------------------
document.getElementById("button1").onclick = function(e){

    if($('#file').val() == "" || $('#version').val()  == "" || $('#creator').val()  == "")
    {
        alert("ERROR: empty fields");
    }
    else{
        var string = make_json(document.getElementById("data"));
        var gpxJson = JSON.parse(string);
        $('#blah').html("json = "+string);
        var exists = false;
        for(var i = 0, opts = document.getElementById('fileDropDownList').options; i < opts.length; ++i){
            if( opts[i].value === gpxJson.file )
            {
                alert("ERROR: FILE WITH NAME ALREADY EXISTS");
                exists = true; 
                break;
            }
        }
    
        if (exists == false) {
            $('#blah').html("field = ");
            
            
            if (isNaN(gpxJson.version)) {
                $('#blah').html("fileName = "+gpxJson.file+" version = "+gpxJson.version+" creator = "+gpxJson.creator);
                alert("ERROR: not a valid version");
            }
            else{
                var sendJSON = new Object();
                sendJSON.version = parseFloat(gpxJson.version);
                sendJSON.creator = gpxJson.creator;
                var sendString = JSON.stringify(sendJSON);
                $('#blah').html(sendString);

                jQuery.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/createGPX',   //The server endpoint we are connecting to
                    data: {
                        fileName: gpxJson.file,
                        jsonString: sendString
                    },        
                    success: function (data) {
                        $('#blah').html(document.getElementById("fileDropDownList").value);
                        if (!data.dataString.localeCompare("FALSE")) {
                            alert("ERROR: GPX could not be added");
                        }
                        else{
                            jQuery.ajax({
                                type: 'get',            //Request type
                                dataType: 'json',       //Data type - we will use JSON for almost everything 
                                url: '/getFileTable',   //The server endpoint we are connecting to
                                data: {
                                },          
                                success: function (data) {
                                    if (data.gpxArray.length == 0) {
                                        $('#blah').html("NO FILES");
                                    }
                                    else{
                                        buildTable(data.gpxArray, 'fileTable');
                                        buildMenu(data.gpxArray, 'fileDropDownList');
                                        buildQueryMenu(data.gpxArray, 'fileList');
                                        listSelect();
                                    } 
                        
                                },
                                fail: function(error) {
                                    // Non-200 return, do something with error
                                    $('#blah').html("something");
                                    console.log("error"); 
                                }
                            });
                        }
                    },
                    fail: function(error) {
                        // Non-200 return, do something with error
                        $('#blah').html("something");
                        console.log("error"); 
                    }
                });
            }
    
        }
    }
    document.getElementById("file").value = "";
    document.getElementById("version").value = "";
    document.getElementById("creator").value = "";
}
    
    
    function make_json(form){
        var json={};
        
        var elements = form.elements;
        
        for (var i = 0, element; element = elements[i++];) {
        if (element.type == "text" && element.value != ""){
            json[element.name] = element.value;
            }
        }
        
        var html = JSON.stringify(json,null,4);
        return html;
    }

//add route---------------------------------------------------------------------------

var latLonList = new Array();
document.getElementById("button2").onclick = function(e){
    var jsonString = make_json(document.getElementById("routeData"));
    var json = JSON.parse(jsonString);
    obj = new Object();
    obj.lat = parseFloat(json.lat);
    obj.lon = parseFloat(json.lon);
    if (isNaN(obj.lat) || isNaN(obj.lon)) {
        alert("ERROR: latitude must be between -90 and 90 and longitude must be between -180 and 180");
    }
    else{
        if (obj.lat <= -90.0 || obj.lat >= 90.0 || obj.lon <= -180.0 || obj.lon >= 180.0 ) {
            alert("ERROR: latitude must be between -90 and 90 and longitude must be between -180 and 180");
        }
        else{
            latLonList.push(obj);
            jsonString = JSON.stringify(latLonList);
            $('#blah').html("convert = "+jsonString);
            buildLatLonTable(latLonList, "waypointTable");
            $("#wayTableDiv").show();
        }
    }
    document.getElementById("lat").value = "";
    document.getElementById("lon").value = "";
}

document.getElementById("button4").onclick = function(e){
    $("#wayTableDiv").hide();
    $("#waypointTable tbody").empty();
    latLonList.length = 0;
    document.getElementById("lat").value = "";
    document.getElementById("lon").value = "";
}

document.getElementById("button3").onclick = function(e){

    var fileName = document.getElementById("fileDropDownList").value;
    var jsonString = JSON.stringify(latLonList);
    
    var routeName = document.getElementById("name").value;

    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/addRoute',   //The server endpoint we are connecting to
        data: {
            fileName: fileName,
            jsonString: jsonString,
            routeName: routeName
        },        
        success: function (data) {
            $('#blah').html(document.getElementById("fileDropDownList").value);
            if (!data.dataString.localeCompare("FALSE")) {
                alert("ERROR: Route could not be added");
            }
            else{
                jQuery.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/getFileTable',   //The server endpoint we are connecting to
                    data: {
                    },          
                    success: function (data) {
                        if (data.gpxArray.length == 0) {
                            $('#blah').html("NO FILES");
                        }
                        else{
                            buildTable(data.gpxArray, 'fileTable');
                            buildMenu(data.gpxArray, 'fileDropDownList');
                            buildQueryMenu(data.gpxArray, 'fileList');
                            listSelect();
                        }
                    },
                    fail: function(error) {
                        // Non-200 return, do something with error
                        $('#blah').html("something");
                        console.log("error"); 
                    }
                });
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("something");
            console.log("error"); 
        }
    });
    $("#wayTableDiv").hide();
    $("#waypointTable tbody").empty();
    latLonList.length = 0;
    document.getElementById("name").value = "";
    document.getElementById("lat").value = "";
    document.getElementById("lon").value = "";
    if (check == true) {
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/FILETABLE',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf
            },        
            success: function (data) {
            },
            fail: function(error) {
            }
        });
    }
}

function buildLatLonTable(data, tableID){
    $("#waypointTable tbody").empty();
    var table = document.getElementById(tableID);

    for (var i = 0; i < data.length; i++) {
        var row = `<tr>
                        <td>${data[i].lat}</td>
                        <td>${data[i].lon}</td>
                   </tr>`
        table.innerHTML += row;
    }
    document.getElementById(tableID).classList = "table table-striped table-condensed table-bordered table-nonfluid";
    $('#'+tableID+' tr:odd').css('background-color', 'light grey'); 
    $('#'+tableID+' tr:even').css('background-color', 'white'); 
}  

//FIND PATH---------------------------------------------------------------------------

document.getElementById("button5").onclick = function(e){
    if($('#sourceLat').val() == "" || $('#sourceLon').val()  == "" || $('#destLat').val()  == "" || $('#destLon').val()  == "" || $('#tol').val()  == "")
    {
        alert("ERROR: empty fields");
    }
    else
    {
        var jsonString = make_json(document.getElementById("pathBetween"));
        var json = JSON.parse(jsonString);
        var ok = new Object();
        $('#blah').html(parseFloat(json.sourceLat)+" "+parseFloat(json.sourceLon)+" "+parseFloat(json.destLon)+" "+parseFloat(json.destLat)+" "+parseFloat(json.tol));
        if (isNaN(json.sourceLat) || isNaN(json.sourceLon) || isNaN(json.destLat) || isNaN(json.destLon) || isNaN(json.tol)) {
            alert("ERROR: latitude must be between -90 and 90 and longitude must be between -180 and 180");
        }
        else{
            if (parseFloat(json.sourceLat) <= -90.0 || parseFloat(json.sourceLat) >= 90.0 || parseFloat(json.sourceLon) <= -180.0 || parseFloat(json.sourceLon) >= 180.0 || parseFloat(json.destLat) <= -90.0 || parseFloat(json.destLat) >= 90.0 || parseFloat(json.destLon) <= -180.0 || parseFloat(json.destLon) >= 180.0) {
                alert("ERROR: latitude must be between -90 and 90 and longitude must be between -180 and 180");
            }
            else{
                $("#findTableDiv").show();

                var fileName = document.getElementById("fileDropDownList").value;
                var jsonString = JSON.stringify(latLonList);
                var routeName = document.getElementById("name").value;
                var sorLat = parseFloat(json.sourceLat);
                var sorLon = parseFloat(json.sourceLon);
                var desLat = parseFloat(json.destLat);
                var desLon = parseFloat(json.destLon);
                var tol = parseFloat(json.tol);
                $('#blah').html("something"+parseFloat(json.sourceLat)+" "+parseFloat(json.sourceLon)+" "+parseFloat(json.destLon)+" "+parseFloat(json.destLat)+" "+parseFloat(json.tol));
                jQuery.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/getBetween',   //The server endpoint we are connecting to
                    data: {
                        fileName: fileName,
                        SourceLat: sorLat,
                        SourceLon: sorLon,
                        DestLat: desLat,
                        DestLon: desLon,
                        Tol: tol
                    },        
                    success: function (data) {
                        $("#findTable tbody").empty();
                        if (data.routeArray.length > 0 || data.trackArray.length > 0) {
                            $("#findTableDiv").show();
                            buildComponentTable(data.routeArray, data.trackArray, "findTable");
                        }
                        else{
                            $("#findTableDiv").hide();
                            alert("No Matching Paths");
                        }
                        

                        
                        $('#blah').html(document.getElementById("fileDropDownList").value);
     
                    },
                    fail: function(error) {
                        $("#findTableDiv").hide();
                        // Non-200 return, do something with error
                        alert("No Matching Paths");
                    }
                });
            }
        }


    }
    
    
    document.getElementById("sourceLat").value = "";
    document.getElementById("sourceLon").value = "";
    document.getElementById("destLat").value = "";
    document.getElementById("destLon").value = "";
    document.getElementById("tol").value = "";

}

//Login---------------------------------------------------------------------------

document.getElementById("loginEnter").onclick = async function(e){
    if($('#username').val() == "" || $('#password').val()  == "" || $('#dbname').val()  == "")
    {
        alert("ERROR: empty fields");
    }
    else
    {
        var jsonString = make_json(document.getElementById("login"));
        var json = JSON.parse(jsonString);
        dbConf = {
            host     : 'dursley.socs.uoguelph.ca',
            user     : json.username,
            password : json.password,
            database : json.DBname
        };
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/loginCheck',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf
            },        
            success: function (data) {
                if (data.check == true) {

                    check = true;
                    alert("Connection established");
                    $("#loginPanel").hide();
                    var tbl = document.getElementById('fileTable');
                    if (tbl.rows.length > 0) {
                        $("#dbButtons").show();
                    }
                }
                else{
                    alert("Connection could not be established. Please re-enter username, password, and DB name again");
                }
            },
            fail: function(error) {
                alert("Connection could not be established. Please re-enter username, password, and DB name again");
            }
        });
    }
    document.getElementById("username").value = "";
    document.getElementById("password").value = "";
    document.getElementById("DBname").value = "";
}

//Store Files---------------------------------------------------------------------------

document.getElementById("storeFiles").onclick = async function(e){

        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/FILETABLE',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf
            },        
            success: function (data) {
                if (data.check == true) {
                    $("#query").show();
                    alert("Files Saved to DB Successfully");
                }
                else{
                    alert("Files not saved to DB");
                }
            },
            fail: function(error) {
                alert("Files not saved to DB");
            }
        });
        
}

document.getElementById("clearFiles").onclick = async function(e){

    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/CLEAR',   //The server endpoint we are connecting to
        data: {
            dbConf: dbConf
        },        
        success: function (data) {
            
        },
        fail: function(error) {
            alert("Files not saved to DB");
        }
    });
    $("#query").hide();
}

document.getElementById("status").onclick = async function(e){

    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/STATUS',   //The server endpoint we are connecting to
        data: {
            dbConf: dbConf
        },        
        success: function (data) {
            console.log(data.fileCount[0]);
            alert("Database has "+data.fileCount[0].fCount+" files, "+data.routeCount[0].rCount+" routes, and "+data.pointCount[0].pCount+" points");
        },
        fail: function(error) {
        }
    });
}

//Route Len---------------------------------------------------------------------------




function buildRouteTable(data, tableID){
    $("#routeTable tbody").empty();
    var table = document.getElementById(tableID);

    for (var i = 0; i < data.length; i++) {
        var row = `<tr>
                        <td>${data[i].route_id}</td>
                        <td>${data[i].route_name}</td>
                        <td>${data[i].route_len}</td>
                        <td>${data[i].gpx_id}</td>
                   </tr>`
        table.innerHTML += row;
    }
    document.getElementById(tableID).classList = "table table-striped table-condensed table-bordered table-nonfluid";
    $('#'+tableID+' tr:odd').css('background-color', 'light grey'); 
    $('#'+tableID+' tr:even').css('background-color', 'white'); 
}

//File Route Len---------------------------------------------------------------------------





//EXECUTE----------------------------------------------------------------------------
document.getElementById("execute").onclick = async function(e){
    console.log(document.getElementById("queries").value);
    $("#rTable").hide();
    $("#wTable").hide();
    if (!document.getElementById("queries").value.localeCompare("RRL")) {
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/routeLen',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf
            },        
            success: function (data) {
                if (data.rows.length > 0) {
                    buildRouteTable(data.rows, 'routeTable');
                    $("#rTable").show();
                }
            },
            fail: function(error) {
            }
        });

    }
    else if (!document.getElementById("queries").value.localeCompare("RRN")) {
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/routeName',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf
            },        
            success: function (data) {
                console.log("retrieved routes by name");
                if (data.rows.length > 0) {
                    $("#rTable").show();
                    buildRouteTable(data.rows, 'routeTable');
                }
            },
            fail: function(error) {
            }
        });

    }
    else if (!document.getElementById("queries").value.localeCompare("RFL")) {
            jQuery.ajax({
                type: 'get',            //Request type
                dataType: 'json',       //Data type - we will use JSON for almost everything 
                url: '/fileRouteLen',   //The server endpoint we are connecting to
                data: {
                    dbConf: dbConf,
                    file: document.getElementById("fileList").value
                },        
                success: function (data) {
                    if (data.rows.length > 0) {
                        $("#rTable").show();
                        buildRouteTable(data.rows, 'routeTable');
                    }
                },
                fail: function(error) {
                }
            });
    }
    else if (!document.getElementById("queries").value.localeCompare("RFN")) {
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/fileRouteName',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf,
                file: document.getElementById("fileList").value
            },        
            success: function (data) {
                if (data.rows.length > 0) {
                    $("#rTable").show();
                    buildRouteTable(data.rows, 'routeTable');
                }
            },
            fail: function(error) {
            }
        });
    }
    else if (!document.getElementById("queries").value.localeCompare("PFL")) {
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/filePointsLen',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf,
                file: document.getElementById("fileList").value
            },        
            success: function (data) {
                if (data.rows.length > 0) {
                    $("#wTable").show();
                    buildWayTable(data.rows, 'routepointTable');
                }
            },
            fail: function(error) {
            }
        });
    }
    else if (!document.getElementById("queries").value.localeCompare("PFN")) {
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/filePointsName',   //The server endpoint we are connecting to
            data: {
                dbConf: dbConf,
                file: document.getElementById("fileList").value
            },        
            success: function (data) {
                if (data.rows.length > 0) {
                    $("#wTable").show();
                    buildWayTable(data.rows, 'routepointTable');
                }
            },
            fail: function(error) {
            }
        });
    }
    else if (!document.getElementById("queries").value.localeCompare("PRI")) {
        if($('#routeID').val() == "")
        {
            alert("ERROR: empty fields");
        }
        else
        {
            var jsonString = make_json(document.getElementById("routeInfo"));
            var json = JSON.parse(jsonString);
            var ok = new Object();
            if (isNaN(json.routeID)) {
                alert("ERROR: Route ID must be a positive integer");
            }
            else{
                jQuery.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/routePoints',   //The server endpoint we are connecting to
                    data: {
                        dbConf: dbConf,
                        index: json.routeID
                    },        
                    success: function (data) {
                        console.log("retrieved routes by name");
                        if (data.rows.length > 0) {
                            $("#wTable").show();
                            buildWayTable(data.rows, 'routepointTable');
                        }
                    },
                    fail: function(error) {
                    }
                });
            }
        }
        document.getElementById("routeID").value = "";
    }
}


function buildWayTable(data, tableID){
    $("#routepointTable tbody").empty();
    var table = document.getElementById(tableID);

    for (var i = 0; i < data.length; i++) {
        console.log(data[i].point_name);
        if (!data[i].point_name) {
            var name = "UNNAMED"
        }
        else{
            var name = data[i].point_name;
        }
        var row = `<tr>
                        <td>${data[i].point_id}</td>
                        <td>${data[i].point_index}</td>
                        <td>${data[i].latitude}</td>
                        <td>${data[i].longitude}</td>
                        <td>${data[i].route_id}</td>
                        <td>${name}</td>
                   </tr>`
        table.innerHTML += row;
    }
    document.getElementById(tableID).classList = "table table-striped table-condensed table-bordered table-nonfluid";
    $('#'+tableID+' tr:odd').css('background-color', 'light grey'); 
    $('#'+tableID+' tr:even').css('background-color', 'white'); 
}