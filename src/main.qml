import QtQuick 2.0
import QtQuick.Window 2.0
import QtLocation 5.6
import QtPositioning 5.6
 
Window {
width: 512
height: 512
visible: true
 
Plugin {
id: mapboxglPlugin
name: "mapboxgl"
//PluginParameter { name: "mapboxgl.mapping.cache.directory"; value: "/root/" }
}

Map {
anchors.fill: parent
plugin: mapboxglPlugin
center: Gps.position.atDistanceAndAzimuth(2000,Gps.bearing) //ps.position //QtPositioning.coordinate(43.162, -77.574) // Oslo
tilt: 45
bearing: Gps.bearing
zoomLevel: 14
}
}
