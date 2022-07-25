package handlers

import (
	"net/http"
	"strconv"
	"strings"
)

func RemoveSchedule(w http.ResponseWriter, r *http.Request)  {
	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	var username = usernameCookie.Value

	idString := r.URL.String()
	idString = strings.TrimPrefix(idString, "/removeSchedule/")
	//still need to strip anything that is not a number after the number

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
		http.Redirect(w, r, "/schedulePage", 302)
		return
	}

	toRemove, err := ScheduleAPI.FindSchedulesByOwnerAndId(username, id)
	if err != nil{
		http.Redirect(w, r, "/schedulePage", 302)
		return
	}

	_ = ScheduleAPI.RemoveSchedule(*toRemove)
	http.Redirect(w, r, "/schedulePage", 302)
}