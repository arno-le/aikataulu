(function() {
  constructLineDropdown();
  window.setTimeout(loadOptions,200);
  submitHandler();
})();

function constructLineDropdown() {
	$.getJSON("lines.json", function(json) {
	
	$.each(json[0], function(i, obj) {	
			var HTML = "";
			HTML+= "<option class = \"item-select-option\">"+obj+"</option>";
			$("#lineCodeOne").append(HTML);
			$("#lineCodeTwo").append(HTML);
	});
		
	});
	
}
function submitHandler() {
  var $submitButton = $('#submitButton');

  $submitButton.on('click', function() {
    console.log('Submit');
	
    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
  });
}

function loadOptions() {

  var $timeFormatCheckbox = $('#timeFormatCheckbox');
  
  if (localStorage.twentyFourHourFormat) {
	console.log("Accessing localStorage");
    $timeFormatCheckbox[0].checked = localStorage.twentyFourHourFormat === 'true';
	document.getElementById('timeToDeparture').value = localStorage.timeToDeparture;
	document.getElementById('refreshRate').value = localStorage.refreshRate;
	document.getElementById('stop-input1').value = localStorage.stop1;
	document.getElementById('stop-input2').value = localStorage.stop2;
	document.getElementById('lineCodeOne').value = localStorage.line1;
	document.getElementById('lineCodeTwo').value = localStorage.line2;
  }
}

function getAndStoreConfigData() {
 
 var $timeFormatCheckbox = $('#timeFormatCheckbox');

  var options = {
    twentyFourHourFormat: $timeFormatCheckbox[0].checked,
	timeToDeparture: document.getElementById('timeToDeparture').options[timeToDeparture.selectedIndex].text,
	refreshRate: document.getElementById('refreshRate').options[refreshRate.selectedIndex].text,
	stop1 : document.getElementById('stop-input1').value,
	stop2 : document.getElementById('stop-input2').value,
	line1 : document.getElementById('lineCodeOne').value,
	line2 : document.getElementById('lineCodeTwo').value
  };

  localStorage.twentyFourHourFormat = options.twentyFourHourFormat;
  localStorage.timeToDeparture = options.timeToDeparture;
  localStorage.stop1 = options.stop1;
  localStorage.stop2 = options.stop2;
  localStorage.refreshRate = options.refreshRate;
  localStorage.line1 = options.line1;
  localStorage.line2 = options.line2;
  
  console.log('Got options: ' + JSON.stringify(options));
  return options;
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}
