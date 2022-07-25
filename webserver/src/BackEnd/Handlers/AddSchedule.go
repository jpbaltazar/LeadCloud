package handlers

import (
	"fmt"
	"math/rand"
	"net/http"
	"time"
	"webTut/src/models"
)

func AddScheduleHandler(w http.ResponseWriter, r *http.Request){
	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	if err := r.ParseForm(); err != nil{
		http.Redirect(w, r, "/scheduleList", 302)
		return
	}

	scheduleName := r.FormValue("scheduleName")
	scheduleFreq := r.FormValue("scheduleFreq")

	if 	scheduleName == "" ||
		scheduleFreq == "" {
		http.Redirect(w, r, "/scheduleList", 302)
	}

	schedules, err := ScheduleAPI.FindSchedulesByOwner(usernameCookie.Value)
	if err != nil {
		return
	}


	scheduleType := 0
	switch scheduleFreq {
	case "daily":
		scheduleType = 1
		break
	case "weekly":
		scheduleType = 2
		break
	case "monthly":
		scheduleType = 3
		break
	case "yearly":
		scheduleType = 4
		break
	default:
		scheduleType = 0
		http.Redirect(w, r, "/scheduleList", 302)
		return
	}

	var scheduleMap = make(map[int]bool)
	for i := 0; i < len(schedules); i++ {
		scheduleMap[schedules[i].ID] = true
	}

	s1 := rand.NewSource(time.Now().UnixNano())
	r1 := rand.New(s1)

	//better system must be created
	id := 0
	for true {
		id = r1.Int()%1000000

		if !scheduleMap[id] {
			break
		}
	}

	sched := models.Schedule{
		Name:         	scheduleName,
		ID: 			id,
		Owner:        	usernameCookie.Value,
		JsonSchedule: 	models.JSONSchedule{
			Header:  models.JSONScheduleHeader{
				Title:       	scheduleName,
				Description:  	r.FormValue("scheduleDesc"),
				ScheduleType: 	scheduleType,

			},
			Entries: nil,
		},
	}

	err = ScheduleAPI.AddSchedule(sched)
	if err != nil {
		return
	}

	http.Redirect(w, r, fmt.Sprintf("/editSchedule/%d",id), 302)
}