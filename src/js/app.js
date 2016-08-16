var liveconfig = require('./liveconfig');

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var customClay = require('./custom-clay');
var clay = new Clay(clayConfig, customClay, { autoHandleEvents: false });


Pebble.addEventListener("showConfiguration", function(e) {
  
    liveconfig.connect(Pebble.getAccountToken(), function(id, value) {
    var config = {};
    config[id] = value;
    console.log(JSON.stringify(config));
    Pebble.sendAppMessage(Clay.prepareSettingsForAppMessage(config));
  });
      
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener("webviewclosed", function(e) {
	if (e && !e.response) {
    return;
  }

  var dict = clay.getSettings(e.response);
  console.log('Configuration page returned: ' + JSON.stringify(dict));
  
	Pebble.sendAppMessage(dict, function(e)
	{
		console.log("Success sending to Pebble!");
	}, function() {
		console.log("Error sending to Pebble!");
    console.log(JSON.stringify(e));
	});

});

//Weather

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + "&APPID=917722cd09b1c481c18b22259de92f88";

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);    

      var temperature;
      var temperature_high;
      var temperature_low;

      var location = json.name;
      console.log("Location is " + location);
         
      // Temperature in Kelvin requires adjustment
      temperature = Math.round(json.main.temp - 273.15);
      temperature_high = Math.round(json.main.temp_max - 273.15);
      temperature_low = Math.round(json.main.temp_min - 273.15);
   
      console.log("Temperature in C is " + temperature);
      console.log("High temp in C is " + temperature_high);
      console.log("Low temp in C is " + temperature_low); 
      
      console.log("Temperature in F is " + Math.round(temperature * 9/5 + 32));
      console.log("High temp in F is " + Math.round(temperature_high * 9/5 + 32)); 
      console.log("Low temp in F is " + Math.round(temperature_low * 9/5 + 32));
      
      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "LOCATION": location,
        "TEMPERATURE": temperature,
        "TEMPERATURE_HIGH": temperature_high,
        "TEMPERATURE_LOW": temperature_low,
        "CONDITIONS": conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}


// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }                     
);

//Configuration

Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
    
    // Get the initial weather
    getWeather();
  }
);
