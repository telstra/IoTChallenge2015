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

            if (data[i].channel.indexOf('managedobjects') > -1) {

              type = record.type;

              if (type === "actility") {
                if (typeof record["c8y_Position"] !== 'undefined') {
                  // Update Map
                  googleMaps.deleteMarkers();
                  googleMaps.myLatLng['lat'] = record["c8y_Position"]['lat'];
                  googleMaps.myLatLng['lng'] = record["c8y_Position"]['lng'];
                  googleMaps.markers[0] = googleMaps.setMarker(record["owner"], i, record["c8y_Position"]['lat'], record["c8y_Position"]['lng']);
                  googleMaps.map.setCenter(googleMaps.myLatLng);
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
              "/managedobjects/*"
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
    }
};

// Begin longpoll setup
cepObject.dataStreamSetup();

// Setup Maps
var googleMaps = {
  map: 0,
  markers: [],
  markerIcon: "http://icons.iconarchive.com/icons/paomedia/small-n-flat/1024/map-marker-icon.png",  
  /*
   * Default Map Center
   */
  myLatLng: {
    lat: -37.818822, 
    lng: 144.953949,
    radius: 260
  },
  // Sets the map on all markers in the array.
  setMapOnAll: function(map) {'use strict';
    for (var i = 0; i < googleMaps.markers.length; i++) {
      googleMaps.markers[i].setMap(map);
    }
  },
  // Removes the markers from the map, but keeps them in the array.
  clearMarkers: function() {'use strict';
    googleMaps.setMapOnAll(null);
  },
  // Deletes all markers in the array by removing references to them.
  deleteMarkers: function() {'use strict';
    googleMaps.clearMarkers();
    googleMaps.markers = [];
  },
  /*
   * creates a marker for the map and returns it
   * by default, the marker does NOT show on the map
   */
  setMarker: function(name, number, lat, lng) {'use strict';
      var image, size, marker;
      size = Math.round(Math.pow(2,googleMaps.map.getZoom()/2.9));
      image = {
          url: googleMaps.markerIcon,
          size: new google.maps.Size(size, size), //size
          scaledSize: new google.maps.Size(size, size) // scaled size (required for Retina display icon)
      };
      if (typeof number === null) {
          marker = new google.maps.Marker({
              position: new google.maps.LatLng(lat,lng),
              map: googleMaps.map,
              draggable: true,
              title: name,
              icon: image
          });
      } else {
          marker = new google.maps.Marker({
              position: new google.maps.LatLng(lat,lng),
              map: googleMaps.map,
              draggable: true,
              title: name,
              number: number,
              icon: image
          });
      }
      marker.setDraggable(false);
      $('#map-canvas div.gmnoprint[alarm="true"]').css({
          "border-radius" : size + "px",
      });
      return marker;
  },  
  /*
   * creates the google map
   */
  init: function() {'use strict';
    var element, options;
    element = $('#map-canvas')[0];
    options = {
      center: googleMaps.myLatLng,
      zoom: 15,
      mapTypeId: google.maps.MapTypeId.ROADMAP
    };

    googleMaps.map = new google.maps.Map(element, options);

    // update map when window is resized
    google.maps.event.trigger(googleMaps.map, 'resize');
  }
};