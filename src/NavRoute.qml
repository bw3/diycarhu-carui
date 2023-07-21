import QtQuick 2.0
import QtPositioning 5.2
import "js/polyline.js" as Polyline
import "js/kdTree.js" as KdTree

QtObject {
    property var waypoints: []
    property var valhallaRouteResp: {}
    property var valhallaRouteLoading: false
    property var valhallaRouteReqTime: 0
    property var routePolylines: []
    onWaypointsChanged: recalc_route()
    onValhallaRouteRespChanged: display_route()
    property var gpsPos: Gps.position
    onGpsPosChanged: update()
    property var distanceLegStr: ""
    property var timeLegStr: ""
    property var timeLegSec: 0

    function setDestination(lat,lon) {
        waypoints = [QtPositioning.coordinate(lat, lon)]
    }

    function recalc_route() {
        valhallaRouteLoading = true;
        let valhalla_waypoints = [{"lat":Gps.position.latitude, "lon":Gps.position.longitude, "heading":Gps.bearing}];
        waypoints.forEach(function(item, index, array) {
            valhalla_waypoints.push({"lat":item.latitude, "lon":item.longitude});
        });
        let valhalla_route_params =  {
            "locations":valhalla_waypoints,
            "costing":"auto",
            "costing_options":{"auto":{"country_crossing_penalty":2000.0}},
            "directions_options":{"units":"km"}
        };
        
        let valhalla_route_req = new XMLHttpRequest();
        valhallaRouteReqTime = (new Date()).getTime(); //TODO: Monotonically increasing
        valhalla_route_req.open("GET",'http://localhost:8553/v2/route?json=' + JSON.stringify(valhalla_route_params));
        valhalla_route_req.onload = function()  {
                console.log(valhalla_route_req.responseText);
                let resp = JSON.parse(valhalla_route_req.responseText)
                resp["coords"] = []
                resp["trip"]["legs"].forEach(function(item, leg_idx, array) {
                    let leg_coords = Polyline.polyline.decode(item["shape"],6);
                    let maneuver_idx = 0;
                    let maneuver_distance = 0;
                    leg_coords.forEach(function(coord, coord_idx, array) {
                        coord.push(leg_idx)
                        if(coord_idx == item["maneuvers"][maneuver_idx]["end_shape_index"]) {
                            maneuver_idx++;
                            maneuver_distance = 0;
                        }
                        coord.push(maneuver_idx);
                        coord.push(maneuver_distance);
                        if( coord_idx < leg_coords.length-1 ) {
                            let next_coord = leg_coords[coord_idx+1];
                            maneuver_distance += QtPositioning.coordinate(coord[0], coord[1]).distanceTo(QtPositioning.coordinate(next_coord[0], next_coord[1]));
                        }
                        resp["coords"].push(coord);
                    });
                });
                console.log(resp["coords"].length);
                resp["coord_idx"] = 0;
                valhallaRouteResp = resp;
                valhallaRouteLoading = false;
        };
        valhalla_route_req.onerror = function()  {
            valhallaRouteLoading = false;
        };
        valhalla_route_req.ontimeout = function()  {
            valhallaRouteLoading = false;
        };

        valhalla_route_req.send();
    }

    function display_route() {
        let polyline = Qt.createQmlObject('import QtLocation 5.3; MapPolyline {}', map)
        polyline.line.width=5;
        polyline.line.color="lime"
        valhallaRouteResp["coords"].forEach(function(coord, index, array) {
            polyline.addCoordinate(QtPositioning.coordinate(coord[0],coord[1]));
        });
        map.addMapItem(polyline);
        routePolylines.forEach(function(item, index) {
            map.removeMapItem(item);
        });
        routePolylines = [polyline]
    }

    function update() {
        if(waypoints.length > 0) {
            updateValhalla();
        }
    }

    function distanceFromLineSegment(segStart,segEnd,point) {
        const R = 6371e3;
        const DEG_RAD = 0.01745329;
        const ang_dist = segStart.distanceTo(point) / R;
        const seg_bearing = segStart.azimuthTo(segEnd) * DEG_RAD;
        const point_bearing = segStart.azimuthTo(point) * DEG_RAD;
        const point_bearing_end = segEnd.azimuthTo(point) * DEG_RAD;
        const cross_track = Math.asin(Math.sin(ang_dist)*Math.sin(point_bearing - seg_bearing)) * R;
        const along_path = Math.acos(Math.cos(ang_dist)/Math.cos(cross_track/R)) * R;
        const before_path = Math.cos(point_bearing - seg_bearing) < 0;
        const after_path = Math.cos(point_bearing_end - seg_bearing) > 0;
        if( before_path ) {
            return segStart.distanceTo(point);
        }
        if( after_path ) {
            return segEnd.distanceTo(point);
        }
        return Math.abs(cross_track);
    }

    function distanceAlongLineSegment(segStart,segEnd,point) {
        const R = 6371e3;
        const DEG_RAD = 0.01745329;
        const ang_dist = segStart.distanceTo(point) / R;
        const seg_bearing = segStart.azimuthTo(segEnd) * DEG_RAD;
        const point_bearing = segStart.azimuthTo(point) * DEG_RAD;
        const point_bearing_end = segEnd.azimuthTo(point) * DEG_RAD;
        const cross_track = Math.asin(Math.sin(ang_dist)*Math.sin(point_bearing - seg_bearing)) * R;
        const along_path = Math.acos(Math.cos(ang_dist)/Math.cos(cross_track/R)) * R;
        const before_path = Math.cos(point_bearing - seg_bearing) < 0;
        const after_path = Math.cos(point_bearing_end - seg_bearing) > 0
        if( before_path ) {
            return 0;
        }
        if( after_path ) {
            return segStart.distanceTo(segEnd);
        }
        return along_path;
    }

    function getNearestCoord(resp) {
        const DEG_RAD = 0.01745329;
        const lowerIdx = Math.max(resp["coord_idx"] - 10,  0);
        const upperIdx = Math.min(resp["coord_idx"] + 30, resp["coords"].length - 1);
        const maxMatchDist = 20;
        const maxMatchAng = 35;
        let lowestDist = 1000000;
        let lowestDistIdx = 0;
        let lowestDistValidHeading = false;
        for(let i=lowerIdx; i<upperIdx; i++) {
            const curCoord = resp["coords"][i];
            const nextCoord = resp["coords"][i+1];
            const curCoordVal = QtPositioning.coordinate(curCoord[0], curCoord[1]);
            const nextCoordVal = QtPositioning.coordinate(nextCoord[0], nextCoord[1]);
            if(curCoordVal.distanceTo(nextCoordVal) == 0) {
                continue;
            }
            const distance = distanceFromLineSegment(curCoordVal, nextCoordVal, Gps.position);
            const heading = curCoordVal.azimuthTo(nextCoordVal);
            const valid = distance < maxMatchDist;
            const validHeading = valid && (Math.abs(Math.sin((Gps.bearing - heading)*DEG_RAD)) < Math.sin(maxMatchAng*DEG_RAD));
            //console.log(i + ": "+ distance + "  " + heading + "  " + valid + "  " + validHeading)
            if(distance < lowestDist && (!lowestDistValidHeading || validHeading)) {
                lowestDistIdx = i;
                lowestDist = distance;
            }
            if(!lowestDistValidHeading && validHeading) {
                lowestDistIdx = i;
                lowestDist = distance;
                lowestDistValidHeading = true;
            }
        }
        console.log("Valid Heading: "+lowestDistValidHeading);
        if(lowestDist > maxMatchDist) {
            if(valhallaRouteReqTime + 5000 < (new Date()).getTime()) {
                recalc_route();
            }
        }
        resp["coord_idx"] = lowestDistIdx;
        return lowestDistIdx;
    }

    function updateValhalla() {
        const coordIdx = getNearestCoord(valhallaRouteResp);
        const legIdx = valhallaRouteResp["coords"][coordIdx][2];
        const maneuverIdx = valhallaRouteResp["coords"][coordIdx][3];
        const maneuverDist = valhallaRouteResp["coords"][coordIdx][4];
        {
            const curCoord = valhallaRouteResp["coords"][coordIdx];
            const nextCoord = valhallaRouteResp["coords"][coordIdx+1];
            const curCoordVal = QtPositioning.coordinate(curCoord[0], curCoord[1]);
            const nextCoordVal = QtPositioning.coordinate(nextCoord[0], nextCoord[1]);
            debugPolyline.path = [curCoordVal, nextCoordVal]
            const dist_meters = valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"][maneuverIdx]["length"]*1000 - maneuverDist - distanceAlongLineSegment(curCoordVal, nextCoordVal, Gps.position); 
            console.log(coordIdx + "  " + legIdx + "    " + maneuverIdx + "  " + maneuverDist + "   " + distanceAlongLineSegment(curCoordVal, nextCoordVal, Gps.position));
            nav_box.distance = metersToString(dist_meters);
            {
                let dist_meters_leg = dist_meters;
                for(let i=maneuverIdx+1;i<valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"].length;i++) {
                    dist_meters_leg += valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"][i]["length"]*1000;
                }
                distanceLegStr = metersToString(dist_meters_leg);
            }
            {
                let time = valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"][maneuverIdx]["time"] * Math.min(1,dist_meters/valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"][maneuverIdx]["length"]/1000);
                for(let i=maneuverIdx+1;i<valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"].length;i++) {
                    time += valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"][i]["time"];
                }
                timeLegSec = time;
                timeLegStr = Math.round(time/60) + " min";
            }
        }
        if(maneuverIdx + 1 == valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"].length) {
            nav_box.icon = "arrive";
            nav_box.exit = "";
            nav_box.line1 = "";
            nav_box.line2 = "";
        } else {
            const banner_maneuver = valhallaRouteResp["trip"]["legs"][legIdx]["maneuvers"][maneuverIdx+1];
            //console.log(JSON.stringify(banner_maneuver));
            nav_box.icon = vallhallaIconLookup(banner_maneuver["type"]);
            if( banner_maneuver.hasOwnProperty("sign") ) {
                if(banner_maneuver["sign"].hasOwnProperty("exit_number_elements")) {
                    nav_box.exit = valhallaJoinText(banner_maneuver["sign"]["exit_number_elements"],"-");
                } else {
                    nav_box.exit = "";
                }
                nav_box.line1 = valhallaJoinText(banner_maneuver["sign"]["exit_branch_elements"],"/");
                if(banner_maneuver["sign"].hasOwnProperty("exit_toward_elements")) {
                    nav_box.line2 = valhallaJoinText(banner_maneuver["sign"]["exit_toward_elements"],"/");
                } else {
                    nav_box.line2 = "";
                }
            } else {
                nav_box.exit = "";
                nav_box.line2 = "";
                if(banner_maneuver.hasOwnProperty("begin_street_names")) {
                    nav_box.line1 = banner_maneuver["begin_street_names"].join("/");
                } else if(banner_maneuver.hasOwnProperty("street_names")) {
                    nav_box.line1 = banner_maneuver["street_names"].join("/");
                } else {
                    nav_box.line1 = "";
                }
            }
        }
    }

    function metersToString(meters) {
        const miles = meters/1609.344;
        const feet = meters*3.28;
        if( miles > 1.75 ) {
            return Math.round(miles) + " mi";
        } else if( miles > 1.25 ) {
            return "1.5 mi";
        } else if ( miles > 0.875) {
            return "1 mi";
        } else if ( miles > 0.625) {
            return "0.75 mi";
        } else if (miles > 0.375) {
            return "0.5 mi";
        } else if (feet > 1250) {
            return "0.25 mi";
        } else if (feet > 750) {
            return "1000 ft";
        } else if (feet > 375) {
            return "500 ft";
        } else if (feet > 175) {
            return "200 ft";
        } else if (feet > 125) {
            return "150 ft";
        } else if(feet > 75) {
            return "100 ft";
        } else if(feet > 25) {
            return "50 ft";
        } else {
            return "0 ft"
        }
    }

    function valhallaJoinText(array,sep) {
        if( array.length == 0 ) {
            return "";
        }
        let ret = array[0]["text"];
        for(let i=1;i<array.length;i++) {
            ret += sep;
            ret += array[i]["text"];
        }
        return ret;
    }

    function vallhallaIconLookup(type) {
        switch(type) {
            default: return "";
            case  1: return "depart";
            case  2: return "depart-right";
            case  3: return "depart-left";
            case  4: return "arrive";
            case  5: return "arrive-right";
            case  6: return "arrive-left";
            case  7: return "new-name-straight";
            case  8: return "continue";
            case  9: return "turn-slight-right";
            case 10: return "turn-right";
            case 11: return "turn-sharp-right";
            case 12: return "uturn";
            case 13: return "uturn";
            case 14: return "turn-sharp-left";
            case 15: return "turn-left";
            case 16: return "turn-slight-left";
            case 17: return "on-ramp-straight";
            case 18: return "on-ramp-right";
            case 19: return "on-ramp-left";
            case 20: return "off-ramp-right";
            case 21: return "off-ramp-left";
            case 22: return "continue-straight";
            case 23: return "continue-right";
            case 24: return "continue-left";
            case 25: return "merge-straight";
            case 26: return "roundabout";
            case 27: return "roundabout-straight";
            case 28: return "ferry";
            case 29: return "ferry";
            case 30: return "";
            case 31: return "";
            case 32: return "";
            case 33: return "";
            case 34: return "";
            case 35: return "";
            case 36: return "";
            case 37: return "merge-right";
            case 38: return "merge-left";
        }
    }
}
