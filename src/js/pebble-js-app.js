var API_KEY = '';

var getMetricsFromJamAPI = function(callback) {
  var req = new XMLHttpRequest();

  req.open('GET', 'https://api.hellojam.fr/api/v1/metrics.json', true);
  req.setRequestHeader('X-METRICS-TOKEN', API_KEY);
  req.overrideMimeType('application/json');

  req.onload = function() {
    callback(JSON.parse(this.responseText));
  };

  req.send();
};

var getMetrics = function() {
  getMetricsFromJamAPI(function(metrics) {
    var dictionnary = {
      "KEY_USERS": metrics.usersTotal.current,
      "KEY_REQUESTS": metrics.requestsTotal.current,
      "KEY_MESSAGES": metrics.messagesTotal.current
    };

    Pebble.sendAppMessage(dictionnary, function(e) {
      console.log('Success');
    }, function(e) {
      console.log('Failed');
    });
  });
};

Pebble.addEventListener('ready', function(e) {
  getMetrics();
});

Pebble.addEventListener('appmessage', function(e) {
  getMetrics();
});
