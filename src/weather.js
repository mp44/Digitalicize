var myAPIKey = 'eb3996112cfad299f3da92308d0598ba';

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
      pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var ctemperature = Math.round(json.main.temp - 273.15);
      console.log("Temperature in Celsius is " + ctemperature);

      var temperature = (Math.round(json.main.temp - 273.15)* 1.8 + 32);
      console.log("Temperature in Fahrenheit is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions,
        "KEY_CELSIUS": ctemperature
      };
      console.log("Weather is " + dictionary);

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

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    //getWeather();
  }                     
);

// Configuration
Pebble.addEventListener('showConfiguration', function() {
	var url = 'http://mp44.github.io/digitalicize-config/';

	console.log('Showing configuration page: ' + url);

	Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
	var configData = JSON.parse(decodeURIComponent(e.response));
  var useCelsius = 1;
	console.log('Configuration page returned: ' + JSON.stringify(configData));
  console.log(configData.useCelsius);
	console.log('Sending data to Pebble.');
  if(configData.useCelsius){
    Pebble.sendAppMessage({"KEY_USE_CELSIUS": useCelsius}, 
    function() {
		console.log('Send successful!');
    
	}, function() {
		console.log('Send failed!');
	});
  }
  else{
	Pebble.sendAppMessage({"KEY_USE_CELSIUS": !useCelsius}, 
    function() {
		console.log('Send successful!');
    
	}, function() {
		console.log('Send failed!');
	});
  }
});
