package handlers

import (
	"bytes"
	"fmt"
	"go.mongodb.org/mongo-driver/mongo"
	"net/http"
	"text/template"
	API "webTut/src/APIs"
)

type scheduleEntry struct {
	Name 		string
	Type 		string
	Description string
	ID 			string
}

var ScheduleAPI API.ScheduleAPI

func SchedulePageHandler(w http.ResponseWriter, r *http.Request) {
	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	var username = usernameCookie.Value

	schedules, err := ScheduleAPI.FindSchedulesByOwner(username)
	if err != nil && err != mongo.ErrNoDocuments{
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	entries := ""

	ScheduleTypeLUT := [5]string{
		"",
		"daily",
		"weekly",
		"monthly",
		"yearly",
	}

	for s := range schedules{
		schedule := schedules[s]

		sE := scheduleEntry{
			Name:        	schedule.Name,
			Type:        	ScheduleTypeLUT[schedule.JsonSchedule.Header.ScheduleType],
			Description: 	schedule.JsonSchedule.Header.Description,
			ID: 			fmt.Sprintf("%d", schedule.ID),
		}

		buf := bytes.Buffer{}
		entry, err := template.ParseFiles("./src/frontend/templates/scheduleEntry.html")
		if err != nil{
			print("Error! %s\n", err.Error())
		}

		err = entry.Execute(&buf, sE)
		if err != nil{
			print("Error in Execute %s\n", err.Error())
		}
		entries += buf.String() //added a row of info
	}

	scheduleListTemplate, _ := template.ParseFiles("./src/frontend/templates/schedulesT.html")
	_ = scheduleListTemplate.Execute(w, entries)
}