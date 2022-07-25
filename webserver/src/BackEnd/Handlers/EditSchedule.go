package handlers

import (
	"bytes"
	"encoding/json"
	"net/http"
	"strconv"
	"strings"
	"text/template"
)

func EditScheduleHandler(w http.ResponseWriter, r *http.Request){
	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}


	idString := r.URL.String()
	idString = strings.TrimPrefix(idString, "/editSchedule/")
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

	schedule, err := ScheduleAPI.FindSchedulesByOwnerAndId(usernameCookie.Value, id)
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	var buf bytes.Buffer
	enc := json.NewEncoder(&buf)
	_ = enc.Encode(schedule.JsonSchedule)


	printableString := struct {
		Name string `json:"name"`
		ID	int `json:"id"`
		Owner string `json:"owner"`
		JsonSchedule string `json:"jsonSchedule"`
	}{
		Name: schedule.Name,
		ID: schedule.ID,
		Owner: schedule.Owner,
		JsonSchedule: buf.String(),
	}

	scheduleEditorTemplate, _ := template.ParseFiles("./src/frontend/templates/ScheduleEditor.html")
	_ = scheduleEditorTemplate.Execute(w, printableString)
}


