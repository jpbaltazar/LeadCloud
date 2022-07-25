package handlers

import (
	"encoding/json"
	"net/http"
	"webTut/src/models"
)

func ModifyScheduleHandler(w http.ResponseWriter, r *http.Request){
	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	//bodyBytes, err := ioutil.ReadAll(r.Body)

	/*body, err := r.GetBody()
	if err != nil {
		fmt.Sprintf("Error %v\n", err)
		return
	}*/
	//body := bodyBytes
	var newSchedule models.Schedule

	jsonDecoder := json.NewDecoder(r.Body)
	err = jsonDecoder.Decode(&newSchedule)
	if err != nil {
		return
	}

	//unauthorized access
	if newSchedule.Owner != usernameCookie.Value {
		http.Redirect(w, r, "/scheduleList", 302)
	}

	schedules, err := ScheduleAPI.FindSchedulesByOwner(usernameCookie.Value)
	if err != nil{
		return
	}

	found := false
	var schedule models.Schedule
	for i := 0; i < len(schedules); i++{
		if newSchedule.ID == schedules[i].ID{
			schedule = schedules[i]
			found = true
			break
		}
	}

	if found {
		_ = ScheduleAPI.RemoveSchedule(schedule)
		_ = ScheduleAPI.AddSchedule(newSchedule)
	}

	http.Redirect(w, r, "/scheduleList", 302)
	return
}
