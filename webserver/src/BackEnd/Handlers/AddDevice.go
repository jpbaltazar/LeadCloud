package handlers

import (
	"math/rand"
	"net/http"
	"time"
	"webTut/src/BackEnd/DeviceConfig"
	"webTut/src/BackEnd/TCPServer"
	"webTut/src/models"
)

var ConfigBinder DeviceConfig.DeviceConfigBinder

func AddDeviceHandler(w http.ResponseWriter, r *http.Request){
	if err := r.ParseForm(); err != nil{
		http.Redirect(w, r, "/addDevicePage.html", 302)
		return
	}


	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	var configKey = r.FormValue("configKey")

	devices, err := DeviceAPI.FindDevicesByOwner(usernameCookie.Value)
	if err != nil {
		return 
	}

	var deviceMap = make(map[int]bool)
	for i := 0; i < len(devices); i++ {
		deviceMap[devices[i].Id] = true
	}

	s1 := rand.NewSource(time.Now().UnixNano())
	r1 := rand.New(s1)

	//better system must be created
	id := 0
	for true {
		id = r1.Int()%1000000

		if !deviceMap[id] {
			break
		}
	}

	//should get some details from the cookies (username, etc)
	var device = models.Device{
		Id:       id,
		Name: 	  r.FormValue("deviceName"),
		Owner:    usernameCookie.Value,
		IpAdd:    "", //filled in by the API
		Configs:  r.FormValue("configs"),
		Location: r.FormValue("location"),
		Status:   "online",
	}

	//TODO check .Configs before sending it

	if configKey == "" || //if any of the fields are not filled
		ConfigBinder.ConfigBindMap[configKey] == nil || //or the key is not valid
		device.Owner == "" ||
		device.Configs == "" ||
		device.Location == "" {

		http.Redirect(w, r, "/addDevicePage.html", 302)
		return
	}



	//Claim the device by getting the key and adding the details to it
	device = ConfigBinder.ClaimDevice(configKey, device)

	defer http.Redirect(w, r, "/devicePage", 302)

	l := TCPServer.LocalSystemConnection{}
	err = l.StartConnection(device.IpAdd)
	if err != nil {
		return
	}

	err = l.NewConfig(device.Configs)
	if err != nil{
		return
	}

	err = l.CloseConnection()
	if err != nil{
		return
	}

}