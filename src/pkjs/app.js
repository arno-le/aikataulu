Pebble.addEventListener('ready',
  function (e) {
    console.log("Pebble JS bereit!");
    Pebble.sendAppMessage({'KEY_JS_READY': 1});
  });

Pebble.addEventListener('appmessage',
  function (e) {
    console.log('AppMessage empfangen!');
    
 var internet_status = navigator.onLine;
   console.log('Sind wir online?' + internet_status);
 /*
    if(!internet_status) {
     Pebble.sendAppMessage({'KEY_INTERNET': 1});
     return;
   }
    */
  // Get the dictionary from the message 
  var dict = e.payload;
  console.log('Got message: ' + JSON.stringify(dict));
    
    var time_to_departure = 15;
    //Some number magic b/c javascript, 000-padding
    //Thanks StackOverflow
    var str = 1;
    var stop1 = ('0000'+str).substring(str.length);
    var stop2 = ('0000'+str).substring(str.length);
    var line1 = 4;
    var line2 = 4;
      if(dict.KEY_STOP1) {
        stop1 = dict.KEY_STOP1;
        console.log('Got stop1: ' + stop1);
      }     
      if(dict.KEY_STOP2) {
        stop2 = dict.KEY_STOP2;
        console.log('Got stop2: ' + stop2);
      }   
      if(dict.KEY_TIME_TO_DEPARTURE) {
        time_to_departure = dict.KEY_TIME_TO_DEPARTURE;
        console.log('Got KEY_TIME_TO_DEPARTURE: ' + time_to_departure);
      }
          if(dict.KEY_LINE1) {
        line1 = dict.KEY_LINE1;
        console.log('Got line1: ' + line1);
      }
      if(dict.KEY_LINE2) {
        line2  = dict.KEY_LINE2;
        console.log('Got line2: ' + line2);
      }
    
    getTimetable(time_to_departure,stop1,stop2,line1,line2);
  });

Pebble.addEventListener('showConfiguration', function() {
  //var url = 'http://127.0.0.1:8080/';
  var url = 'https://www.students.tut.fi/~lehtone2/config/';
  console.log("Opening config");
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData; 
  if(e.response) {
    configData = JSON.parse(decodeURIComponent(e.response));
  } else {
    return;
  }

  console.log('Configuration page returned: ' + JSON.stringify(configData));
  var dictionary = {
        'KEY_TWENTY_FOUR_HOUR_FORMAT': configData.twentyFourHourFormat,
        'KEY_TIME_TO_DEPARTURE': configData.timeToDeparture,
        'KEY_REFRESH_RATE': configData.refreshRate,
        'KEY_STOP1' : configData.stop1,
        'KEY_STOP2' : configData.stop2,
        'KEY_LINE1' : configData.line1,
        'KEY_LINE2' : configData.line2
        };
    Pebble.sendAppMessage(dictionary, 
    function() {console.log('Send successful!');
    }, function() {
      console.log('Send failed!');
    });
});

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.timeout = 5000;
  xhr.ontimeout = function () { sendData('Yhteys-','','ongelma.',''); };
  xhr.send();
};

function getDeparture(data, busnumber) {
  console.log(JSON.stringify(data));
  var patt = new RegExp(busnumber);
  console.log(patt);
  for(var i = 0; i <  9 ; ++i) {
    if(data[i]) {
      console.log(patt.test(data[i].code));
      if( patt.test(data[i].code)) {
        console.log(data[i].code);
        console.log(data[i].time);
        return i;
      }
    }
  }
  return false;
}
//Copyright  Kip @ Stackoverflow, ah the elegance
function addMinutes(date, minutes) {
    return new Date(date.getTime() + minutes*60000);
}

function getTimetable(time_window,stop1,stop2,line1,line2) {
  
  var time1;
  var time2;
  var code1;
  var code2;
  var apiusername = "arnolehtonen";
  var password = "Sk6Q9EwOAp";
  
  //Jetzige Zeit + time_window
  var currentdate = addMinutes(new Date(), time_window);
  var hour_string = currentdate.getHours();
  var h;
  var minute_string = currentdate.getMinutes();
  var m;
  //This as a function
  if(minute_string < 10) {
    m = ('0'+minute_string).substring(minute_string.length);
  } else {
    m = minute_string;
  } 
  
  if(hour_string < 10) {
    h = ('0'+hour_string).substring(hour_string.length);
  } else {
    h = hour_string;
  }
  
  

  var currenttime =  h.toString()+m.toString();
  console.log("Angeforderte Abfahrtszeit:" + currenttime); 

  var url = "http://api.publictransport.tampere.fi/prod/?request=stop&user="+apiusername+"&pass="+password+"&code="+stop1+"& dep_limit=10&time="+currenttime;
  
  //1st stop 
  xhrRequest(url, 'GET',
    function(responseText) {
        if(!responseText) {
        code1 = "No";
        time1 =  "Stop";
        } else {
          var json = JSON.parse(responseText);
          //Die Abfahrtszeit
          if(json[0].departures) {
          var item = getDeparture(json[0].departures,line1);
         if(item === false) {
            time1 = "----";
            code1 = line1;
          } else {
          code1 = json[0].departures[item].code;
          time1 = json[0].departures[item].time;
        
         console.log('Die Abfahrtszeit ' + time1);
        }
      
      }
      }

      
      //2nd stop
      url = "http://api.publictransport.tampere.fi/prod/?request=stop&user="+apiusername+"&pass="+password+"&code="+stop2+"& dep_limit=10&time="+currenttime;
      xhrRequest(url, 'GET',
    function(responseText) {
      if(!responseText) {
        code2 = "No";
        time2 =  "stop";
        } else {
        var json = JSON.parse(responseText);
      
      //Die Abfahrtszeit
      if(json[0].departures) { 
        var item = getDeparture(json[0].departures,line2);
        if(item === false) {
            time2 = "----";
            code2 = line2;
          } else {
          code2 = json[0].departures[item].code;
          time2 = json[0].departures[item].time;
        
         console.log('Die Abfahrtszeit ' + time2);
        }
      
      }
    }
  
      sendData(code1,time1,code2,time2);
  }
  );
});
}

function sendData(code1,time1,code2,time2) {
  
    //Kommunikation mit dem OS
      var dictionary = {
        'KEY_CODE1' : code1,
        'KEY_TIME1': time1,
        'KEY_CODE2' : code2,
        'KEY_TIME2' : time2
        };

    Pebble.sendAppMessage(dictionary,
    function (e) {
      console.log('Daten erfolgreich geschickt.');
      },
    function (e) {
    console.log('Ein Fehler hat aufgetreten!');
      }
    );
}
