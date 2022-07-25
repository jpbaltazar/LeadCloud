package handlers

import (
	"fmt"
	"net/http"
	"strconv"
	"strings"
	"webTut/src/BackEnd/TCPServer"
)

func AssignSchedule(w http.ResponseWriter, r* http.Request){

	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	//looks like this: /assignSchedule/55555/666666?...
	str := "" + r.URL.String()
	str = strings.TrimPrefix(str, "/assignSchedule/")

	if str == r.URL.String(){
		print(fmt.Errorf("prefix does not exist in string"))
		http.Redirect(w, r, "/deviceList", 302)
		return
	}

	//should look like this 55555/666666?...
	devIdString := ""

	scheduleIdStart := strings.Index(str, "/")
	if scheduleIdStart != -1 {
		devIdString = str[0:scheduleIdStart] //"55555"

		str = str[scheduleIdStart + 1:] //666666?
	}

	deviceId, err := strconv.Atoi(devIdString)
	if err != nil{
		http.Redirect(w, r, "/deviceList", 302)
		return
	}

	scheduleIDString := ""

	tokenStartIndex := strings.Index(str, "?")
	if scheduleIdStart != -1 {
		scheduleIDString = str[0:tokenStartIndex]
	}

	scheduleId, err := strconv.Atoi(scheduleIDString)
	if err != nil{
		http.Redirect(w, r, "/deviceList", 302)
		return
	}

	device, err := DeviceAPI.FindDeviceByOwnerAndId(usernameCookie.Value, deviceId)
	if err != nil{
		http.Redirect(w, r, "/deviceList", 302)
		return
	}

	schedule, err := ScheduleAPI.FindSchedulesByOwnerAndId(usernameCookie.Value, scheduleId)
	if err != nil{
		http.Redirect(w, r, "/deviceList", 302)
		return
	}

	var l = TCPServer.LocalSystemConnection{}
	err = l.StartConnection(device.IpAdd)
	if err != nil {
		http.Redirect(w, r, "/deviceList", 302)
		return
	}

	err = l.NewSchedule(*schedule)
	if err != nil {
		return
	}

	err = l.CloseConnection()
	if err != nil {
		return
	}

	http.Redirect(w, r, "/devicePage", 302)
}
