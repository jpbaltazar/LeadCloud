package models

type Device struct {
	//Effective fields
	Id int
	Name string
	//GId int
	Owner string
	IpAdd string
	Configs string

	//Fluff
	Location string
	Status string //TODO placeholder, should be acquired by an active connection to the rpi
}

type User struct {
	Username string
	Password []byte
}

type Schedule struct {
	Name 			string `json:"name"`
	ID 				int `json:"id"`

	Owner 			string `json:"owner"`

	JsonSchedule 	JSONSchedule `json:"jsonSchedule"`
}

//the representation that matters to the local system

type JSONScheduleHeader struct{
	Title 			string 	`json:"title"`
	Description 	string 	`json:"description"`
	ScheduleType	int 	`json:"scheduleType"`
}

type JSONScheduleEntry struct {
	EntryType 	int 		`json:"entryType"`

	//for adding/removing event
	ID			int 		`json:"id"`

	//properties when adding an event
	Name		string		`json:"name"` //not actually the program name, just a convenient representation
	Duration	int 		`json:"duration"`
	Args		[]string 	`json:"args"`

	//time for setting time
	Time		int 		`json:"time"`
}

type JSONSchedule struct{
	Header JSONScheduleHeader `json:"header"`
	Entries []JSONScheduleEntry `json:"entries"`
}