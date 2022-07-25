package handlers

import (
	"net/http"
	"strconv"
	"strings"
)

func RemoveDevice(w http.ResponseWriter, r *http.Request)  {
	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}
	print("hello")
	var username = usernameCookie.Value

	idString := r.URL.String()
	idString = strings.TrimPrefix(idString, "/removeDevice/")

	if idString == r.URL.String(){
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	tokenStartIndex := strings.Index(idString, "?")
	if tokenStartIndex != -1 {
		idString = idString[0:tokenStartIndex]
	}

	id, err := strconv.Atoi(idString)
	if err != nil{
		http.Redirect(w, r, "/devicePage", 302)
		return
	}

	toRemove, err := DeviceAPI.FindDeviceByOwnerAndId(username, id)
	if err != nil{
		http.Redirect(w, r, "/devicePage", 302)
		return
	}

	_ = DeviceAPI.RemoveDevice(*toRemove)

	http.Redirect(w, r, "/devicePage", 302)
}
