var cepObject = {
    cepMessages: function (msg) {
      $('#cep_messages').prepend(msg);
    },
    /*
     * parses the long-polling data stream
     */
    parseDataStream: function(data) {'use strict';

        // Append CEP Message
        cepObject.cepMessages('<div class="alert alert-success" role="alert">Received: '+JSON.stringify(data)+'</div>');

        var i, record, id, type, measurement, object, now, date;
        for (i = 0; i < data.length; i += 1) {
          // ignore the 'success' array element
          if (typeof data[i].data !== 'undefined') {
            record = data[i].data.data;
            console.log(record);

            // parse for measurement channel data
            if (data[i].channel.indexOf('measurements') > -1) {
              type = record.type;
              console.log(type);
              if (type === "c8y_SensorValue") {
                if (typeof record["c8y_TemperatureMeasurement"] !== 'undefined') {
                  cepObject.chartData.setValue(0, 1, record["c8y_TemperatureMeasurement"].T.value);
                  cepObject.chart.draw(cepObject.chartData, cepObject.chartOptions);                  
                }
                else if (typeof record["c8y_SensorValue"] !== 'undefined') {
                  cepObject.chartData.setValue(1, 1, record["c8y_SensorValue"]['Sensor 1'].value);
                  cepObject.chart.draw(cepObject.chartData, cepObject.chartOptions);                  
                }
              }
            }
          }
        }
    },
    /*
     * long-polling to continuously get data - requires the API url and clientId
     */
    longPolling: function(url, clientId) {'use strict';
        var i, data = {
            "channel": "/meta/connect",
            "connectionType": "long-polling",
            "clientId": clientId
        };
        // connect to get messages
        $.ajax({
            url: url,
            type: 'POST',
            headers: { 
                "Authorization": 'Basic xxxxx'
            },
            contentType: 'application/json',
            dataType: 'json',
            data: JSON.stringify(data),
            success: function(data, status, jqXHR) {
                //console.log(data);
                cepObject.parseDataStream(data);
                //cepObject.longPolling(clientId);
            },
            error: function(jqXHR, status, error) {
                console.log(status + ", " + error);
                console.log(jqXHR);
            },
            complete: function() {
                cepObject.longPolling(url, clientId);
            },
            timeout: 30000
        });
    },
    /*
     * sets up the subscription to a particular channel
    *      - requires API url, clientID and channel name to subscribe to
     */
    setupSubscription: function(url, channel, clientId, callback) {'use strict';
        var clientId, headers = { 
            Authorization: 'Basic xxxxx'
        }, data = {
            "channel": "/meta/subscribe",
            "subscription": channel,
            "clientId": clientId
        };
        $.ajax({
            url: url,
            type: 'POST',
            headers: headers,
            contentType: 'application/json',
            dataType: 'json',
            data: JSON.stringify(data),
            success: function(data, status, jqXHR) {
                callback();
            },
            error: function(jqXHR, status, error) {
                console.log(status + ", " + error);
                console.log(jqXHR);
            }
        });
    },
    /*
     * setups the handshake for real time streaming/notifications to get the clientId
     */
    setupHandShake: function(url, callback) {'use strict';
        var obj, clientId, headers = { 
            Authorization: 'Basic xxxxx'
        }, data = {
            "version": "1.0",
            "minimumVersion": "0.9",
            "channel": "/meta/handshake",
            "supportedConnectionTypes": ["long-polling"],
            "advice": {
                "timeout": 60000, "interval": 0
            }
        };
        $.ajax({
            url: url,
            type: 'POST',
            headers: headers,
            contentType: 'application/json',
            dataType: 'json',
            data: JSON.stringify(data),
            success: function(data, status, jqXHR) {
                clientId = data[0].clientId;
                obj = {
                    "clientId": clientId,
                    "url": url
                };
                callback(clientId);
            },
            error: function(jqXHR, status, error) {
                console.log(status + ", " + error);
                console.log(jqXHR);
            }
        });
    },
    /*
     * setups the web sockets for the real time data streams (for both normal 
     * records as well as alarms)
     */
    dataStreamSetup: function() {'use strict';
      var urlRealTime = "https://iotchallengeXXX.telstra-iot.com/cep/realtime",
          channels = [
              "/measurements/*"
          ], i, clientId;

      // handshake
      cepObject.setupHandShake(urlRealTime, function(id) {
          clientId = id;
          // subscribe to the channels
          for (i = 0; i < channels.length; i += 1) {
              cepObject.setupSubscription(urlRealTime, channels[i], clientId, function() {
                  console.log("Success subscribing to channel: " + channels[i]);
              });
          }
          // start long-polling
          cepObject.longPolling(urlRealTime, clientId);
      });
    },
    chartData: google.visualization.arrayToDataTable([
      ['Label', 'Value'],
      ['Temperature', 0],
      ['Sensor 1', 0]
    ]),
    chartOptions: {
      width: 800, height: 240,
      redFrom: 90, redTo: 100,
      yellowFrom:75, yellowTo: 90,
      minorTicks: 5
    },
    chart: new google.visualization.Gauge(document.getElementById('chart_div'))
};

// Begin longpoll setup
cepObject.dataStreamSetup();

// Begin Google Gauge Setup
google.setOnLoadCallback(drawChart);
function drawChart() {
  cepObject.chart.draw(cepObject.chartData, cepObject.chartOptions);
}