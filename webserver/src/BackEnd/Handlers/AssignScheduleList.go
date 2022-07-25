package handlers

import (
	"bytes"
	"net/http"
	"strconv"
	"text/template"
)

//shows a page through a template
func AssignScheduleList(w http.ResponseWriter, r* http.Request){

	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}
	username := usernameCookie.Value

	idString, err := stripUrlPrefix(r.URL.String(), "/assignScheduleList/")
	if err != nil {
		http.Redirect(w, r, "/devicePage", 302)
		return
	}

	id, err := strconv.Atoi(idString)
	if err != nil{
		http.Redirect(w, r, "/devicePage", 302);
		return
	}

	device, err := DeviceAPI.FindDeviceByOwnerAndId(username, id)
	if err != nil{
		http.Redirect(w, r, "/devicePage", 302)
		return
	}

	schedules, err := ScheduleAPI.FindSchedulesByOwner(username)
	if err != nil{
		http.Redirect(w, r, "/devicePage", 302)
		return
	}

	type ScheduleRep struct {
		ID		int
		Title	string
		Desc 	string
		
		DID		int //device ID
	}

	entries := ""

	if len(schedules) > 0 {
		for i := 0; i < len(schedules); i++ {
			schedule := schedules[i]

			scheduleRep := ScheduleRep{
				ID: schedule.ID,
				Title: schedule.JsonSchedule.Header.Title,
				Desc: schedule.JsonSchedule.Header.Description,

				DID: device.Id,
			}

			buf := bytes.Buffer{}
			entry, err := template.ParseFiles("./src/frontend/templates/AssignScheduleEntry.html")
			if err != nil{
				print("Error! %s\n", err.Error())
			}

			err = entry.Execute(&buf, scheduleRep)
			if err != nil{
				print("Error in executing template %s\n", err.Error())
			}

			entries += buf.String()
		}
	} else{
		buf := bytes.Buffer{}
		entry, err := template.ParseFiles("./src/frontend/templates/AssignScheduleEmpty.html")
		if err != nil{
			print("Error! %s\n", err.Error())
		}

		err = entry.Execute(&buf, nil)

		entries += buf.String()
	}


	type TemplateRep struct {
		Name string
		Entries string
	}

	templateRep := TemplateRep{
		device.Name,
		entries,
	}

	assignScheduleListTemplate, _ := template.ParseFiles("./src/frontend/templates/AssignScheduleT.html")
	assignScheduleListTemplate.Execute(w, templateRep)
}

