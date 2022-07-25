package handlers

import (
	"net/http"
	"strings"
	"webTut/src/models"
)

func DeviceSelfRegisterHandler(w http.ResponseWriter, r *http.Request) {
	//get form info
	//save IPAddr

	var d models.Device
	d.IpAdd = r.RemoteAddr
	//comes in format 255.255.255.255:6666
	//drop the last part

	portStartIndex := strings.Index(d.IpAdd, ":")
	if portStartIndex != -1 {
		d.IpAdd = d.IpAdd[0:portStartIndex]
	}

	var key = ""
	for i := 0; i < 5; i++{
		key = ConfigBinder.GenConfigKey()
		if err := ConfigBinder.AddDevice(key, &d); err == nil{
			break
		}
	}

	w.Write([]byte(key))
}